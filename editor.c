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


/*
TODO:
	room decorate.
	free room.
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

/********************************************\
*      Randomization sublayer headers        *
\********************************************/

/* E_x_y = e^(-(x^2+y^2)) -> gaussian blur */
#define E_0_1 .36787944117144232159
#define E_0_2 .01831563888873418029
#define E_1_1 .13533528323661269189
#define E_1_2 .00673794699908546709

#pragma pack(push, 1)
typedef union {
 struct {
		byte fg;
		byte bg;
	} concept; /* tile_and_mod but packed */
	word number;
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

void stack_push(long data) {
	if (!stack_size) {
		stack_size=100;
		stack_pointer=malloc(sizeof(long)*stack_size);
	}
	stack_pointer[stack_cursor]=data;
	stack_cursor++;
	/*if (stack_top<stack_cursor)*/ stack_top=stack_cursor;
	if (stack_size<=stack_top) {
		stack_size*=2;
		stack_pointer=realloc(stack_pointer,sizeof(long)*stack_size);
	}
}
int stack_pop(long* data) {
	if (!stack_cursor) return false;
	*data=stack_pointer[--stack_cursor];
	return true;
}
int stack_unpop(long* data) {
	if (stack_cursor==stack_top) return false;
	*data=stack_pointer[stack_cursor++];
	return true;
}
void stack_or(long data) {
	if (stack_cursor) {
		stack_pointer[stack_cursor-1]|=data;
	}
}
#undef true
#undef false

/* DUR: Do actions sublayer */

typedef enum {
	mark_middle=0,
	mark_start=1,
	mark_end=2,
	mark_all=3
}tUndoQueueMark;

tUndoQueueMark prevMark=mark_middle;
#define editor__do_mark_start() prevMark=mark_start
void editor__do_mark_end() {
	if (prevMark!=mark_start) /* if writing is open */
		stack_or(mark_end<<16);
	prevMark=mark_middle;
}

/* editor level layer used by do/undo/redo */
void editor__load_level();
tile_and_mod copied={0,0};
byte copied_room_fg[30]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
byte copied_room_bg[30]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
level_type edited;
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
	stack_push(offset<<18|mark<<16|before<<8|c);
}

void editor__undo() {
	long aux,offset;
	byte before;
	tUndoQueueMark mark;
	while (stack_pop(&aux)) {
		/* after=   aux     & 0xff; */
		before= (aux>>8) & 0xff;
		mark=   (aux>>16)& mark_all;
		offset= (aux>>18);
		offset[(char*)(&level)]=before;
		offset[(char*)(&edited)]=before;
		if (mark & mark_start) break;
	}
}
void editor__redo() {
	long aux,offset;
	byte after;
	tUndoQueueMark mark;
	while (stack_unpop(&aux)) {
		after=   aux     & 0xff;
		/* before= (aux>>8) & 0xff; */
		mark=   (aux>>16)& mark_all;
		offset= (aux>>18);
		offset[(char*)(&level)]=after;
		offset[(char*)(&edited)]=after;
		if (mark & mark_end) break;
	}
}

/********************************************\
*            External functions              *
\********************************************/

/* TODO: move to header file */

void __pascal far redraw_screen(int drawing_different_room);

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
				(edited.fg[tile-1]&TILE_MASK)==tiles_16_level_door_left) { /* TODO: use a define for this mask */
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
*             Editor functions               *
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
	if (chtab_editor_sprites) free_chtab(chtab_editor_sprites);
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

void editor__paste_room(int room) {
	int i;

	editor__do_mark_start();
	for (i=0;i<30;i++) {
		editor__do(fg[T(drawn_room,i)],copied_room_fg[i],mark_middle);
		editor__do(bg[T(drawn_room,i)],copied_room_bg[i],mark_middle);
	}
	editor__do_mark_end();
}

/*
Debug char position
printf("Guard | %d-%d %d,%d |\n",
	Guard.curr_col,Guard.curr_row,Guard.x,Guard.y
);
*/

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
	printf("tile %d\n",level.guards_tile[loaded_room-1]);
	printf("c %d\n",level.guards_color[loaded_room-1]);
	printf("x %d\n",level.guards_x[loaded_room-1]);
	printf("dir %d\n",level.guards_dir[loaded_room-1]);
	printf("skill %d\n",level.guards_skill[loaded_room-1]);
	if (level.guards_tile[loaded_room-1]>=30) {
		editor__do(guards_tile[loaded_room-1],tilepos,mark_start);
		editor__do(guards_color[loaded_room-1],1,mark_middle);
		editor__do(guards_x[loaded_room-1],x,mark_middle);
		editor__do(guards_dir[loaded_room-1],0,mark_middle);
		editor__do(guards_seq_hi[loaded_room-1],0,mark_middle);
		editor__do(guards_skill[loaded_room-1],0,mark_end);
	} else {
		editor__do(guards_tile[loaded_room-1],tilepos,mark_start);
		editor__do(guards_x[loaded_room-1],x,mark_end);
	}
	enter_guard(); // load color, HP
}

void editor__remove_guard() {
	draw_guard_hp(0, 10); // delete HP
	Guard.direction = dir_56_none; // delete guard from screen
	editor__do(guards_tile[loaded_room-1],30,mark_all);
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
	if (0<=new_color && new_color<=7 && level.guards_tile[loaded_room-1]<30) {
		editor__do(guards_color[loaded_room-1],new_color,mark_all);
		// If I call redraw_screen() or enter_guard() directly then the kid changes into a guard...
		curr_guard_color = new_color;
		need_full_redraw = 1; // force redraw
		return new_color;
	}
	return -1;
}

void editor__guard_toggle() {
	if (level.guards_tile[loaded_room-1]<30) {
		editor__do(guards_dir[loaded_room-1],~level.guards_dir[loaded_room-1],mark_all);
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
			editor__do(fg[tile],tt.concept.fg,mark_middle);
			editor__do(bg[tile],tt.concept.bg,mark_middle);
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
	editor__do(fg[t],tt.concept.fg,mark_start);
	editor__do(bg[t],tt.concept.bg,mark_end);
}

void sanitize_room(int room, int sanitation_level) {
	#define tile_at(tilepos,room) (edited.fg[T(room,tilepos)]&TILE_MASK)
	#define room_link edited.roomlinks[room-1]
	#define up_is(x,def) ((x>=10)?tile_at(x-10,room):(room_link.up?tile_at(x+20,room_link.up):def))
	#define down_is(x,def) ((x<20)?tile_at(x+10,room):(room_link.down?tile_at(x-20,room_link.down):def))
	#define left_is(x,def) ((x%10)?tile_at(x-1,room):(room_link.left?tile_at(x+9,room_link.left):def))
	#define right_is(x,def) ((x%10!=9)?tile_at(x+1,room):(room_link.right?tile_at(x-9,room_link.right):def))

	int i;
	byte tile;
	for (i=0;i<30;i++) {
		tile=tile_at(i,room);
		/* printf("sanitize %d %d %d\n",i,tile,(edited.bg[(room-1)*30+(i)])); */
		if (sanitation_level==1) if(tile!=tiles_11_loose && tile!=tiles_20_wall && tile!=31) { /* check for alone tile */
			if (left_is(i,tiles_20_wall)==tiles_20_wall && right_is(i,tiles_20_wall)==tiles_20_wall) {
				int tileup=up_is(i,tiles_20_wall);
				if (tileup!=tiles_11_loose && tileup!=31) {
					editor__do(fg[T(room,i)],tiles_20_wall,mark_middle);
					editor__do(bg[T(room,i)],0,mark_middle);
				}
			}
		}
		switch(tile) {
		case tiles_11_loose:
		case tiles_0_empty:
			if (sanitation_level==0) {
				if (down_is(i,-1)==tiles_20_wall || down_is(i,-1)==tiles_3_pillar) {
					editor__do(fg[T(room,i)],tiles_1_floor,mark_middle);
					editor__do(bg[T(room,i)],0,mark_middle);
				}
			} else if (sanitation_level==1) {
				if (right_is(i,-1)==tiles_20_wall && left_is(i,-1)==tiles_20_wall && up_is(i,tiles_20_wall)==tiles_20_wall) {
					editor__do(fg[T(room,i)],tiles_20_wall,mark_middle);
					editor__do(bg[T(room,i)],0,mark_middle);
				}
			}
			break;
		case tiles_19_torch:
			if (right_is(i,-1)==tiles_20_wall || right_is(i,-1)==tiles_3_pillar) {
				editor__do(fg[T(room,i)],tiles_1_floor,mark_middle);
				editor__do(bg[T(room,i)],0,mark_middle);
			}
			break;
		case tiles_30_torch_with_debris:
			if (right_is(i,-1)==tiles_20_wall || right_is(i,-1)==tiles_3_pillar) {
				editor__do(fg[T(room,i)],tiles_14_debris,mark_middle);
				editor__do(bg[T(room,i)],0,mark_middle);
			}
			break;
		case tiles_4_gate:
			if (right_is(i,-1)==tiles_20_wall || left_is(i,-1)==tiles_20_wall) {
				editor__do(fg[T(room,i)],tiles_20_wall,mark_middle);
				editor__do(bg[T(room,i)],0,mark_middle);
			}
			break;
		case tiles_2_spike:
			if (sanitation_level==1 && down_is(i,tiles_20_wall)!=tiles_20_wall) {
				editor__do(fg[T(room,i)],tiles_1_floor,mark_middle);
				editor__do(bg[T(room,i)],0,mark_middle);
			}
			break;
		}

		if (left_is(i,-1)==tiles_16_level_door_left) {
				editor__do(fg[T(room,i)],tiles_17_level_door_right,mark_middle);
				editor__do(bg[T(room,i)],0,mark_middle);
		}
		if (right_is(i,-1)==tiles_17_level_door_right) {
				editor__do(fg[T(room,i)],tiles_16_level_door_left,mark_middle);
				editor__do(bg[T(room,i)],0,mark_middle);
		}
	}

}
void editor__randomize(int room) {
	editor__do_mark_start();
	randomize_room(room);
	editor__do_mark_end();
	ed_select_room(room);
	ed_redraw_room();
	redraw_screen(1);
}


/********************************************\
*           Blitting editor layer            *
\********************************************/

typedef enum {
	cMain=1,    /* White+grey: cursors cross+tile. */
	cRight=3,   /* Red: cursor tile when ctrl+alt is pressed and the tile is a door/button. */
	cWrong=5,   /* Blue: cursor tile when ctrl+alt is pressed and the tile is NOT a door/button. No actions possible. */
	cLinked=7,  /* Green: When a door/button is selected, the linked tiles will have this color */
	cSelected=9,/* Yellow: The selected door/button. */
	cExtra=11   /* Cyan: Not used yet. */
} tCursorColors; /* The palettes are taken from the res201 bitmap, ignoring the res200 palette. */

typedef enum {
	cCross=0,
	cSingleTile=1,
	cExitdoor=4,
	cBigPillar=7,
	cSmallTiles=10,
	cSmallCharacters=11
} tEditorImageOffset;

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

	if (SDL_SetPaletteColors(image->format->palette, chtab_editor_sprites->images[0]->format->palette->colors+colors, 1, colors_total) != 0) {
		printf("Couldn't set video mode: %s\n", SDL_GetError());
	}

	if (SDL_BlitSurface(image, &src_rect, screen, &dest_rect) != 0) {
		sdlperror("SDL_BlitSurface on editor");
		quit(1);
	}
}

/********************************************\
*             INPUT BINDINGS!!!!             *
\********************************************/

#define MouseState !GetUnscaledMouseState(&x,&y) && (x>=0) && (y>=3) && (x<320) && (y<192)
void editor__on_refresh(surface_type* screen) {
	if (chtab_editor_sprites) {
		int x,y, colors_total=1;
		tEditorImageOffset image_offset=cSingleTile;
		tCursorColors colors;
		const Uint8 *state = SDL_GetKeyboardState(NULL);

		int is_m_pressed=state[SDL_SCANCODE_M];
		int is_ctrl_alt_pressed=(state[SDL_SCANCODE_LALT] || state[SDL_SCANCODE_RALT]) && (state[SDL_SCANCODE_LCTRL] || state[SDL_SCANCODE_RCTRL]);
		int is_only_shift_pressed=(state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT]) && (!(state[SDL_SCANCODE_LCTRL] || state[SDL_SCANCODE_RCTRL]));
		int is_ctrl_shift_pressed=(state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT]) && (state[SDL_SCANCODE_LCTRL] || state[SDL_SCANCODE_RCTRL]);

		if (PALETTE_MODE) {
			is_ctrl_alt_pressed=0;
			is_only_shift_pressed=0;
			is_ctrl_shift_pressed=0;
		}

		if (MouseState) {
			int col,row,tilepos;
			colors=cMain;
			col=(x)/32;
			row=(y-3)/63;
			tilepos=row*10+col;
			/* if Shift is pressed a cross is shown */
			if (is_only_shift_pressed || is_m_pressed) {
				image_offset=cCross;
				x-=chtab_editor_sprites->images[cCross]->w/2;
				y-=chtab_editor_sprites->images[cCross]->h/2;
				colors=cMain;
			} else { /* If not, a 3D selection box is shown. When alt is pressed cRight or cWrong colors are used */
				if (is_ctrl_alt_pressed) colors=cWrong;
				if (is_ctrl_shift_pressed) colors=cExtra;
				x=col*32;
				y=row*63-10;
				switch(level.fg[T(drawn_room,tilepos)]&TILE_MASK) {
				case tiles_17_level_door_right:
					x-=32;
				case tiles_16_level_door_left:
					image_offset=cExitdoor;
					colors_total=2;
					if (is_ctrl_alt_pressed) colors=cRight;
					break;
				case tiles_8_bigpillar_bottom:
					y-=63;
				case tiles_9_bigpillar_top:
					image_offset=cBigPillar;
					colors_total=2;
					break;
				case tiles_6_closer:
				case tiles_15_opener:
				case tiles_4_gate:
					if (is_ctrl_alt_pressed) colors=cRight;
					break;
				}

				static unsigned short i_frame_clockwise=0; /* What's this? There are two different layers
				                                              of selected boxes, the fixed ones and the
				                                              cursor/pointer ones; when a tile is selected
				                                              with both layers as they are animated clockwise
				                                              and anticlockwise both selections are visible */
				image_offset+=(i_frame_clockwise++)%3;

				if (is_ctrl_alt_pressed && colors==cRight) { /* show number of links */
					char text_aux[20];
					int count=0;
					/* count links */
					tIterator it;
					tile_global_location_type current_tile,junk_tile;
					current_tile=T(drawn_room,tilepos);
					door_api_init_iterator(&it,current_tile);
					while(door_api_get(&it,&junk_tile))
						count++;
					sprintf(text_aux,"%d",count);

					/* draw text*/
					rect_type r={y,x,y+63,x+32*colors_total};
					screen_updates_suspended=1;
					surface_type* save_screen=current_target_surface;
					current_target_surface=screen;
					show_text_with_color(&r,0,0,text_aux,4);
					current_target_surface=save_screen;
					screen_updates_suspended=0;
				}
			}

			blit_sprites(x,y,image_offset,colors,colors_total,screen);
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

		/* draw map */
		if (is_m_pressed) {
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

			if (is_ctrl_alt_pressed) mode=2;
			if (is_only_shift_pressed) mode=1;
			if (is_ctrl_shift_pressed) mode=1;

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

						if (is_only_shift_pressed) {
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
									sdlperror("SDL_BlitSurface on editor showing map");
									quit(1);
								}
							}

							/* draw guard positions */
							int v=edited.guards_tile[R_(t)];
							if (v<30 && v==P(t)) {
								src2_rect.x=tw*1;

								if (SDL_BlitSurface(people, &src2_rect, screen, &dest_rect) != 0) {
									sdlperror("SDL_BlitSurface on editor showing map");
									quit(1);
								}
							}

						} /* /is_only_shift_pressed */
					}
					sx+=tw;
				}
				sy+=th;
			}
			SDL_Rect lineH={offsetx,sy,sw,1};
			SDL_FillRect(screen,&lineH,0xffffff);
			SDL_Rect lineV={sx,offsety,1,sh};
			SDL_FillRect(screen,&lineV,0xffffff);

			if (MouseState) {
				x=x-offsetx;
				y=y-offsety;
				if (x<sw && y<sh) {
					int i=x/(tw*10+1);
					int j=y/(th*3+1);
					tile_global_location_type t=room_api_translate(&edited_map,i*10,j*3);
					if (t!=-1) { //room is R(t)
						map_selected_room=R(t);
						SDL_Rect line1={offsetx+(tw*10+1)*i,offsety+(th*3+1)*j,tw*10+2,1};
						SDL_Rect line2={offsetx+(tw*10+1)*i,offsety+(th*3+1)*j,1,th*3+2};
						SDL_Rect line3={offsetx+(tw*10+1)*i,offsety+(th*3+1)*(j+1),tw*10+2,1};
						SDL_Rect line4={offsetx+(tw*10+1)*(i+1),offsety+(th*3+1)*j,1,th*3+2};
						SDL_FillRect(screen,&line1,0x55ff55);
						SDL_FillRect(screen,&line2,0x55ff55);
						SDL_FillRect(screen,&line3,0x55ff55);
						SDL_FillRect(screen,&line4,0x55ff55);
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

void editor__handle_mouse_button(SDL_MouseButtonEvent e,int shift, int ctrl, int alt, int m) {
	GetUnscaledMouseState(&e.x, &e.y);
	int col,row,tilepos,x;
	if (!editor_active) return;
	col=e.x/32;
	row=(e.y-3)/63;
	x=e.x*140/320+62;
	if (row<0 || row>2) return;
	tilepos=row*10+col;

	if (e.button==SDL_BUTTON_LEFT && !shift && !alt && !ctrl && !m) { /* left click: edit tile */
		editor__do(fg[T(loaded_room,tilepos)],copied.tiletype,mark_start);
		editor__do(bg[T(loaded_room,tilepos)],copied.modifier,mark_end);
		ed_redraw_tile(tilepos);
		if (tilepos) ed_redraw_tile(tilepos-1);
		if (tilepos!=29) ed_redraw_tile(tilepos+1);
		redraw_screen(1);
	} else if (e.button==SDL_BUTTON_RIGHT && !shift && !alt && !ctrl && !m) { /* right click: copy tile */
		copied.tiletype=edited.fg[T(loaded_room,tilepos)];
		copied.modifier=edited.bg[T(loaded_room,tilepos)];
	} else if (e.button==SDL_BUTTON_LEFT && shift && !alt && !ctrl && !m) { /* shift+left click: move kid */
		editor__position(&Kid,col,row,loaded_room,x,6563);
	} else if (e.button==SDL_BUTTON_RIGHT && shift && !alt && !ctrl && !m) { /* shift+right click: move move/put guard */
		editor__set_guard(tilepos,x);
		editor__position(&Guard,col,row,loaded_room,x,6569);
		redraw_screen(1);
	} else if (e.button==SDL_BUTTON_LEFT && shift && !alt && ctrl && !m) { /* ctrl+shift+left click: randomize tile */
		randomize_tile(tilepos);
		redraw_screen(1);
	} else if (e.button==SDL_BUTTON_LEFT && !shift && alt && ctrl && !m) { /* ctrl+alt+left click: toggle door mechanism links */
		if (door_api_is_related(edited.fg[T(loaded_room,tilepos)]&TILE_MASK)) {
			editor__do_mark_start();
			display_text_bottom(editor__toggle_door_tile(loaded_room,tilepos));
			editor__do_mark_end();
			text_time_total = 24;
			text_time_remaining = 24;
		}
	} else if (e.button==SDL_BUTTON_RIGHT && !shift && alt && ctrl && !m) { /* ctrl+alt+right click: pick door mechanism tile */
		if (door_api_is_related(edited.fg[T(loaded_room,tilepos)]&TILE_MASK)) {
			editor__save_door_tile(T(loaded_room,tilepos));
		}
	} else if (e.button==SDL_BUTTON_LEFT && !shift && !alt && !ctrl && m) { /* m+left click: go to map room */
		if (map_selected_room) {
			goto_next_room=map_selected_room;
		}
	} else if (e.button==SDL_BUTTON_LEFT && shift && !alt && ctrl && m) { /* ctrl+shift+m+left click: go to map room */
		if (map_selected_room) editor__randomize(map_selected_room);
	}

/* printf("hola mundo %d %d %d %d %c%c%c\n",
	e.state,
	e.button,
	tile,
	current_level,
	shift?'s':' ',
	ctrl?'c':' ',
	alt?'a':' '
);*/

}
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

	switch (key) {
	case SDL_SCANCODE_Z | WITH_CTRL: /* ctrl-z */
		editor__undo();
		ed_redraw_room();
		redraw_screen(1);
		break;
	case SDL_SCANCODE_Z | WITH_CTRL | WITH_ALT: /* ctrl-alt-z */
		editor__redo();
		ed_redraw_room();
		redraw_screen(1);
		break;
	case SDL_SCANCODE_DELETE: /* delete */
	case SDL_SCANCODE_BACKSPACE: /* backspace */
		editor__remove_guard();
		/* TODO: synch without sending the guard to a buffer overflow (room NUMBER_OF_ROOMS+1) */
		break;
	case SDL_SCANCODE_LEFTBRACKET:
	case SDL_SCANCODE_RIGHTBRACKET:
		aux_int=editor__guard_skill(key==SDL_SCANCODE_LEFTBRACKET?-1:1);
		if (aux_int!=-1) {
			snprintf(aux,50,"Guard skill is %d",aux_int);
			*answer_text=aux;
			*need_show_text=1;
			/* TODO: synch */
		}
		break;
	case SDL_SCANCODE_Q:
	case SDL_SCANCODE_W:
		aux_int=editor__guard_color(key==SDL_SCANCODE_Q?-1:1);
		if (aux_int!=-1) {
			snprintf(aux,50,"Guard color code is %d",aux_int);
			*answer_text=aux;
			*need_show_text=1;
			/* TODO: synch */
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
		editor__do_mark_start();
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
			next_room=aux_int;
			snprintf(aux,50,"Added S%d",aux_int);
			*answer_text=aux;
		} else {
			*answer_text="NO MORE SCREENS AVAILABLE";
		}
		editor__do_mark_end();
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
		editor__do_mark_start();
		sanitize_room(loaded_room,0);
		editor__do_mark_end();
		redraw_screen(1);
		break;
	case SDL_SCANCODE_C | WITH_CTRL: /* ctrl-c: copy room */
		memcpy(copied_room_fg,&(edited.fg[T(drawn_room,0)]),30);
		memcpy(copied_room_bg,&(edited.bg[T(drawn_room,0)]),30);
		*answer_text="ROOM COPIED";
		*need_show_text=1;
		break;
	case SDL_SCANCODE_V | WITH_CTRL: /* ctrl-v: paste room */
		editor__paste_room(drawn_room);
		redraw_screen(1);
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
		result.concept.fg=edited.fg[aux];
		result.concept.bg=edited.bg[aux];
	} else {
		result.number=NO_TILE;
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
	if (target_tile.number==NO_TILE) return;
	origin_tile=room_api__private_get_tile_if_exists(map,origin,level_mask,x,y);
	if (origin_tile.number==NO_TILE) return;
	*total+=weight;
	if (origin_tile.number==target_tile.number) *result+=weight;
}

void room_api_measure_similarity(const tMap* map, tile_global_location_type origin, tile_global_location_type target, byte* level_mask, tProbability* probabilities) {
	float result=0,total=0; /* ORIGIN is iterating CURRENT */
	tile_packed_type current_tile;

/* printf("room_api_measure_similarity(tile_global_location_type or=(%d,%d) target=(%d,%d))\n",origin.room,origin.tilepos,target.room,target.tilepos); */

	current_tile=room_api__private_get_tile_if_exists(map,origin,level_mask,0,0);
	if (current_tile.number==NO_TILE) return; /* there is no tile here */

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
	for (j=0;j!=NO_TILE;j++)
		probability[j]=aux;

	for (i=0;i<NUMBER_OF_ROOMS*30;i++) {
		if (map->list[R_(i)])
			room_api_measure_similarity(map,i,tilepos,level_mask,probability);
	}

#ifdef __SCREEN_DEBUG__
	for (j=0;j!=NO_TILE;j++) {
		if (probability[j].count)
			printf("k=%d v=%f c=%d\n",j,probability[j].value,probability[j].count);
	}
#endif

	for (j=0;j!=NO_TILE;j++)
		total+=probability[j].value;
	random_prob=(total*rand())/RAND_MAX; /* what if random==0 ???? */

	if (random_prob==0) {
		free(probability);
		return (tile_packed_type)NO_TILE;
	}
	total=0;
	for (j=0;j!=NO_TILE;j++) {
		random_prob-=probability[j].value;
		if (random_prob<=0) {
			free(probability);
			return (tile_packed_type)j;
		}
	}
	free(probability);
	return (tile_packed_type)NO_TILE;
}

#endif
