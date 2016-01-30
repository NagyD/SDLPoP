/*
SDLPoP editor module
Copyright (C) 2013-2016  Dávid Nagy, Enrique P. Calot

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

The authors of this program may be contacted at http://forum.princed.org
*/

#include "common.h"
#ifdef USE_EDITOR

/********************************************\
*             Headers of room API            *
\********************************************/

#define MAP_SIDE (NUMBER_OF_ROOMS*2+3)
#define MAP_CENTER (NUMBER_OF_ROOMS+1)
#define POS_UP (-MAP_SIDE)
#define POS_DOWN MAP_SIDE
#define POS_LEFT (-1)
#define POS_RIGHT 1
#define MAP_POS(x,y) ((y)*MAP_SIDE+(x))

#define PALETTE_MODE (drawn_room>NUMBER_OF_ROOMS)

#define TILE_MASK 0x1F

typedef struct {
	long* list;
	byte* map;
	rect_type crop;
} tMap;

long room_api_where_room(const tMap* map, byte room);
int room_api_get_room_location(const tMap* map, int room, int* i, int* j);
void room_api_refresh(tMap* map);
void room_api_init(tMap* map);
void room_api_free(tMap* map);
void room_api_get_size(const tMap* map,int* w, int* h);
tile_global_location_type room_api_translate(const tMap* map,int x, int y);
int room_api_get_free_room(const tMap* map);
void room_api_free_room(tMap* map,int r); /* Pre-condition: DO NOT FREE THE STARTING ROOM! */
void room_api_put_room(tMap* map, long where, int r);
int room_api_alloc_room(tMap* map, int where);
int room_api_insert_room_right(tMap* map, int where);
int room_api_insert_room_left(tMap* map, int where);
int room_api_insert_room_up(tMap* map, int where);
int room_api_insert_room_down(tMap* map, int where);
tile_global_location_type room_api_tile_move(const tMap* map, tile_global_location_type t, char col, char row);

/********************************************\
*      Randomization sublayer headers        *
\********************************************/

/* E_x_y = e^(-(x^2+y^2)) -> gaussian blur */
#define E_0_1 .36787944117144232159
#define E_0_2 .01831563888873418029
#define E_1_1 .13533528323661269189
#define E_1_2 .00673794699908546709

#pragma pack(push, 1)
typedef struct {
		byte fg;
		byte bg;
	} concept_type; /* tile_and_mod but packed */

typedef union {
	concept_type concept;
	Uint16 number;
} tile_packed_type;
#pragma pack(pop)

typedef struct probability_info {
	int count;
	float value;
} tProbability;

float room_api_measure_entropy(const tMap* map, tile_global_location_type t, byte* level_mask);
tile_packed_type room_api_suggest_tile(const tMap* map, tile_global_location_type tilepos, byte* level_mask);

/********************************************\
*  DUR: Do, undo, redo layer implementation  *
\********************************************/

/* DUR: stack sublayer */
#define true 1
#define false 0

int stack_size=0;
int stack_cursor=0;
long* stack_pointer=NULL;
int stack_top=0;
#define stack_reset() stack_top=0;
/* private type */
typedef uint64_t flag_type; /* note that uint64_t/unsigned long long is supported even in 32 bits machines and virtually by all linux, 32 bit windows and OSX. The variable is used as:
  RRRRRRRR RRRRRRRR RRRRRRRR RRRRRRRR RRRRRRRR Rgpdmres BBBBBBBB AAAAAAAA
	where R are an offset/reserved bit
	r,m,d,p,g are redraw,remap,redoor,guard_presence,guard_repaint flags
	e,s are startm end marks
	B is the 8 bits code before editing
	A is the 8 bits code after editing
*/

void stack_push(flag_type data) {
	if (!stack_size) {
		stack_size=100;
		stack_pointer=malloc(sizeof(flag_type)*stack_size);
	}
	stack_pointer[stack_cursor]=data;
	stack_cursor++;
	/*if (stack_top<stack_cursor)*/ stack_top=stack_cursor;
	if (stack_size<=stack_top) {
		stack_size*=2;
		stack_pointer=realloc(stack_pointer,sizeof(flag_type)*stack_size);
	}
}

int stack_pop(flag_type* data) {
	if (!stack_cursor) return false;
	*data=stack_pointer[--stack_cursor];
	return true;
}

int stack_unpop(flag_type* data) {
	if (stack_cursor==stack_top) return false;
	*data=stack_pointer[stack_cursor++];
	return true;
}

void stack_or(flag_type data) {
	if (stack_cursor) {
		stack_pointer[stack_cursor-1]|=data;
	}
}
#undef true
#undef false

/* DUR: Do actions sublayer */

#define MARK_BITS 7
typedef enum {
	mark_middle=0,
	mark_start=1,
	mark_end=2,
	mark_all=3,

	flag_redraw=4,
	flag_remap=8,
	flag_redoor=16,

	flag_guard_presence   =32,
	flag_guard_repaint    =64,

	flag_mask=124,
	mark_flag_mask=127
}tUndoQueueMark;

tUndoQueueMark prevMark=mark_middle;
#define editor__do_mark_start(m) prevMark=mark_start|m
void editor__do_mark_end(tUndoQueueMark m) {
	if (prevMark) /* if writing is open */
		stack_or((mark_end|m)<<16);
	prevMark=mark_middle;
}

/* editor level layer used by do/undo/redo */
typedef enum {extra_none,extra_up='A',extra_down='B',extra_left='L',extra_right='R'} movement_type;
typedef struct {
	tile_packed_type main;
	movement_type extratype;
	tile_packed_type extra;
} copied_type;

void editor__load_level();
copied_type copied={NO_TILE_,extra_none,NO_TILE_};

tile_packed_type clipboard[30]={NO_TILE_,NO_TILE_,NO_TILE_,NO_TILE_,NO_TILE_,NO_TILE_,NO_TILE_,NO_TILE_,NO_TILE_,NO_TILE_,NO_TILE_,NO_TILE_,NO_TILE_,NO_TILE_,NO_TILE_,NO_TILE_,NO_TILE_,NO_TILE_,NO_TILE_,NO_TILE_,NO_TILE_,NO_TILE_,NO_TILE_,NO_TILE_,NO_TILE_,NO_TILE_,NO_TILE_,NO_TILE_,NO_TILE_,NO_TILE_};
byte selected_mask[30]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
#define clean_selected_mask() memset(selected_mask,0,sizeof(byte)*30)
int selected_mask_room=-1;
enum {chNothing=-1,chFullRoom=1,chTiles=0} clipboard_has=chNothing;
int clipboard_shift=0;

level_type edited;
int ambiguous_mode=0;
int remember_room=0; /* when switching to palette room mode */
int map_selected_room=0;
int goto_next_room=0;
tMap edited_map={NULL,NULL};
int edited_doorlinks=0;
int edition_level=-1;

#define editor__do(field,c,mark) editor__do_( ((long)(&(((level_type*)NULL)->field))) ,c,mark)
void editor__do_(long offset, byte c, tUndoQueueMark mark) {
	byte before;
	mark=mark|prevMark;
	prevMark=mark_middle;

	if (edition_level!=current_level)
		editor__load_level();

	before=offset[(char*)(&edited)];
	offset[(char*)(&edited)]=c;
	offset[(char*)(&level)]=c;
	stack_push(offset<<(16+MARK_BITS)|mark<<16|before<<8|c);
}

void editor_change_tile(tile_global_location_type l, tile_packed_type t) {
	editor__do(fg[l],t.concept.fg,mark_middle);
	editor__do(bg[l],t.concept.bg,mark_middle);
}

tUndoQueueMark editor__undo() {
	flag_type aux;
	int offset;
	byte before;
	tUndoQueueMark mark=0;
	while (stack_pop(&aux)) {
		/* after=   aux     & 0xff; */
		before= (aux>>8) & 0xff;
		mark=   (aux>>16)& mark_flag_mask;
		offset= (aux>>(16+MARK_BITS));
		offset[(char*)(&level)]=before;
		offset[(char*)(&edited)]=before;
		if (mark & mark_start) break;
	}
	return mark&flag_mask;
}

tUndoQueueMark editor__redo() {
	flag_type aux;
	int offset;
	byte after;
	tUndoQueueMark mark=0;
	while (stack_unpop(&aux)) {
		after=   aux     & 0xff;
		/* before= (aux>>8) & 0xff; */
		mark=   (aux>>16)& mark_flag_mask;
		offset= (aux>>(16+MARK_BITS));
		offset[(char*)(&level)]=after;
		offset[(char*)(&edited)]=after;
		if (mark & mark_end) break;
	}
	return mark&flag_mask;
}

/********************************************\
*           Room refreshing functions        *
\********************************************/

void ed_select_room(int room) {
	get_room_address(room);
	room_L = level.roomlinks[room-1].left;
	room_R = level.roomlinks[room-1].right;
}

void ed_redraw_tile(int tilepos) { //of the current room
	level.fg[T(loaded_room,tilepos)]=edited.fg[T(loaded_room,tilepos)];
	level.bg[T(loaded_room,tilepos)]=edited.bg[T(loaded_room,tilepos)];
	load_alter_mod(tilepos);
}

void ed_redraw_room() { //all the current room
	for (int i=0;i<30;i++) ed_redraw_tile(i);
}

/********************************************\
*             Door linking API               *
\********************************************/

typedef struct {
	enum {doorIterator, buttonIterator, noneIterator} type;
	union {
		short index;
		struct {
			tile_global_location_type tile;
			short i;
		} info;
	} data;
} tIterator;

void door_api_init(int* max_doorlinks);
void door_api_free(int* max_doorlinks);
void door_api_init_iterator(tIterator* it, tile_global_location_type tp);
int door_api_get(tIterator* it, tile_global_location_type *tile); /* returns false when end_of_list */
int door_api_link(int* max_doorlinks, tile_global_location_type door,tile_global_location_type button); /* Assumption: door is a door (or left exitdoor) and button is a button */
void door_api_unlink(int* max_doorlinks, tile_global_location_type door,tile_global_location_type button);

typedef enum {
	cButton=2,
	cDoor=1,
	cOther=0
} tTileDoorType;

tTileDoorType door_api_is_related(byte tile) {
	switch(tile) {
	case tiles_6_closer:
	case tiles_15_opener:
		return cButton;
	case tiles_4_gate:
	case tiles_16_level_door_left:
	case tiles_17_level_door_right:
		return cDoor;
	}
	return cOther;
}

void door_api_init_iterator(tIterator* it, tile_global_location_type tp) {
	byte tile=edited.fg[tp]&TILE_MASK;
	switch(tile) {
		case tiles_6_closer:
		case tiles_15_opener:
			it->data.index=edited.bg[tp];
			it->type=it->data.index==255?noneIterator:buttonIterator; /* 255 is reserved as no link */
			return;
		case tiles_4_gate:
		case tiles_16_level_door_left:
			it->data.info.tile=tp;
			it->data.info.i=0;
			it->type=doorIterator;
			return;
		case tiles_17_level_door_right:
			if (P(tp)%10 &&
				(edited.fg[tp-1]&TILE_MASK)==tiles_16_level_door_left) {
					tp--;
					door_api_init_iterator(it,tp);
					return;
				}
		}
		it->type=noneIterator;
		return;
}

int door_api_get(tIterator* it, tile_global_location_type *tile) {
	switch (it->type) {
	case buttonIterator:
		if (it->data.index>=NUMBER_OF_DOORLINKS) return 0;
		short next;
		get_doorlink((edited.doorlinks2[it->data.index]<<8)|edited.doorlinks1[it->data.index],tile,&next);
		it->data.index++;
		if (!next) it->type=noneIterator;
		return 1;
	case doorIterator:
		for (short* i=&it->data.info.i;(*i)<NUMBER_OF_ROOMS*30;(*i)++) { /* first loop: check all tiles to find buttons */
			byte fg=edited.fg[*i]&TILE_MASK;
			if (fg==tiles_6_closer || fg==tiles_15_opener) {
				tIterator it2;
				tile_global_location_type linked_tile;
				if (!edited_map.list[R_(*i)]) continue; /* skip unused rooms */
				door_api_init_iterator(&it2,*i);
				while(door_api_get(&it2,&linked_tile)) { /* second loop: find door opened by those buttons */
					if (linked_tile==it->data.info.tile) {
						*tile=*i;
						(*i)++;
						return 1;
					}
				}
			}
		}
		it->type=noneIterator;
	case noneIterator:
		break;
	}
	return 0;
}

void door_api_free(int* max_doorlinks) {}
void door_api_init(int* max_doorlinks) {
	tile_global_location_type i;
	*max_doorlinks=0;
	for (i=0;i<NUMBER_OF_ROOMS*30;i++) {
		if (door_api_is_related(edited.fg[i]&TILE_MASK)==cButton) {
			byte aux=edited.bg[i];
			if (*max_doorlinks<aux) *max_doorlinks=aux;
		}
	}

	/* itarate over the last value */
	short next=1;
	tile_global_location_type junk;
	do {
		get_doorlink((edited.doorlinks2[*max_doorlinks]<<8)|edited.doorlinks1[*max_doorlinks],&junk,&next);
		(*max_doorlinks)++;
	} while ((*max_doorlinks)<256 && next);
	(*max_doorlinks)--;
}

int door_api_link(int* max_doorlinks, tile_global_location_type door,tile_global_location_type button) { /* Assumption: door is a door (or left exitdoor) and button is a button */
	if (*max_doorlinks==255) return 0; /* no more space available */
	int pivot=edited.bg[button];
	Uint16 doorlink;

	if (pivot==255) { /* I'm defining 255 as "no links" */
		/* append link on the top of the list */
		(*max_doorlinks)++;
		editor__do(bg[button],(*max_doorlinks),mark_middle);

		/* 3) insert the link */
		set_doorlink(&doorlink,door,0);
		editor__do(doorlinks1[*max_doorlinks],doorlink&0xff,mark_middle);
		editor__do(doorlinks2[*max_doorlinks],(doorlink>>8)&0xff,mark_middle);
	} else {
		/* three steps to insert a new link: */
		/* 1) make space in the table */
		int i;
		for (i=*max_doorlinks;i>=pivot;i--) {
			editor__do(doorlinks1[i+1],edited.doorlinks1[i],mark_middle);
			editor__do(doorlinks2[i+1],edited.doorlinks2[i],mark_middle);
		}
		/* 2) update button references */
		for (i=0;i<NUMBER_OF_ROOMS*30;i++)
			if (door_api_is_related(edited.fg[i]&TILE_MASK)==cButton)
				if (edited.bg[i]>pivot && edited.bg[i]!=255)
					editor__do(bg[i],edited.bg[i]+1,mark_middle);

		/* 3) insert the link */
		(*max_doorlinks)++;
		set_doorlink(&doorlink,door,1);
		editor__do(doorlinks1[pivot],doorlink&0xff,mark_middle);
		editor__do(doorlinks2[pivot],(doorlink>>8)&0xff,mark_middle);
	}
	return 1;
}

void door_api_unlink(int* max_doorlinks, tile_global_location_type door,tile_global_location_type button) {
	/* read link */
	tile_global_location_type tile;
	short next;
	int i=edited.bg[button];
	if (i==255) return; /* error, button has no links */
	do {
		get_doorlink((edited.doorlinks2[i]<<8)|edited.doorlinks1[i],&tile,&next);
		i++;
	} while(next && door!=tile);
	i--;
	if (door!=tile) return; /* error, link not found */
	if (!next) { /* this is the last link */
		if (i==edited.bg[button]) { /* if the last link is the first, empty the list */
			editor__do(bg[button],255,mark_middle); /* remove link */
		} else { /* if there are more links before this one, set the !next bit in the previous one */
			get_doorlink((edited.doorlinks2[i-1]<<8)|edited.doorlinks1[i-1],&tile,&next);
			next=0;
			Uint16 aux;
			set_doorlink(&aux,tile,next);
			editor__do(doorlinks1[i-1],aux&0xff,mark_middle);
			editor__do(doorlinks2[i-1],(aux>>8)&0xff,mark_middle);
		}
	}
	/* update references */
	for (int j=0;j<NUMBER_OF_ROOMS*30;j++)
		if (door_api_is_related(edited.fg[j]&TILE_MASK)==cButton)
			if (edited.bg[j]>i && edited.bg[j]!=255) /* 255 is no link */
				editor__do(bg[j],edited.bg[j]-1,mark_middle);

	/* just shift the array one position */
	for (;i<*max_doorlinks;i++) {
		editor__do(doorlinks1[i],edited.doorlinks1[i+1],mark_middle);
		editor__do(doorlinks2[i],edited.doorlinks2[i+1],mark_middle);
	}

	(*max_doorlinks)--;
}

int door_api_fix_pos(tile_global_location_type* door,tile_global_location_type* button) {
	if (*door==-1 || *button==-1) return 0;
	byte tile_door=edited.fg[*door]&TILE_MASK;
	byte tile_button=edited.fg[*button]&TILE_MASK;
	tTileDoorType door_type=door_api_is_related(tile_door);
	tTileDoorType button_type=door_api_is_related(tile_button);
	if (door_type==button_type || !door_type || !button_type) return 0;
	if (door_type==cButton) { /* Swap */
		tile_global_location_type aux=*door;
		*door=*button;
		*button=aux;
		tile_door=edited.fg[*door]&TILE_MASK; /* refresh tile_door after swapping */
	}
	if (tile_door==tiles_17_level_door_right) { /* door right case */
		if (
				(*door)%10 &&
				(edited.fg[(*door)-1]&TILE_MASK)==tiles_16_level_door_left
		) {
			(*door)--;
		}
	}
	return 1;
}

/* editor functions related to this api */

int selected_door_tile=-1;
void editor__save_door_tile(tile_global_location_type tile) { /* if the same tile was selected "unselect" if not "select this tile".*/
	if ((edited.fg[tile]&TILE_MASK)==tiles_17_level_door_right) {
			if (tile%10 &&
				(edited.fg[tile-1]&TILE_MASK)==tiles_16_level_door_left) {
					tile--;
			} else {
				return;
			}
	}

	selected_door_tile=(selected_door_tile==tile)?-1:tile;
}
/* debug function
void printl() {
	int i;
	for (i=0;i<=edited_doorlinks;i++) {
		tile_global_location_type tile;
		short next;
		get_doorlink((edited.doorlinks2[i]<<8)|edited.doorlinks1[i],&tile,&next);
		printf("i=%d r=%d,t=%d next=%d\n",i,tile.room,tile.tilepos,next);
	}
}*/

const char* editor__toggle_door_tile(short room,short tilepos) {
	tile_global_location_type door=T(room,tilepos);
	tile_global_location_type button=selected_door_tile;
	if (!door_api_fix_pos(&door,&button)) return "Select a button and a door"; /* Error */

	tIterator it;
	tile_global_location_type check_tile;
	door_api_init_iterator(&it,door);
	while(door_api_get(&it,&check_tile)) /* check if existent to unlink */
		if (check_tile==button) {
			door_api_unlink(&edited_doorlinks,door,button);
			return "Door unlinked";
		}

	if (door_api_link(&edited_doorlinks,door,button))
		return "Door linked";

	return "Link error";
}

/********************************************\
*               INI functions                *
\********************************************/
int ini_load(const char *filename,
             int (*report)(const char *section, const char *name, const char *value));

typedef enum {
	cMain=1,    /* White+grey: cursors cross+tile. */
	cRight=3,   /* Red: cursor tile when ctrl+alt is pressed and the tile is a door/button. */
	cWrong=5,   /* Blue: cursor tile when ctrl+alt is pressed and the tile is NOT a door/button. No actions possible. */
	cLinked=7,  /* Green: When a door/button is selected, the linked tiles will have this color */
	cSelected=9,/* Yellow: The selected door/button. */
	cRandom=11, /* Cyan: Before randomization. */
	cTileSel=13 /* Orange: Selected tile. */
} tCursorColors; /* The palettes are taken from the res201 bitmap, ignoring the res200 palette. */

typedef enum {
	cCross=0,
	cSingleTile=1,
	cExitdoor=4,
	cBigPillar=7,
	cSmallTiles=10,
	cSmallCharacters=11,
	aFeather=12,
	aFlip=13,
	aLoose=14,
	aLife=15,
	aPlaneVertical=16,
	aPlaneHorizontal=19
} tEditorImageOffset;

typedef struct  {
	tile_packed_type tile,mask;
	tEditorImageOffset res;
} ambi_type;

typedef union {
	struct {
		tile_packed_type tile,mask;
	} by_mask;
	Uint32 by_flags;
} match_type;

typedef struct {
	char type,by,level;
	match_type match;
	tile_packed_type new_tile;
	tile_packed_type new_mask;
} sani_c_tile_type;

typedef struct {
	char type,by,x,y,null_is_true;
	match_type match;
} sani_tile_type;

typedef union {
	char type;
	sani_c_tile_type center;
	sani_tile_type adjacent;
} sani_type;

struct {
	int ambi_count;
	ambi_type ambi[50];
	int sani_count;
	int sani_alloc;
	sani_type* sani;
} editor_tables;

void add_sani(sani_type s) {
	if (!editor_tables.sani_alloc) {
		editor_tables.sani_alloc=100;
		editor_tables.sani=malloc(editor_tables.sani_alloc*sizeof(sani_type));
	} else if (editor_tables.sani_count==editor_tables.sani_alloc) {
		editor_tables.sani_alloc<<=1;
		editor_tables.sani=realloc(editor_tables.sani,editor_tables.sani_alloc*sizeof(sani_type));
	}

	editor_tables.sani[editor_tables.sani_count++]=s;
}

int parse_match(const char* str,match_type* mt, char* by) {
	int t1,t2,m1,m2;
	if (sscanf(str,"%d.%d/%d.%d",&t1,&t2,&m1,&m2)) {
		mt->by_mask.tile=TP_(t1,t2);
		mt->by_mask.mask=TP_(m1,m2);
		*by=0;
	} else {
		char str2[100];
		if (sscanf(str,"(%[^)])",str2)) {
			char* token;
			char* init=str2;
			mt->by_flags=0;
			while((token = strtok(init,","))) {
				mt->by_flags|=1<<atoi(token);
				init=NULL;
			}
			*by=1;
		} else {
			printf("editor.ini matching error '%s'\n",str);
			return 0;
		}
	}
	return 1;
}

int ini_editor_callback(const char *section, const char *name, const char *value) {
	if (!strcmp(section,"ambiguous")) {
		int fg,bg,fgm,bgm;
		int res,c;
		c=editor_tables.ambi_count;
		if (sscanf(value,"%d.%d/%d.%d %d",&fg,&bg,&fgm,&bgm,&res)) {
			editor_tables.ambi[c].tile=TP_(fg,bg);
			editor_tables.ambi[c].mask=TP_(fgm,bgm);
			editor_tables.ambi[c].res=res-1;
			editor_tables.ambi_count++;
		} else {
			printf("editor.ini: parsing error for %s: '%s'\n",name,value);
			quit(0);
		}
	} else if (!strcmp(section,"sanitation")) {
		if (!strcmp(name,"change")) { //change=<?match>([0-9]*.[0-9]*/[0-9]*.[0-9]*|\([0-9]*(,[0-9]*)*\)) <?replacement>([0-9]*.[0-9]*/[0-9]*.[0-9]*) <?level>[0-9]
			char str[100];
			int nt1,nt2,nm1,nm2,level;
			if (sscanf(value,"%[^ ] %d.%d/%d.%d %d",str,&nt1,&nt2,&nm1,&nm2,&level)) {
				sani_type aux;
				aux.type=0; //central tile
				if (!parse_match(str,&aux.center.match,&aux.center.by)) quit(0);
				aux.center.level=level;
				aux.center.new_tile=TP_(nt1,nt2);
				aux.center.new_mask=TP_(nm1,nm2);
				add_sani(aux);
			} else {
				printf("editor.ini: parsing error for %s: '%s'\n",name,value);
				quit(0);
			}
		} else if (!strcmp(name,"match")) { //match=<?match>([0-9]*.[0-9]*/[0-9]*.[0-9]*|\([0-9]*(,[0-9]*)*\)) <?where>([udlr]*) <?does_null_match>[yn]
			char str[100];
			char str2[20];
			char str3[4];
			if (sscanf(value,"%[^ ] %[udlr] %[yn]",str,str2,str3)) {
				sani_type aux;
				aux.type=1; //adjacent tile
				if (!parse_match(str,&aux.adjacent.match,&aux.adjacent.by)) quit(0);
				Uint8 x=0,y=0;
				const char* i=str2;
				while (*i) switch(*(i++)){
				case 'u':y--;break;
				case 'd':y++;break;
				case 'l':x--;break;
				case 'r':x++;break;
				default:printf("editor.ini: the code '%c' from '%s' is expected to be a,b,l or r\n",*(i-1),str2);quit(0);
				}
				aux.adjacent.x=x;
				aux.adjacent.y=y;
				aux.adjacent.null_is_true=str3[0]=='y';
				add_sani(aux);
			} else {
				printf("editor.ini: parsing error for %s: '%s'\n",name,value);
				quit(0);
			}
		}
	}
	return 1;
}

/********************************************\
*              Editor functions              *
\********************************************/

chtab_type* chtab_editor_sprites=NULL;
void editor__load_level() {
	dat_type* dathandle;

	level=edited;
	edition_level=current_level;
	stack_reset();
	selected_door_tile=-1;
	remember_room=0;

	dathandle = open_dat("editor", 0);
	if (chtab_editor_sprites) {
		free_chtab(chtab_editor_sprites);
	} else {
		//TODO: move the ini to another place (hook it in the init game)
		memset(&editor_tables,0,sizeof(editor_tables));
		ini_load("data/editor/editor.ini", ini_editor_callback);
	}
	chtab_editor_sprites = load_sprites_from_file(200, 1<<11, 1);
	close_dat(dathandle);

	/* TODO: free_chtab(chtab_editor_sprites); */
}

#define copy_block(field,size,type) memcpy(&dst->field,&src->field,((size)*sizeof(type)))
#define level_copy_function(name,dst_type,src_type) \
void name(dst_type* dst, src_type* src) { \
	copy_block(fg,NUMBER_OF_ROOMS * 30,byte); \
	copy_block(bg,NUMBER_OF_ROOMS * 30,byte); \
	copy_block(doorlinks1,256,byte); \
	copy_block(doorlinks2,256,byte); \
	copy_block(roomlinks,NUMBER_OF_ROOMS,link_type); \
	copy_block(used_rooms,1,byte); \
	copy_block(roomxs,NUMBER_OF_ROOMS,byte); \
	copy_block(roomys,NUMBER_OF_ROOMS,byte); \
	copy_block(fill_1,15,byte); \
	copy_block(start_room,1,byte); \
	copy_block(start_pos,1,byte); \
	copy_block(start_dir,1,sbyte); \
	copy_block(fill_2,4,byte); \
	copy_block(guards_tile,NUMBER_OF_ROOMS,byte); \
	copy_block(guards_dir,NUMBER_OF_ROOMS,byte); \
	copy_block(guards_x,NUMBER_OF_ROOMS,byte); \
	copy_block(guards_seq_lo,NUMBER_OF_ROOMS,byte); \
	copy_block(guards_skill,NUMBER_OF_ROOMS,byte); \
	copy_block(guards_seq_hi,NUMBER_OF_ROOMS,byte); \
	copy_block(guards_color,NUMBER_OF_ROOMS,byte); \
	copy_block(fill_3,18,byte); \
}
level_copy_function(editor__extend_level,level_type,level_real_type);
level_copy_function(editor__simplify_level,level_real_type,level_type);

int load_resource(const char* file, int res, void* data, int size, const char* ext);
void save_resource(const char* file, int res, const void* data, int size, const char* ext);

void load_edit_palettes(level_type* level_to_load) {
	load_resource("editor",100,&(level_to_load->fg[NUMBER_OF_ROOMS*30]),8*30, "bin");
	load_resource("editor",101,&(level_to_load->bg[NUMBER_OF_ROOMS*30]),8*30, "bin");
	if (!load_resource("editor",102,&(level_to_load->roomlinks[NUMBER_OF_ROOMS]),8*sizeof(link_type), "bin"))
		for (int a=24;a<24+8;a++) {
			level_to_load->roomlinks[a].left=(a==24)?0:a-1+1;
			level_to_load->roomlinks[a].right=(a==24+7)?0:a+1+1;
			level_to_load->roomlinks[a].up=(a<24+4)?0:a-4+1;
			level_to_load->roomlinks[a].down=(a>=24+4)?0:a+4+1;
		}
	for (int i=NUMBER_OF_ROOMS;i<NUMBER_OF_ROOMS+8;i++)
		level_to_load->guards_tile[i]=30;
}

void save_edit_palettes() {
	save_resource("editor",100,&(edited.fg[NUMBER_OF_ROOMS*30]),8*30, "bin");
	save_resource("editor",101,&(edited.bg[NUMBER_OF_ROOMS*30]),8*30, "bin");
	save_resource("editor",102,&(edited.roomlinks[NUMBER_OF_ROOMS]),8*sizeof(link_type), "bin");
}

void editor__load_dat_file(level_type* l) {
	level_real_type aux;

	dat_type* dathandle;
	dathandle = open_dat("LEVELS.DAT", 0);
	load_from_opendats_to_area(current_level + 2000, &aux, sizeof(aux), "bin");
	close_dat(dathandle);

	/* set up extended level */
	memset(l,0,sizeof(level_type));
	editor__extend_level(l,&aux);
	load_edit_palettes(l);
}

void editor__loading_dat() {
	remember_room=0;

	if (edition_level!=current_level) {
		editor__load_dat_file(&edited);
		editor__load_level();
		room_api_free(&edited_map);
		door_api_free(&edited_doorlinks);
		room_api_init(&edited_map);
		door_api_init(&edited_doorlinks);
	} else {
		level=edited;
	}
}

void editor_revert_level() {
	editor__load_dat_file(&edited);
	editor__load_level();
	level=edited;
	room_api_free(&edited_map);
	door_api_free(&edited_doorlinks);
	room_api_init(&edited_map);
	door_api_init(&edited_doorlinks);
}

void editor__position(char_type* character,int col,int row,int room,int x,word seq) {
	character->curr_col=col;
	character->curr_row=row;
	character->room=room;
	character->x=x;
	character->y=55+63*row;
	character->curr_seq=seq;
	if (character->direction == dir_56_none) character->direction = dir_0_right;
}

void editor__paste_room(int room) { //TODO: add tilepos
	int i;
	editor__do_mark_start(flag_redraw);
	for (i=0;i<30;i++)
		if (clipboard[i].number!=NO_TILE.number)
			editor_change_tile(T(drawn_room,i+((clipboard_has==chTiles)?clipboard_shift:0)),clipboard[i]);
	editor__do_mark_end(flag_redraw);
}

void save_resource(const char* file, int res, const void* data, int size, const char* ext) {
	char aux[255];
	FILE* fp;
	snprintf(aux,255,"data/%s/res%d.%s",file,res,ext);
	fp=fopen(aux,"wb");
	if (fp) {
		fwrite(data,size,1,fp);
		fclose(fp);
	} else {
		printf("error opening '%s'\n",aux);
	}
}

int load_resource(const char* file, int res, void* data, int size, const char* ext) {
	char aux[255];
	FILE* fp;
	snprintf(aux,255,"data/%s/res%d.%s",file,res,ext);
	fp=fopen(aux,"rb");
	if (fp) {
		fread(data,size,1,fp);
		fclose(fp);
		return 1;
	} else {
		printf("error opening '%s'\n",aux);
		return 0;
	}
}

void save_level() {
	level_real_type aux;
	editor__simplify_level(&aux,&edited);
	save_resource("LEVELS.DAT",current_level + 2000, &aux, sizeof(aux), "bin");
}

void editor__set_guard(byte tilepos,byte x) {
	if (level.guards_tile[loaded_room-1]>=30) { //if not present: create
		editor__do(guards_tile[loaded_room-1],tilepos,mark_start|flag_guard_presence);
		editor__do(guards_color[loaded_room-1],1,mark_middle);
		editor__do(guards_x[loaded_room-1],x,mark_middle);
		editor__do(guards_dir[loaded_room-1],0,mark_middle);
		editor__do(guards_seq_hi[loaded_room-1],0,mark_middle);
		editor__do(guards_skill[loaded_room-1],0,mark_end|flag_guard_presence);
	} else { //if present move
		editor__do(guards_tile[loaded_room-1],tilepos,mark_start|flag_guard_presence);
		editor__do(guards_x[loaded_room-1],x,mark_end|flag_guard_presence);
	}
	enter_guard(); // load color, HP
}

void editor__synchronize_guard_repaint() {
	curr_guard_color = edited.guards_color[loaded_room-1];
	set_chtab_palette(chtab_addrs[id_chtab_5_guard], &guard_palettes[0x30 * curr_guard_color - 0x30], 0x10);
	draw_guard_hp(guardhp_curr,guardhp_max);
}

void editor__synchronize_guard_presence() {
	if (edited.guards_tile[loaded_room-1]>=30) { //not present
		draw_guard_hp(0, 10); // delete HP
		Guard.direction = dir_56_none; // delete guard from screen
	} else { //repositon guard if deleted and undo
		int tilepos=edited.guards_tile[loaded_room-1];
		int x=edited.guards_x[loaded_room-1];
		if (x==255) {
			x=x_bump[tilepos % 10 + 5] + 14;
			level.guards_x[loaded_room-1]=x; //re-initialize guard after undo/redo
		}
		editor__position(&Guard,tilepos%10,tilepos/10,loaded_room,x,6569);
		Guard.direction=edited.guards_dir[loaded_room-1];
		editor__synchronize_guard_repaint();
	}
}

void editor__remove_guard() {
	editor__do(guards_tile[loaded_room-1],30,mark_all|flag_guard_presence);
	editor__synchronize_guard_presence();
}

int editor__guard_skill(int delta) {
	int new_skill=level.guards_skill[loaded_room-1]+delta;
	if (0<=new_skill && new_skill<=20 && level.guards_tile[loaded_room-1]<30) {
		editor__do(guards_skill[loaded_room-1],new_skill,mark_all);
		return new_skill;
	}
	return -1;
}

int editor__guard_color(int delta) {
	int new_color=level.guards_color[loaded_room-1]+delta;
	if (1<=new_color && new_color<=7 && level.guards_tile[loaded_room-1]<30) {
		editor__do(guards_color[loaded_room-1],new_color,mark_all|flag_guard_repaint);
		// If I call redraw_screen() or enter_guard() directly then the kid changes into a guard...
		editor__synchronize_guard_repaint();
		return new_color;
	}
	return -1;
}

void editor__guard_toggle() {
	if (level.guards_tile[loaded_room-1]<30) {
		editor__do(guards_dir[loaded_room-1],~level.guards_dir[loaded_room-1],mark_all|flag_guard_presence);
	}
}

/********************************************\
*        Room randomizer & sanitizer         *
\********************************************/

void sanitize_room(int room, int sanitation_level);
void randomize_room(int room) {
	byte level_mask[NUMBER_OF_ROOMS*30];
	float f;
	tile_packed_type tt;
	tile_global_location_type i,tile=-1;
	float max=0;

	memset(level_mask,1,NUMBER_OF_ROOMS*30);
	memset(level_mask+T(room,0),0,30);

	do {
		tile=-1;
		max=0;
		for (i=0;i<NUMBER_OF_ROOMS*30;i++)
			if (!level_mask[i] && edited_map.list[R_(i)]) {
				f=room_api_measure_entropy(&edited_map,i,level_mask);
				if (max<f) {
					max=f;
					tile=i;
				}
			}
		if (tile!=-1) {
			tt=room_api_suggest_tile(&edited_map,tile,level_mask);
			editor_change_tile(tile,tt);
			level_mask[tile]=1;
		}
	} while (tile!=-1);
	sanitize_room(room,0);
	sanitize_room(room,1);
}

void randomize_tile(int tilepos) {
	byte level_mask[NUMBER_OF_ROOMS*30];
	tile_global_location_type t=T(loaded_room,tilepos);
	tile_packed_type tt;
	memset(level_mask,1,NUMBER_OF_ROOMS*30);
	level_mask[t]=0;
	tt=room_api_suggest_tile(&edited_map,t,level_mask);
	editor__do_mark_start(flag_redraw);
	editor_change_tile(t,tt);
	editor__do_mark_end(flag_redraw);
}

void print_match(match_type m,int by) {
	if (!by) {
		printf("by mask: t=%x m=%x\n",m.by_mask.tile.number,m.by_mask.mask.number);
	} else {
		printf("by flag %x: ",m.by_flags);
		for (int i=0;i<32;i++) if ((m.by_flags>>i)&1) printf("%d,",i);
		printf("\n");
	}
}

int matches(match_type m,int by,tile_packed_type t) {
	if (!by) { /* by mask */
		return (t.number&m.by_mask.mask.number)==m.by_mask.tile.number;
	} else { /* by flag */
		return (1<<(t.concept.fg&TILE_MASK))&m.by_flags;
	}
}

void sanitize_room(int room, int sanitation_level) {
/* debug:
		for (int j=0;j<editor_tables.sani_count;j++) {
			sani_type aux=editor_tables.sani[j];
			if (!aux.type) { //center
				printf("change by=%d level=%d t=%x m=%x ",aux.center.by,aux.center.level,aux.center.new_tile.number,aux.center.new_mask.number);
				print_match(aux.center.match,aux.center.by);
			} else {
				printf("match xy=(%d,%d) null is true?=%d ",aux.adjacent.x,aux.adjacent.y,aux.adjacent.null_is_true);
				print_match(aux.adjacent.match,aux.adjacent.by);
			}
		}
*/
	int i;
	for (i=0;i<30;i++) {
		tile_global_location_type t=T(room,i);
		int match=1;
		for (int j=0;j<editor_tables.sani_count;j++) {
			sani_type aux=editor_tables.sani[j];
			if (!aux.type) { //center
				tile_packed_type current=TP(edited,t);
				if (match&&aux.center.level<=sanitation_level&&matches(aux.center.match,aux.center.by,current))
					editor_change_tile(t,(tile_packed_type){.number=( (current.number&(~aux.center.new_mask.number)) | (aux.center.new_tile.number&aux.center.new_mask.number) )});
				match=1; /* restart */
			} else {
				if (match) {
					tile_global_location_type match_t=room_api_tile_move(&edited_map,t,aux.adjacent.x,aux.adjacent.y);
					if (match_t==-1) {
						match=aux.adjacent.null_is_true;
					} else {
						match=matches(aux.adjacent.match,aux.adjacent.by,TP(edited,match_t));
					}
				}
			}
		}
	}
}

void editor__randomize(int room) {
	editor__do_mark_start(flag_redraw);
	randomize_room(room);
	editor__do_mark_end(flag_redraw);
	ed_select_room(room);
	ed_redraw_room();
	need_full_redraw=1;
}

int editor__copy_room(int room) {
	int select_all_room=1;
	if (room==selected_mask_room) /* only use selected tiles if the user is in the selection room */
		for (int i=0;i<30;i++)
			select_all_room=select_all_room&&selected_mask[i];

	for (int i=0;i<30;i++)
		clipboard[i]=(select_all_room||selected_mask[i])?TP(edited,T(drawn_room,i)):NO_TILE;

	clipboard_has=select_all_room;
	return select_all_room;
}
void editor__clean_room(int room) {
	tile_packed_type empty={.number=0};
	editor__do_mark_start(flag_redraw);
	for (int i=0;i<30;i++)
		if (selected_mask[i])
			editor_change_tile(T(room,i),empty);
	editor__do_mark_end(flag_redraw);
	ed_redraw_room();
	need_full_redraw=1;
}

/********************************************\
*           Blitting editor layer            *
\********************************************/

/* blit top surface layer (cursor+annotations) */
void blit_sprites(int x,int y, tEditorImageOffset sprite, tCursorColors colors, int colors_total, surface_type* screen) {
	image_type* image;
	SDL_Rect src_rect= {0, 0, 0 , 0};
	SDL_Rect dest_rect = {0, 0, 0, 0};

	image=chtab_editor_sprites->images[sprite];
	dest_rect.x=x;
	dest_rect.y=y;
	dest_rect.w=src_rect.w=image->w;
	dest_rect.h=src_rect.h=image->h;
	SDL_SetSurfaceBlendMode(image, SDL_BLENDMODE_NONE);
	SDL_SetSurfaceAlphaMod(image, 255);
	SDL_SetColorKey(image, SDL_TRUE, 0);

	if (colors_total!=-1) {
		if (SDL_SetPaletteColors(image->format->palette, chtab_editor_sprites->images[0]->format->palette->colors+colors, 1, colors_total) != 0) {
			printf("Couldn't set video mode: %s\n", SDL_GetError());
		}
	}

	if (SDL_BlitSurface(image, &src_rect, screen, &dest_rect) != 0) {
		sdlperror("SDL_BlitSurface on editor");
		quit(1);
	}
}

/* private structure */
typedef struct {
	int inside;
	int col,row,tilepos,x,y,x_b;
	int keys;
	Uint32 buttons;
} mouse_type;

void draw_clipboard(surface_type* screen,int movement, mouse_type mouse) {
	if (clipboard_has!=chTiles) return;
	if (mouse.keys!=k_ctrl) return;
	if (!mouse.inside) return;

	rect_type crop={100,100,-100,-100};
	for (int i=0;i<30;i++)
		if (clipboard[i].number!=NO_TILE.number) {
			int col,row;

			col=i%10;
			row=i/10;

			if (col<crop.left) crop.left=col;
			if (col>crop.right) crop.right=col;
			if (row<crop.top) crop.top=row;
			if (row>crop.bottom) crop.bottom=row;
		}

	/* assert */
	if (crop.top==100) {clipboard_has=chNothing;return;}

	int new_col=mouse.col-(crop.right+crop.left+1)/2;
	int new_row=mouse.row-(crop.bottom+crop.top+1)/2;
	if (crop.left+new_col<0) new_col=-crop.left;
	if (crop.top+new_row<0) new_row=-crop.top;
	if (crop.right+new_col>9) new_col=9-crop.right;
	if (crop.bottom+new_row>2) new_row=2-crop.bottom;
	clipboard_shift=10*new_row+new_col;

	for (int i=0;i<30;i++)
		if (clipboard[i].number!=NO_TILE.number) {
			int col,row;
			int x,y;

			col=i%10;
			row=i/10;

			x=(col+new_col)*32;
			y=(row+new_row)*63-10;

			int
				u=row!=0&&(clipboard[i-10].number!=NO_TILE.number),
				d=row!=2&&(clipboard[i+10].number!=NO_TILE.number),
				l=col!=0&&(clipboard[i-1].number!=NO_TILE.number),
				r=col!=9&&(clipboard[i+1].number!=NO_TILE.number);
			//up
			blit_sprites(x,y+u,aPlaneHorizontal+movement,cSelected+u,1,screen);
			blit_sprites(x-32+l,y,aPlaneVertical+movement,cSelected+l,1,screen);
			if (!d) blit_sprites(x,y+63,aPlaneHorizontal+movement,cSelected,1,screen);
			if (!r) blit_sprites(x,y,aPlaneVertical+movement,cSelected,1,screen);
		}

}
void draw_selected(surface_type* screen,int movement){
	for (int i=0;i<30;i++)
		if (selected_mask[i] && selected_mask_room==drawn_room) {
			int col,row;
			int x,y;

			col=i%10;
			row=i/10;

			x=col*32;
			y=row*63-10;

			int
				u=row!=0&&(selected_mask[i-10]),
				d=row!=2&&(selected_mask[i+10]),
				l=col!=0&&(selected_mask[i-1]),
				r=col!=9&&(selected_mask[i+1]);
			//up
			blit_sprites(x,y+u,aPlaneHorizontal+movement,cTileSel+u,1,screen);
			blit_sprites(x-32+l,y,aPlaneVertical+movement,cTileSel+l,1,screen);
			if (!d) blit_sprites(x,y+63,aPlaneHorizontal+movement,cTileSel,1,screen);
			if (!r) blit_sprites(x,y,aPlaneVertical+movement,cTileSel,1,screen);
		}
}

void draw_ambiguous_on(surface_type* screen, tile_packed_type tile, int x, int y) {
	int i;
	for (i=0;i<editor_tables.ambi_count;i++)
		if ((tile.number&editor_tables.ambi[i].mask.number)==editor_tables.ambi[i].tile.number)
			blit_sprites(x,y,editor_tables.ambi[i].res,0,-1,screen);
}

void name_tile(char* res, int n, tile_packed_type tile, const char* format) {
	snprintf(res,n,format,tile.concept.fg&TILE_MASK, tile.concept.fg>>5, tile.concept.bg);
}

void draw_ambiguous_full(surface_type* screen, tile_packed_type tile, int x, int y) {
	/* draw text*/
	char aux[40];
	rect_type r={y,x,y+76,x+44};
	screen_updates_suspended=1;
	surface_type* save_screen=current_target_surface;
	current_target_surface=screen;
	name_tile(aux,40,tile,"%d\n%d\n\n%d");
	show_text_with_color(&r,0,0,aux,15);
	current_target_surface=save_screen;
	screen_updates_suspended=0;
}

void draw_ambiguous(surface_type* screen){
	if (!ambiguous_mode && !PALETTE_MODE) return;
	/* ambiguous is always on in palette mode */

	for (int i=0;i<30;i++) {
		int col,row;
		int x,y;

		col=i%10;
		row=i/10;

		x=col*32;
		y=row*63-10;

		tile_packed_type tile=TP(edited,T(drawn_room,i));

		if (ambiguous_mode==2) {
			draw_ambiguous_full(screen,tile,x,y);
		} else { /* ambiguous_mode==2 or PALETTE_MODE */
			draw_ambiguous_on(screen,tile,x,y);
		}

	}
}


void highlight_room(surface_type* screen, int offsetx,int offsety, int tw, int th, int i, int j, Uint32 color) {
	SDL_Rect line1={offsetx+(tw*10+1)*i,offsety+(th*3+1)*j,tw*10+2,1};
	SDL_Rect line2={offsetx+(tw*10+1)*i,offsety+(th*3+1)*j,1,th*3+2};
	SDL_Rect line3={offsetx+(tw*10+1)*i,offsety+(th*3+1)*(j+1),tw*10+2,1};
	SDL_Rect line4={offsetx+(tw*10+1)*(i+1),offsety+(th*3+1)*j,1,th*3+2};
	SDL_FillRect(screen,&line1,color);
	SDL_FillRect(screen,&line2,color);
	SDL_FillRect(screen,&line3,color);
	SDL_FillRect(screen,&line4,color);
}

mouse_type calculate_mouse(const Uint8* key_states) {
	mouse_type mouse;
	mouse.keys=
		(key_states[SDL_SCANCODE_LSHIFT] || key_states[SDL_SCANCODE_RSHIFT]) * k_shift |
		(key_states[SDL_SCANCODE_LCTRL] || key_states[SDL_SCANCODE_RCTRL])   * k_ctrl  |
		(key_states[SDL_SCANCODE_LALT] || key_states[SDL_SCANCODE_RALT])     * k_alt   |
		(key_states[SDL_SCANCODE_M])                                         * k_m     ;

	mouse.buttons=GetUnscaledMouseState(&mouse.x,&mouse.y);
	if ((mouse.x>0) && (mouse.y>=3) && (mouse.x<319) && (mouse.y<192)) {
		mouse.col=mouse.x/32;
		mouse.row=(mouse.y-3)/63;
		mouse.x_b=mouse.x*140/320+62; //x_bump[col + 5] + 14;
		//x_bump[n] === -12+14*n --> x_bump[col + 5] + 14 === -12+14*(col + 5) + 14 === 14*(x/32) + 72 === (140*x)/320+72
		mouse.inside=!(mouse.row<0 || mouse.row>2);
		mouse.tilepos=mouse.row*10+mouse.col;
	} else {
		mouse.inside=0;
	}
	return mouse;
}

/********************************************\
*             INPUT BINDINGS!!!!             *
\********************************************/

void editor__on_refresh(surface_type* screen) {
	if (chtab_editor_sprites) {
		mouse_type mouse=calculate_mouse(SDL_GetKeyboardState(NULL));
		int colors_total=1;
		tEditorImageOffset image_offset=cSingleTile;
		tCursorColors colors;

		if (PALETTE_MODE)
			mouse.keys=mouse.keys&k_m;

		if (mouse.inside) {
			colors=cMain;
			/* if Shift is pressed a cross is shown */
			if (mouse.keys==k_shift || mouse.keys&k_m) {
				image_offset=cCross;
				mouse.x-=chtab_editor_sprites->images[cCross]->w/2;
				mouse.y-=chtab_editor_sprites->images[cCross]->h/2;
				//colors=cMain;
			} else { /* If not, a 3D selection box is shown. When alt is pressed cRight or cWrong colors are used */
				if (mouse.keys==(k_ctrl|k_alt)) colors=cWrong;
				if ((mouse.keys|k_shift)==(k_ctrl|k_shift)) colors=cRandom;
				mouse.x=mouse.col*32;
				mouse.y=mouse.row*63-10;
				switch(level.fg[T(drawn_room,mouse.tilepos)]&TILE_MASK) {
				case tiles_17_level_door_right:
				case tiles_24_balcony_right:
					mouse.x-=32;
				case tiles_16_level_door_left:
				case tiles_23_balcony_left:
					image_offset=cExitdoor;
					colors_total=2;
					if (mouse.keys==(k_ctrl|k_alt)) colors=cRight;
					break;
				case tiles_8_bigpillar_bottom:
					mouse.y-=63;
				case tiles_9_bigpillar_top:
					image_offset=cBigPillar;
					colors_total=2;
					break;
				case tiles_6_closer:
				case tiles_15_opener:
				case tiles_4_gate:
					if (mouse.keys==(k_ctrl|k_alt)) colors=cRight;
					break;
				}

				static unsigned short i_frame_clockwise=0; /* What's this? There are two different layers
				                                              of selected boxes, the fixed ones and the
				                                              cursor/pointer ones; when a tile is selected
				                                              with both layers as they are animated clockwise
				                                              and anticlockwise both selections are visible */
				image_offset+=(i_frame_clockwise++)%3;

				if (mouse.keys==(k_ctrl|k_alt) && colors==cRight) { /* show number of links */
					char text_aux[20];
					int count=0;
					/* count links */
					tIterator it;
					tile_global_location_type current_tile,junk_tile;
					current_tile=T(drawn_room,mouse.tilepos);
					door_api_init_iterator(&it,current_tile);
					while(door_api_get(&it,&junk_tile))
						count++;
					sprintf(text_aux,"%d",count);

					/* draw text*/
					rect_type r={mouse.y,mouse.x,mouse.y+63,mouse.x+32*colors_total};
					screen_updates_suspended=1;
					surface_type* save_screen=current_target_surface;
					current_target_surface=screen;
					show_text_with_color(&r,0,0,text_aux,4);
					current_target_surface=save_screen;
					screen_updates_suspended=0;
				}
			}

			blit_sprites(mouse.x,mouse.y,image_offset,colors,colors_total,screen);
		}

		static unsigned short i_frame_anticlockwise=0;
		i_frame_anticlockwise--;
		/* draw selected door tiles */
		if (selected_door_tile!=-1) {
			if (R(selected_door_tile)==loaded_room) {
				colors_total=1;
				if ((level.fg[selected_door_tile]&TILE_MASK)==tiles_16_level_door_left) {
					colors_total=2;
					image_offset=cExitdoor;
				} else {
					colors_total=1;
					image_offset=cSingleTile;
				}
				blit_sprites((P(selected_door_tile)%10)*32,(P(selected_door_tile)/10)*63-10,image_offset+(i_frame_anticlockwise)%3,cSelected,colors_total,screen);
			}
			tIterator it;
			tile_global_location_type linked;
			door_api_init_iterator(&it,selected_door_tile);
			while(door_api_get(&it,&linked)) {
				if (R(linked)==loaded_room) { /* there is a linked tile in the drawn room */
					if ((level.fg[linked]&TILE_MASK)==tiles_16_level_door_left) {
						colors_total=2;
						image_offset=cExitdoor;
					} else {
						colors_total=1;
						image_offset=cSingleTile;
					}
					blit_sprites((P(linked)%10)*32,(P(linked)/10)*63-10,image_offset+(i_frame_anticlockwise)%3,cLinked,colors_total,screen);
				} else { /* there is a linked tile in the last column of the left room */
					if (level.roomlinks[loaded_room-1].left==R(linked) && P(linked)%10==9) {
						/* TODO: exit doors */
						blit_sprites((-1)*32,(P(linked)/10)*63-10,cSingleTile+(i_frame_anticlockwise)%3,cLinked,1,screen);
					}
				}
			}
		}

		/* draw ambiguous information */
		draw_ambiguous(screen);

		/* draw selected tiles */
		draw_selected(screen,i_frame_anticlockwise%3);
		draw_clipboard(screen,i_frame_anticlockwise%3,mouse);

		/* draw map */
		if (mouse.keys&k_m) {
			image_type* image=chtab_editor_sprites->images[cSmallTiles];
			image_type* people=chtab_editor_sprites->images[cSmallCharacters];

			SDL_SetSurfaceBlendMode(image, SDL_BLENDMODE_NONE);
			SDL_SetSurfaceAlphaMod(image, 255);
			SDL_SetColorKey(image, SDL_FALSE, 0);
			SDL_SetSurfaceBlendMode(people, SDL_BLENDMODE_NONE);
			SDL_SetSurfaceAlphaMod(people, 255);
			SDL_SetColorKey(people, SDL_FALSE, 0);
			int tw,th,offsetx,offsety;
			int mw,mh,mi,mj;
			int sw,sh,sx,sy;
			int mode=0;

			if ((mouse.keys&(k_ctrl|k_alt))==(k_ctrl|k_alt)) mode=2;
			if (mouse.keys&k_shift) mode=1;
			if ((mouse.keys&(k_ctrl|k_shift))==(k_ctrl|k_shift)) mode=1;

			tw=image->w/32;
			th=image->h/4;
			room_api_get_size(&edited_map,&mw,&mh);
			sw=(mw/10)*(tw*10+1)+1;
			sh=(mh/3)*(th*3+1)+1;

			offsetx=(screen->w-sw)/2;
			offsety=(screen->h-sh)/2;

			SDL_Rect src_rect= {0, mode*th, tw , th};
			SDL_Rect src2_rect= {0, 0, tw , th};
			SDL_Rect dest_rect = {0, 0, tw, th};

			for (mj=0,sy=offsety;mj<mh;mj++) {
				if (!(mj%3)) {
					SDL_Rect line={offsetx,sy,sw,1};
					SDL_FillRect(screen,&line,0xffffff);
					sy++;
				}
				for (mi=0,sx=offsetx;mi<mw;mi++) {
					if (!(mi%10)) {
						SDL_Rect line={sx,offsety,1,sh};
						SDL_FillRect(screen,&line,0xffffff);
						sx++;
					}
					tile_global_location_type t=room_api_translate(&edited_map,mi,mj);
					if (t!=-1) {
						byte c=(edited.fg[t]&TILE_MASK);
						src_rect.x=tw*c;
						dest_rect.x=sx;
						dest_rect.y=sy;

						if (SDL_BlitSurface(image, &src_rect, screen, &dest_rect) != 0) {
							sdlperror("SDL_BlitSurface on editor showing map");
							quit(1);
						}

						if (mouse.keys==k_shift) {
							/* draw starting position */
							if (t==T(edited.start_room,edited.start_pos)) {
								src2_rect.x=tw*0;

								if (SDL_BlitSurface(people, &src2_rect, screen, &dest_rect) != 0) {
									sdlperror("SDL_BlitSurface on editor showing map");
									quit(1);
								}
							}

							/* draw kid current position (2 ticks yes, 2 ticks no) */
							if (t==T(Kid.room,(Kid.curr_col>0?Kid.curr_col:0)+10*Kid.curr_row) && (i_frame_anticlockwise&2)) {
								src2_rect.x=tw*0;

								if (SDL_BlitSurface(people, &src2_rect, screen, &dest_rect) != 0) {
									sdlperror("SDL_BlitSurface on editor showing kid on map");
									quit(1);
								}
							}

							/* draw guard positions */
							int v=edited.guards_tile[R_(t)];
							if (v<30 && v==P(t)) {
								src2_rect.x=tw*1;

								if (SDL_BlitSurface(people, &src2_rect, screen, &dest_rect) != 0) {
									sdlperror("SDL_BlitSurface on editor showing guard on map");
									quit(1);
								}
							}

						} /* /mouse.keys==(k_shift) */
					}
					sx+=tw;
				}
				sy+=th;
			}
			SDL_Rect lineH={offsetx,sy,sw,1};
			SDL_FillRect(screen,&lineH,0xffffff);
			SDL_Rect lineV={sx,offsety,1,sh};
			SDL_FillRect(screen,&lineV,0xffffff);

			{
			int i,j;
			if (room_api_get_room_location(&edited_map,drawn_room,&i,&j))
				highlight_room(screen,offsetx,offsety,tw,th,i,j,0xff8040);
			}

			mouse=calculate_mouse(SDL_GetKeyboardState(NULL)); //re-calculate x&y
			if (mouse.inside) {
				mouse.x-=offsetx;
				mouse.y-=offsety;
				if (0<mouse.x && mouse.x<sw && 0<mouse.y && mouse.y<sh) {
					int i=mouse.x/(tw*10+1);
					int j=mouse.y/(th*3+1);
					tile_global_location_type t=room_api_translate(&edited_map,i*10,j*3);
					if (t!=-1) { //room is R(t)
						map_selected_room=R(t);
						highlight_room(screen,offsetx,offsety,tw,th,i,j,0x55ff55);
					} else {
						map_selected_room=0;
					}
				}
			} else {
				map_selected_room=0;
			}
		}
	} else {
		printf("Sprites not loaded\n");
	}

}

void print(const char* text) {
	display_text_bottom(text);
	text_time_total = 24;
	text_time_remaining = 24;
}

void editor__handle_mouse_wheel(SDL_MouseWheelEvent e,mouse_type mouse);
void editor__handle_mouse_button(SDL_MouseButtonEvent e,mouse_type mouse);
void editor__handle_mouse_motion(SDL_MouseMotionEvent e,mouse_type mouse,mouse_type* last);

mouse_type last_keydown={0,0,0,0,0,0,0,0};
void editor__handle_mouse(SDL_Event e,const Uint8* states) {
	if (!editor_active) return;
	mouse_type mouse=calculate_mouse(states);
	if (mouse.inside)
		switch (e.type) {
		case SDL_MOUSEWHEEL:
			editor__handle_mouse_wheel(e.wheel,mouse);
			return;
		case SDL_MOUSEBUTTONDOWN:
			last_keydown=mouse;
			editor__handle_mouse_button(e.button,mouse);
			return;
		case SDL_MOUSEBUTTONUP:
			last_keydown.inside=0;
			return;
		case SDL_MOUSEMOTION:
			editor__handle_mouse_motion(e.motion,mouse,&last_keydown);
			return;
		}
}
void editor__handle_mouse_motion(SDL_MouseMotionEvent e,mouse_type mouse,mouse_type* last) {
	if (last->inside && mouse.inside && mouse.tilepos!=last->tilepos) {
		SDL_MouseButtonEvent e; //emulate a keypress
		if (last->buttons&SDL_BUTTON(SDL_BUTTON_LEFT)) {
			e.button=SDL_BUTTON_LEFT;
		} else {
			e.button=SDL_BUTTON_RIGHT; //TODO: check if exists a drag with right
		}
		editor__handle_mouse_button(e,mouse);
		*last=mouse;
	}
}

void editor__handle_mouse_wheel(SDL_MouseWheelEvent e,mouse_type mouse) {
	if (mouse.inside) {
		if (mouse.keys&k_ctrl) {
			e.x=e.y;
			e.y=0;
		}
		if (mouse.keys&k_alt) { /* precision for trackpads */
			if (e.x) e.x/=abs(e.x);
			if (e.y) e.y/=abs(e.y);
		}

		if (e.y!=0 || e.x!=0) {
			tile_global_location_type location=T(loaded_room,mouse.tilepos);
			if (e.y!=0) {
				byte v=(edited.fg[location]+e.y)&TILE_MASK;
				editor__do(fg[location],v,mark_start|flag_redraw);
				editor__do(bg[location],0,mark_end|flag_redraw);
			} else {
				byte v=(edited.bg[location]+e.x);
				editor__do(bg[location],v,mark_all|flag_redraw);
			}

			ed_redraw_tile(mouse.tilepos);
			if (mouse.tilepos) ed_redraw_tile(mouse.tilepos-1);
			if (mouse.tilepos!=29) ed_redraw_tile(mouse.tilepos+1);

			char aux[40];
			name_tile(aux,40,TP(edited,location),"FG:%d/%d BG:%d");
			print(aux);
			need_full_redraw=1;
		}
	}
}

void editor__handle_mouse_button(SDL_MouseButtonEvent e,mouse_type mouse) {
	if (!mouse.inside) return;

	if (e.button==SDL_BUTTON_LEFT && mouse.keys==k_none) { /* left click: edit tile */
		if (copied.main.number==NO_TILE.number) {
			print("NOTHING TO PASTE");
			return;
		}
		tile_global_location_type t=T(loaded_room,mouse.tilepos);
		editor__do_mark_start(flag_redraw);
		editor_change_tile(t,copied.main);
		switch(copied.extratype) {
		case extra_up:
			t=room_api_tile_move(&edited_map,t,0,-1);
			break;
		case extra_down:
			t=room_api_tile_move(&edited_map,t,0,1);
			break;
		case extra_left:
			t=room_api_tile_move(&edited_map,t,-1,0);
			break;
		case extra_right:
			t=room_api_tile_move(&edited_map,t,1,0);
			break;
		case extra_none:
			t=-1;
			break;
		}
		if (t!=-1)
			editor_change_tile(t,copied.extra);

		editor__do_mark_end(flag_redraw);
		ed_redraw_tile(mouse.tilepos);
		if (mouse.tilepos) ed_redraw_tile(mouse.tilepos-1);
		if (mouse.tilepos!=29) ed_redraw_tile(mouse.tilepos+1);
		need_full_redraw=1;
	} else if (e.button==SDL_BUTTON_RIGHT && mouse.keys==k_none) { /* right click: copy tile */
		tile_global_location_type t=T(loaded_room,mouse.tilepos);
		copied.main.concept.fg=edited.fg[t];
		copied.main.concept.bg=edited.bg[t];
		switch(copied.main.concept.fg&TILE_MASK) {
		case tiles_16_level_door_left:
		case tiles_23_balcony_left:
			t=room_api_tile_move(&edited_map,t,1,0);
			copied.extratype=extra_right;
			break;
		case tiles_17_level_door_right:
		case tiles_24_balcony_right:
			t=room_api_tile_move(&edited_map,t,-1,0);
			copied.extratype=extra_left;
			break;
		case tiles_9_bigpillar_top:
			t=room_api_tile_move(&edited_map,t,0,1);
			copied.extratype=extra_down;
			break;
		case tiles_8_bigpillar_bottom:
			t=room_api_tile_move(&edited_map,t,0,-1);
			copied.extratype=extra_up;
			break;
		default:
			copied.extratype=extra_none;
			break;
		}
		char aux[40];
		if (copied.extratype!=extra_none && t!=-1) {
			copied.extra.concept.fg=edited.fg[t];
			copied.extra.concept.bg=edited.bg[t];
			snprintf(aux,40,"COPIED FG:%d/%d+%d/%d%c BG:%d+%d",
			         copied.main.concept.fg&TILE_MASK, copied.main.concept.fg>>5,
			         copied.extra.concept.fg&TILE_MASK, copied.extra.concept.fg>>5,
			         copied.extratype,
			         copied.main.concept.bg,
			         copied.extra.concept.bg
			);
		} else {
			snprintf(aux,40,"COPIED FG:%d/%d BG:%d",copied.main.concept.fg&TILE_MASK, copied.main.concept.fg>>5, copied.main.concept.bg);
		}
		print(aux);
	} else if (e.button==SDL_BUTTON_LEFT && mouse.keys==k_shift) { /* shift+left click: move kid */
		editor__position(&Kid,mouse.col,mouse.row,loaded_room,mouse.x_b,6563);
	} else if (e.button==SDL_BUTTON_RIGHT && mouse.keys==k_shift) { /* shift+right click: move move/put guard */
		editor__set_guard(mouse.tilepos,mouse.x_b);
		editor__position(&Guard,mouse.col,mouse.row,loaded_room,mouse.x_b,6569);
		need_full_redraw=1;
	} else if (e.button==SDL_BUTTON_LEFT && mouse.keys==(k_shift|k_ctrl)) { /* ctrl+shift+left click: randomize tile */
		randomize_tile(mouse.tilepos);
		ed_redraw_tile(mouse.tilepos);
		if (mouse.tilepos) ed_redraw_tile(mouse.tilepos-1);
		if (mouse.tilepos!=29) ed_redraw_tile(mouse.tilepos+1);
		need_full_redraw=1;
	} else if (e.button==SDL_BUTTON_LEFT && mouse.keys==(k_ctrl|k_alt)) { /* ctrl+alt+left click: toggle door mechanism links */
		if (door_api_is_related(edited.fg[T(loaded_room,mouse.tilepos)]&TILE_MASK)) {
			editor__do_mark_start(flag_redoor);
			print(editor__toggle_door_tile(loaded_room,mouse.tilepos));
			editor__do_mark_end(flag_redoor);
		}
	} else if (e.button==SDL_BUTTON_RIGHT && mouse.keys==(k_ctrl|k_alt)) { /* ctrl+alt+right click: pick door mechanism tile */
		if (door_api_is_related(edited.fg[T(loaded_room,mouse.tilepos)]&TILE_MASK)) {
			editor__save_door_tile(T(loaded_room,mouse.tilepos));
		}
	} else if (e.button==SDL_BUTTON_LEFT && mouse.keys==k_m) { /* m+left click: go to map room */
		if (map_selected_room) {
			goto_next_room=map_selected_room;
		}
	} else if (e.button==SDL_BUTTON_LEFT && mouse.keys==(k_ctrl|k_shift|k_m)) { /* ctrl+shift+m+left click: go to map room */
		if (map_selected_room) editor__randomize(map_selected_room);
	} else if (e.button==SDL_BUTTON_LEFT && mouse.keys==(k_ctrl)) { /* ctrl+left click: select tile */
		if (selected_mask_room!=drawn_room) {
			selected_mask_room=drawn_room;
			clean_selected_mask();
		}
		selected_mask[mouse.tilepos]^=1;
	}
}

extern word cheats_enabled;
void editor__process_key(int key,const char** answer_text, word* need_show_text) {
	static char aux[50];
	int aux_int;
	if (goto_next_room) {
		if (PALETTE_MODE) {
			remember_room=drawn_room;
		}
		next_room=goto_next_room;
		goto_next_room=0;
	}

	if (key==(SDL_SCANCODE_E | WITH_ALT)) { // alt-e
		editor_enabled=!editor_enabled;
		if (editor_enabled) {
			*answer_text="EDITOR ENABLED";
			cheats_enabled=1;
		} else {
			*answer_text="EDITOR DISABLED";
		}
		*need_show_text=1;
		return;
	}

	if (!editor_enabled) return;
	switch (key) {
	case SDL_SCANCODE_Z | WITH_CTRL: /* ctrl-z */
	case SDL_SCANCODE_Z | WITH_CTRL | WITH_ALT: /* ctrl-alt-z */
		{
		tUndoQueueMark mrk;
		if (key==(SDL_SCANCODE_Z | WITH_CTRL)) {
			mrk=editor__undo();
		} else {
			mrk=editor__redo();
		}
		ed_redraw_room();
		if (mrk&flag_redraw) need_full_redraw = 1; // force redraw
		if (mrk&flag_remap) room_api_refresh(&edited_map);
		//TODO: if (mrk&flag_redoor) door_api_refresh(&edited_doorlinks);
		if (mrk&flag_guard_presence)
			editor__synchronize_guard_presence();
		if (mrk&flag_guard_repaint)
			editor__synchronize_guard_repaint();
		}
		break;
	case SDL_SCANCODE_A:
		{
		const char* am[]={"AMBIGUOUS MODE OFF","AMBIGUOUS MODE ON","AMBIGUOUS MODE FULL"};
		ambiguous_mode=(ambiguous_mode+1)%3;
		*answer_text=am[ambiguous_mode];
		*need_show_text=1;
		}
		break;
	case SDL_SCANCODE_DELETE: /* delete */
	case SDL_SCANCODE_BACKSPACE: /* backspace */
		editor__remove_guard();
		break;
	case SDL_SCANCODE_DELETE | WITH_CTRL: /* ctrl-delete */
	case SDL_SCANCODE_BACKSPACE | WITH_CTRL: /* ctrl-backspace */
		editor__clean_room(drawn_room);
		clean_selected_mask();
		break;
	case SDL_SCANCODE_DELETE | WITH_SHIFT: /* shift-delete */
	case SDL_SCANCODE_BACKSPACE | WITH_SHIFT: /* shift-backspace */
		editor__do_mark_start(flag_remap);
		if (drawn_room!=edited.start_room) {
			/* select next room to show */
			aux_int=edited.roomlinks[drawn_room-1].left;
			if (!aux_int) aux_int=edited.roomlinks[drawn_room-1].right;
			if (!aux_int) aux_int=edited.roomlinks[drawn_room-1].up;
			if (!aux_int) aux_int=edited.roomlinks[drawn_room-1].down;
			room_api_free_room(&edited_map,drawn_room);
			/* when a room is removed the level may be split in two disconnected level parts, to avoid staying
			   in the wrong part of the level, I'll send the next_room to the starting position */
			if (!edited_map.list[aux_int-1]) aux_int=edited.start_room;
			//TODO: free door links
			next_room=aux_int;
			*answer_text="Room deleted";
		} else {
			*answer_text="MOVE STARTING ROOM FIRST";
		}
		*need_show_text=1;
		editor__do_mark_end(flag_remap);
		break;
	case SDL_SCANCODE_LEFTBRACKET:
	case SDL_SCANCODE_RIGHTBRACKET:
		aux_int=editor__guard_skill(key==SDL_SCANCODE_LEFTBRACKET?-1:1);
		if (aux_int!=-1) {
			snprintf(aux,50,"Guard skill is %d",aux_int);
			*answer_text=aux;
			*need_show_text=1;
			/* TODO: synchronize skills with the game */
		}
		break;
	case SDL_SCANCODE_Q:
	case SDL_SCANCODE_W:
		aux_int=editor__guard_color(key==SDL_SCANCODE_Q?-1:1);
		if (aux_int!=-1) {
			snprintf(aux,50,"Guard color code is %d",aux_int);
			*answer_text=aux;
			*need_show_text=1;
		}
		break;
	case SDL_SCANCODE_TAB:
		editor__guard_toggle();
		Guard.direction=~Guard.direction; /* synch */
		break;
	case SDL_SCANCODE_J | WITH_SHIFT:
	case SDL_SCANCODE_H | WITH_SHIFT:
	case SDL_SCANCODE_U | WITH_SHIFT:
	case SDL_SCANCODE_N | WITH_SHIFT:
		if (PALETTE_MODE) break;
		editor__do_mark_start(flag_redraw|flag_remap);
		if (key==(SDL_SCANCODE_J | WITH_SHIFT))
			aux_int=room_api_insert_room_right(&edited_map,room_api_where_room(&edited_map,drawn_room));
		if (key==(SDL_SCANCODE_H | WITH_SHIFT))
			aux_int=room_api_insert_room_left(&edited_map,room_api_where_room(&edited_map,drawn_room));
		if (key==(SDL_SCANCODE_U | WITH_SHIFT))
			aux_int=room_api_insert_room_up(&edited_map,room_api_where_room(&edited_map,drawn_room));
		if (key==(SDL_SCANCODE_N | WITH_SHIFT))
			aux_int=room_api_insert_room_down(&edited_map,room_api_where_room(&edited_map,drawn_room));
		if (aux_int) {
			randomize_room(aux_int);
			ed_select_room(aux_int);
			ed_redraw_room();
			room_api_refresh(&edited_map);
			next_room=aux_int;
			snprintf(aux,50,"Added S%d",aux_int);
			*answer_text=aux;
		} else {
			*answer_text="NO MORE SCREENS AVAILABLE";
		}
		editor__do_mark_end(flag_redraw|flag_remap);
		*need_show_text=1;
		break;
#ifdef __DEBUG__
	case SDL_SCANCODE_D: /* d for debugging purposes */
		{
			*answer_text="DEBUG ACTION";
			*need_show_text=1;
		}
		break;
#endif
	case SDL_SCANCODE_P: /* p: Toggle palette */
		{
		if (!remember_room) {
			if (PALETTE_MODE) { /* go to a level room */
				remember_room=edited.start_room;
			} else {
				remember_room=NUMBER_OF_ROOMS+1;
			}
		}
		next_room=remember_room;
		if ((remember_room>NUMBER_OF_ROOMS)!=PALETTE_MODE)
			remember_room=drawn_room;
		}
		break;
	case SDL_SCANCODE_P | WITH_ALT: /* alt-p: Save palette */
		save_edit_palettes();
		*answer_text="PALETTE SAVED";
		*need_show_text=1;
		break;
	case SDL_SCANCODE_Y: /* y: save starting position */
		editor__do(start_pos,Kid.curr_row*10+Kid.curr_col,mark_start);
		editor__do(start_room,Kid.room,mark_middle);
		editor__do(start_dir,Kid.direction,mark_end);
		*answer_text="New starting position set";
		*need_show_text=1;
		break;
	case SDL_SCANCODE_R | WITH_CTRL | WITH_SHIFT: /* ctrl-shift-r */
		editor__randomize(loaded_room);
		break;
	case SDL_SCANCODE_S | WITH_CTRL | WITH_SHIFT: /* ctrl-shift-s */
		editor__do_mark_start(flag_redraw);
		sanitize_room(loaded_room,0);
		editor__do_mark_end(flag_redraw);
		ed_redraw_room();
		need_full_redraw=1;
		break;
	case SDL_SCANCODE_C | WITH_CTRL: /* ctrl-c: copy room */
		*answer_text=editor__copy_room(drawn_room)?"ROOM COPIED":"TILES COPIED";
		clean_selected_mask();
		*need_show_text=1;
		break;
	case SDL_SCANCODE_X | WITH_CTRL: /* ctrl-x: copy room */
		*answer_text=editor__copy_room(drawn_room)?"ROOM CUT":"TILES CUT";
		editor__clean_room(drawn_room);
		clean_selected_mask();
		*need_show_text=1;
		break;
	case SDL_SCANCODE_V | WITH_CTRL: /* ctrl-v: paste room */
		editor__paste_room(drawn_room);
		ed_redraw_room();
		need_full_redraw=1;
		break;
	case SDL_SCANCODE_S | WITH_ALT: /* alt-s */
		if (edition_level==current_level) {
			*answer_text="LEVEL SAVED";
			*need_show_text=1;
			save_level();
		} else {
			*answer_text="Nothing to save";
			*need_show_text=1;
		}
		break;
	case SDL_SCANCODE_R | WITH_ALT: /* alt-r */
		if (edition_level==current_level) {
			*answer_text="LEVEL REVERTED";
			*need_show_text=1;
			editor_revert_level();
			is_restart_level = 1;
		} else {
			*answer_text="Nothing to revert";
			*need_show_text=1;
		}
		break;
	}
}

/********************************************\
*               Room linking API             *
\********************************************/

void room_api__private_recurse(tMap* map, long r, int aux_room) {
	if ( (!aux_room) || map->list[aux_room-1]) return; /* if the room exists and is the first time we visit it */
	map->map[r]=aux_room;
	map->list[aux_room-1]=r;

	if (r%MAP_SIDE<map->crop.left) map->crop.left=r%MAP_SIDE;
	if (r%MAP_SIDE>map->crop.right) map->crop.right=r%MAP_SIDE;
	if (r/MAP_SIDE<map->crop.top) map->crop.top=r/MAP_SIDE;
	if (r/MAP_SIDE>map->crop.bottom) map->crop.bottom=r/MAP_SIDE;

#define GO(field,offset) room_api__private_recurse(map,r+offset,edited.roomlinks[aux_room-1].field)
	GO(up,POS_UP);
	GO(right,POS_RIGHT);
	GO(down,POS_DOWN);
	GO(left,POS_LEFT);
#undef GO
}

void room_api_refresh(tMap* map) {
	memset(map->map,0,sizeof(byte)*MAP_SIDE*MAP_SIDE);
	memset(map->list,0,sizeof(long)*NUMBER_OF_ROOMS);

	rect_type aux={MAP_CENTER+10,MAP_CENTER+10,MAP_CENTER-10,MAP_CENTER-10};
	map->crop=aux;
	room_api__private_recurse(map,MAP_POS(MAP_CENTER,MAP_CENTER),edited.start_room);

#ifdef __SCREEN_DEBUG__
	{
		int i,j;
		printf("----------------------------------");
		for (i=map->crop.top;i<=map->crop.bottom;i++) {
			printf("\n");
			for (j=map->crop.left;j<=map->crop.right;j++) {
				if (map->map[i*MAP_SIDE+j]) {
					printf("%c",'a'+map->map[i*MAP_SIDE+j]-1);
				} else {
					printf(" ");
				}
			}
		}
		printf("\n");
	}
#endif
}

void room_api_get_size(const tMap* map,int* w, int* h) {
	*w=(map->crop.right-map->crop.left+1)*10;
	*h=(map->crop.bottom-map->crop.top+1)*3;
}

int room_api_get_room_location(const tMap* map, int room, int* i, int* j) {
	int where=map->list[room-1];
	if (!where) return 0;

	*i= (where%MAP_SIDE) - map->crop.left;
	*j= (where/MAP_SIDE) - map->crop.top;
	return 1;
}

tile_global_location_type room_api_translate(const tMap* map,int x, int y) {
	int col=x%10;
	int row=y%3;
	int tilepos=row*10+col;
	int r=MAP_POS(map->crop.left+x/10,map->crop.top+y/3);

	if (map->map[r]) return T(map->map[r],tilepos);
	return -1;
}

void room_api_init(tMap* map) {
	if (map->list) return;
	map->map=malloc(sizeof(byte)*MAP_SIDE*MAP_SIDE);
	map->list=malloc(sizeof(long)*NUMBER_OF_ROOMS);

	room_api_refresh(map);
}

void room_api_free(tMap* map) {
	if (!map->list) return;
	free(map->list);
	free(map->map);
	map->list=NULL;
	map->map=NULL;
}

int room_api_get_free_room(const tMap* map) {
	int i;
	for (i=0;i<NUMBER_OF_ROOMS;i++)
		if (!map->list[i])
			return i+1;
	return 0;
}

void room_api_free_room(tMap* map,int r) { /* DO NOT FREE THE STARTING ROOM! */
#define REMOVE_LINK(dir,opo) if (edited.roomlinks[r-1].dir) editor__do(roomlinks[edited.roomlinks[r-1].dir-1].opo,0,mark_middle);
	REMOVE_LINK(up,down)
	REMOVE_LINK(left,right)
	REMOVE_LINK(down,up)
	REMOVE_LINK(right,left)
#undef REMOVE_LINK
	room_api_refresh(map);

	/*
	simple algorithm to change the room to 0

	for (i=0;i<MAP_SIDE*MAP_SIDE;i++) {
		if (map->map[i]==r) {
			map->map[i]=0;
			return;
		}
	*/
}

void room_api_put_room(tMap* map, long where, int r) {
	map->list[r-1]=where;
	map->map[where]=r;

#define ADD_LINK(dir,opodir,offset) \
	editor__do(roomlinks[r-1].dir,map->map[where+offset],mark_middle);\
	if (map->map[where+offset])\
		editor__do(roomlinks[map->map[where+offset]-1].opodir,r,mark_middle)
	ADD_LINK(down,up,POS_DOWN);
	ADD_LINK(up,down,POS_UP);
	ADD_LINK(left,right,POS_LEFT);
	ADD_LINK(right,left,POS_RIGHT);
#undef ADD_LINK

}

long room_api_where_room(const tMap* map, byte room) {
	return map->list[room-1];
}

int room_api_alloc_room(tMap* map, int where) {
	int r=room_api_get_free_room(map);
	if (!r) return 0;
	room_api_put_room(map,where,r);
	return r;
}

int room_api_insert_room_right(tMap* map, int where) {
	int i,j,_i,r;
	if (!map->map[where+POS_RIGHT]) {
		return room_api_alloc_room(map,where+POS_RIGHT);
	}
	/* there is a room on the right, I must move the whole right part of the level one position to the right */
	r=room_api_get_free_room(map);
	if (!r) return 0;
	_i=(where+POS_RIGHT)%MAP_SIDE;
	for (j=MAP_SIDE-2;j;j--) {
		for (i=MAP_SIDE-2;_i<=i;i--)
			map->map[MAP_POS(i+1,j)]=map->map[MAP_POS(i,j)];
		map->map[MAP_POS(_i,j)]=0;
	}
	for (j=MAP_SIDE-2;j;j--) {
		if (map->map[MAP_POS(_i-1,j)])
			editor__do(roomlinks[map->map[MAP_POS(_i-1,j)]-1].right,0,mark_middle);
		if (map->map[MAP_POS(_i+1,j)])
			editor__do(roomlinks[map->map[MAP_POS(_i+1,j)]-1].left,0,mark_middle);
	}
	room_api_put_room(map,where+POS_RIGHT,r);

#ifdef __SCREEN_DEBUG__
	{
		printf("----------------------------------");
		for (i=0;i<MAP_SIDE*MAP_SIDE;i++) {
			if (!(i%MAP_SIDE)) printf("\n");
			if (map->map[i]) {
				printf("%c",'a'+map->map[i]-1);
			} else {
				printf(" ");
			}
		}
		printf("\n");
	}
#endif
	return r;
}

int room_api_insert_room_down(tMap* map, int where) {
	int i,_j,j,r;
	if (!map->map[where+POS_DOWN]) {
		return room_api_alloc_room(map,where+POS_DOWN);
	}
	/* there is a room down, I must move the whole down part of the level one position down */
	r=room_api_get_free_room(map);
	if (!r) return 0;
	_j=(where+POS_DOWN)/MAP_SIDE;
	for (i=MAP_SIDE-2;i;i--) {
		for (j=MAP_SIDE-2;_j<=j;j--)
			map->map[MAP_POS(i,j+1)]=map->map[MAP_POS(i,j)];
		map->map[MAP_POS(i,_j)]=0;
	}
	for (i=MAP_SIDE-2;i;i--) {
		if (map->map[MAP_POS(i,_j-1)])
			editor__do(roomlinks[map->map[MAP_POS(i,_j-1)]-1].down,0,mark_middle);
		if (map->map[MAP_POS(i,_j+1)])
			editor__do(roomlinks[map->map[MAP_POS(i,_j+1)]-1].up,0,mark_middle);
	}
	room_api_put_room(map,where+POS_DOWN,r);

#ifdef __SCREEN_DEBUG__
	{
		printf("----------------------------------");
		for (i=0;i<MAP_SIDE*MAP_SIDE;i++) {
			if (!(i%MAP_SIDE)) printf("\n");
			if (map->map[i]) {
				printf("%c",'a'+map->map[i]-1);
			} else {
				printf(" ");
			}
		}
		printf("\n");
	}
#endif
	return r;
}

int room_api_insert_room_left(tMap* map, int where) {
	if (!map->map[where+POS_LEFT]) {
		return room_api_alloc_room(map,where+POS_LEFT);
	}
	return room_api_insert_room_right(map,where+POS_LEFT);
}

int room_api_insert_room_up(tMap* map, int where) {
	if (!map->map[where+POS_UP]) {
		return room_api_alloc_room(map,where+POS_UP);
	}
	return room_api_insert_room_down(map,where+POS_UP);
}

/* Randomization Room Linking API sublayer */
/* Randomize tiles, rooms and complete levels using statistical inference. */

tile_global_location_type room_api_tile_move(const tMap* map, tile_global_location_type t, char col, char row) {
	int absolute_col,absolute_row,where,room;

	where=map->list[R_(t)];
	absolute_col= 10 * (where%MAP_SIDE) + P(t)%10 + col;
	absolute_row= 3  * (where/MAP_SIDE) + P(t)/10 + row;

	room=map->map[MAP_POS(absolute_col/10,absolute_row/3)];
	if (room==0) return -1;
	return T(room,absolute_col%10 + 10 * (absolute_row%3));
}

int room_api__private_entropy(const tMap* map, tile_global_location_type t, byte* level_mask,char col, char row) {
	tile_global_location_type aux;
	aux=room_api_tile_move(map,t,col,row);
	return aux!=-1 && level_mask[aux];
}

tile_packed_type room_api__private_get_tile_if_exists(const tMap* map, tile_global_location_type t, byte* level_mask,char col, char row) {
	tile_global_location_type aux;
	tile_packed_type result;
	aux=room_api_tile_move(map,t,col,row);
	if (aux!=-1 && level_mask[aux]) {
		result=TP(edited,aux);
	} else {
		result=NO_TILE;
	}
	return result;
}

float room_api_measure_entropy(const tMap* map, tile_global_location_type t, byte* level_mask) {
	float result=0;

#define ENTROPY_MEASURE_TILE(x,y,e) \
	if (room_api__private_entropy(map,t,level_mask,x,y)) result+=e

	/* kernel */
	ENTROPY_MEASURE_TILE(1,0,E_0_1);
	ENTROPY_MEASURE_TILE(0,1,E_0_1);
	ENTROPY_MEASURE_TILE(-1,0,E_0_1);
	ENTROPY_MEASURE_TILE(0,-1,E_0_1);
	ENTROPY_MEASURE_TILE(1,1,E_1_1);
	ENTROPY_MEASURE_TILE(-1,1,E_1_1);
	ENTROPY_MEASURE_TILE(1,-1,E_1_1);
	ENTROPY_MEASURE_TILE(-1,-1,E_1_1);
	ENTROPY_MEASURE_TILE(2,0,E_0_2);
	ENTROPY_MEASURE_TILE(0,2,E_0_2);
	ENTROPY_MEASURE_TILE(-2,0,E_0_2);
	ENTROPY_MEASURE_TILE(0,-2,E_0_2);
	ENTROPY_MEASURE_TILE(1,2,E_1_2);
	ENTROPY_MEASURE_TILE(1,-2,E_1_2);
	ENTROPY_MEASURE_TILE(-1,2,E_1_2);
	ENTROPY_MEASURE_TILE(-1,-2,E_1_2);
	ENTROPY_MEASURE_TILE(2,1,E_1_2);
	ENTROPY_MEASURE_TILE(2,-1,E_1_2);
	ENTROPY_MEASURE_TILE(-2,1,E_1_2);
	ENTROPY_MEASURE_TILE(-2,-1,E_1_2);

	return result;
}

void room_api__private_measure_similarity(const tMap* map,tile_global_location_type origin, tile_global_location_type target, byte* level_mask, int x, int y,float* result, float* total, float weight) {
	tile_packed_type target_tile, origin_tile;
	target_tile=room_api__private_get_tile_if_exists(map,target,level_mask,x,y);
	if (target_tile.number==NO_TILE.number) return;
	origin_tile=room_api__private_get_tile_if_exists(map,origin,level_mask,x,y);
	if (origin_tile.number==NO_TILE.number) return;
	*total+=weight;
	if (origin_tile.number==target_tile.number) *result+=weight;
}

void room_api_measure_similarity(const tMap* map, tile_global_location_type origin, tile_global_location_type target, byte* level_mask, tProbability* probabilities) {
	float result=0,total=0; /* ORIGIN is iterating CURRENT */
	tile_packed_type current_tile;

	current_tile=room_api__private_get_tile_if_exists(map,origin,level_mask,0,0);
	if (current_tile.number==NO_TILE.number) return; /* there is no tile here */

#define ENTROPY_MEASURE_SIMILARITY(x,y,e) \
	room_api__private_measure_similarity(map,origin,target,level_mask,x,y,&result,&total,e)

	/* kernel */
	ENTROPY_MEASURE_SIMILARITY(1,0,E_0_1);
	ENTROPY_MEASURE_SIMILARITY(0,1,E_0_1);
	ENTROPY_MEASURE_SIMILARITY(-1,0,E_0_1);
	ENTROPY_MEASURE_SIMILARITY(0,-1,E_0_1);
	ENTROPY_MEASURE_SIMILARITY(1,1,E_1_1);
	ENTROPY_MEASURE_SIMILARITY(-1,1,E_1_1);
	ENTROPY_MEASURE_SIMILARITY(1,-1,E_1_1);
	ENTROPY_MEASURE_SIMILARITY(-1,-1,E_1_1);
	ENTROPY_MEASURE_SIMILARITY(2,0,E_0_2);
	ENTROPY_MEASURE_SIMILARITY(0,2,E_0_2);
	ENTROPY_MEASURE_SIMILARITY(-2,0,E_0_2);
	ENTROPY_MEASURE_SIMILARITY(0,-2,E_0_2);
	ENTROPY_MEASURE_SIMILARITY(1,2,E_1_2);
	ENTROPY_MEASURE_SIMILARITY(1,-2,E_1_2);
	ENTROPY_MEASURE_SIMILARITY(-1,2,E_1_2);
	ENTROPY_MEASURE_SIMILARITY(-1,-2,E_1_2);
	ENTROPY_MEASURE_SIMILARITY(2,1,E_1_2);
	ENTROPY_MEASURE_SIMILARITY(2,-1,E_1_2);
	ENTROPY_MEASURE_SIMILARITY(-2,1,E_1_2);
	ENTROPY_MEASURE_SIMILARITY(-2,-1,E_1_2);

	if (total) {
		probabilities[current_tile.number].count++;
		probabilities[current_tile.number].value+=result/total;
	}
}

#define PROBABILITY_SIZE 65536
tile_packed_type room_api_suggest_tile(const tMap* map, tile_global_location_type tilepos, byte* level_mask) {
	tProbability* probability;
	tProbability aux={0,0};
	tile_global_location_type i;
	Uint16 j;
	float total=0,random_prob;

	probability=malloc(PROBABILITY_SIZE*sizeof(tProbability));
	for (j=0;j!=NO_TILE.number;j++)
		probability[j]=aux;

	for (i=0;i<NUMBER_OF_ROOMS*30;i++) {
		if (map->list[R_(i)])
			room_api_measure_similarity(map,i,tilepos,level_mask,probability);
	}

#ifdef __SCREEN_DEBUG__
	for (j=0;j!=NO_TILE.number;j++) {
		if (probability[j].count)
			printf("k=%d v=%f c=%d\n",j,probability[j].value,probability[j].count);
	}
#endif

	for (j=0;j!=NO_TILE.number;j++)
		total+=probability[j].value;
	random_prob=(total*rand())/RAND_MAX; /* what if random==0 ???? */

	if (random_prob==0) {
		free(probability);
		return NO_TILE;
	}
	total=0;
	for (j=0;j!=NO_TILE.number;j++) {
		random_prob-=probability[j].value;
		if (random_prob<=0) {
			free(probability);
			return (tile_packed_type)j;
		}
	}
	free(probability);
	return NO_TILE;
}

#endif
