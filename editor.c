/*
SDLPoP editor module
Copyright (C) 2013-2016  Dávid Nagy, Enrique Calot

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
	room fix errors.
	room decorate.

	door handling
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

typedef struct {
	long* list;
	byte* map;
} tMap;

long room_api_where_room(const tMap* map, byte room);
void room_api_refresh(tMap* map);
void room_api_init(tMap* map);
void room_api_free(tMap* map);
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

typedef struct {
	byte room;
	byte tilepos;
} tTilePlace;

#define NO_TILE ((word)(-1))

typedef struct probability_info {
	int count;
	float value;
} tProbability;

float room_api_measure_entropy(const tMap* map, tTilePlace t, byte* level_mask);
tile_packed_type room_api_suggest_tile(const tMap* map, tTilePlace tilepos, byte* level_mask);

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
level_type edited;
tMap edited_map={NULL,NULL};
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
*            Editor functions                *
\********************************************/

chtab_type* chtab_editor_sprites=NULL;
void editor__load_level() {
	dat_type* dathandle;

	dathandle = open_dat("LEVELS.DAT", 0);
	load_from_opendats_to_area(current_level + 2000, &edited, sizeof(edited), "bin");
	close_dat(dathandle);
	edition_level=current_level;
	stack_reset();

	dathandle = open_dat("editor", 0);
	if (chtab_editor_sprites) free_chtab(chtab_editor_sprites);
	chtab_editor_sprites = load_sprites_from_file(200, 1<<11, 1);
	close_dat(dathandle);

	//TODO: free_chtab(chtab_editor_sprites);
}

void editor_revert_level() {
	editor__load_level();
	level=edited;
	room_api_free(&edited_map);
	room_api_init(&edited_map);
	is_restart_level = 1;
}

void editor__loading_dat() {
	if (edition_level!=current_level) {
		editor__load_level();
		room_api_free(&edited_map);
		room_api_init(&edited_map);
	} else {
		level=edited; /* TODO: when new palettes are added this should change */
	}
}

void editor__position(char_type* character,int col,int row,int room,int x,word seq) {
	character->curr_col=col;
	character->curr_row=row;
	character->room=room;
	character->x=x;
	character->y=55+63*row;
	character->curr_seq=seq;
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

void save_level() {
	save_resource("LEVELS.DAT",current_level + 2000, &edited, sizeof(edited), "bin");
}

void editor__set_guard(byte tilepos,byte x) {
	printf("tile %d\n",level.guards_tile[loaded_room-1]);
	printf("c %d\n",level.guards_color[loaded_room-1]);
	printf("x %d\n",level.guards_x[loaded_room-1]);
	printf("dir %d\n",level.guards_dir[loaded_room-1]);
	printf("skill %d\n",level.guards_skill[loaded_room-1]);
	if (level.guards_tile[loaded_room-1]>=30) {
		editor__do(guards_tile[loaded_room-1],tilepos,mark_start);
		editor__do(guards_color[loaded_room-1],0,mark_middle);
		editor__do(guards_x[loaded_room-1],x,mark_middle);
		editor__do(guards_dir[loaded_room-1],0,mark_middle);
		editor__do(guards_skill[loaded_room-1],0,mark_end);
	} else {
		editor__do(guards_tile[loaded_room-1],tilepos,mark_all);
	}
}

void editor__remove_guard() {
	Guard.alive=0;
	guardhp_delta = -guardhp_curr;
	Guard.room=NUMBER_OF_ROOMS+1; /* TODO: fix this... sending a guard to a buffer overflow */
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
	tTilePlace t;
	tile_packed_type tt;
	int i,tile=-1;
	float max=0;

	memset(level_mask,1,NUMBER_OF_ROOMS*30);
	memset(level_mask+(room-1)*30,0,30);

	do {
		tile=-1;
		max=0;
		for (i=0;i<NUMBER_OF_ROOMS*30;i++)
			if (!level_mask[i] && edited_map.list[i/30]) {
				t.room=(i/30)+1;
				t.tilepos=i%30;
				f=room_api_measure_entropy(&edited_map,t,level_mask);
				if (max<f) {
					max=f;
					tile=i;
				}
			}
		if (tile!=-1) {
			t.room=(tile/30)+1;
			t.tilepos=tile%30;
			tt=room_api_suggest_tile(&edited_map,t,level_mask);
			editor__do(fg[tile],tt.concept.fg,mark_start); /* TODO: fix marks */
			editor__do(bg[tile],tt.concept.bg,mark_end);
			level_mask[tile]=1;
		}
	} while (tile!=-1);
	sanitize_room(room,0);
	sanitize_room(room,1);
}

void randomize_tile(int tilepos) {
	byte level_mask[NUMBER_OF_ROOMS*30];
	tTilePlace t;
	tile_packed_type tt;
	t.room=loaded_room;
	t.tilepos=tilepos;
	memset(level_mask,1,NUMBER_OF_ROOMS*30);
	level_mask[30*(t.room-1)+t.tilepos]=0;
	tt=room_api_suggest_tile(&edited_map,t,level_mask);
	editor__do(fg[(loaded_room-1)*30+tilepos],tt.concept.fg,mark_start);
	editor__do(bg[(loaded_room-1)*30+tilepos],tt.concept.bg,mark_end);
}

void sanitize_room(int room, int sanitation_level) {
	#define tile_at(tilepos,room) (edited.fg[(room-1)*30+(tilepos)]&0x1f)
	#define room_link edited.roomlinks[room-1]
	#define up_is(x,def) ((x>=10)?tile_at(x-10,room):(room_link.up?tile_at(x+20,room_link.up):def))
	#define down_is(x,def) ((x<20)?tile_at(x+10,room):(room_link.down?tile_at(x-20,room_link.down):def))
	#define left_is(x,def) ((x%10)?tile_at(x-1,room):(room_link.left?tile_at(x+9,room_link.left):def))
	#define right_is(x,def) ((x%10!=9)?tile_at(x+1,room):(room_link.right?tile_at(x-9,room_link.right):def))

	int i;
	byte tile;
	for (i=0;i<30;i++) {
		tile=tile_at(i,room);
		//printf("sanitize %d %d %d\n",i,tile,(edited.bg[(room-1)*30+(i)]));
		if (sanitation_level==1) switch(tile) { /* check for alone tile */
		case tiles_1_floor:
		case tiles_2_spike:
		case tiles_3_pillar:
		case tiles_4_gate:
		/* case tiles_5_stuck: */
		case tiles_6_closer:
		case tiles_7_doortop_with_floor:
		case tiles_8_bigpillar_bottom:
		case tiles_9_bigpillar_top:
		case tiles_10_potion:
		case tiles_12_doortop:
		case tiles_13_mirror:
		case tiles_14_debris:
		case tiles_15_opener:
		case tiles_16_level_door_left:
		case tiles_17_level_door_right:
		case tiles_18_chomper:
		case tiles_19_torch:
		case tiles_21_skeleton:
		case tiles_22_sword:
		case tiles_23_balcony_left:
		case tiles_24_balcony_right:
		case tiles_25_lattice_pillar:
		case tiles_26_lattice_down:
		case tiles_27_lattice_small:
		case tiles_28_lattice_left:
		case tiles_29_lattice_right:
		case tiles_30_torch_with_debris:
			if (left_is(i,tiles_20_wall)==tiles_20_wall && right_is(i,tiles_20_wall)==tiles_20_wall) {
				switch (up_is(i,tiles_20_wall)) {
				case tiles_1_floor:
				case tiles_2_spike:
				case tiles_3_pillar:
				case tiles_4_gate:
				/* case tiles_5_stuck: */
				case tiles_6_closer:
				case tiles_7_doortop_with_floor:
				case tiles_8_bigpillar_bottom:
				case tiles_9_bigpillar_top:
				case tiles_10_potion:
				case tiles_12_doortop:
				case tiles_13_mirror:
				case tiles_14_debris:
				case tiles_15_opener:
				case tiles_16_level_door_left:
				case tiles_17_level_door_right:
				case tiles_18_chomper:
				case tiles_19_torch:
				case tiles_20_wall:
				case tiles_21_skeleton:
				case tiles_22_sword:
				case tiles_23_balcony_left:
				case tiles_24_balcony_right:
				case tiles_25_lattice_pillar:
				case tiles_26_lattice_down:
				case tiles_27_lattice_small:
				case tiles_28_lattice_left:
				case tiles_29_lattice_right:
				case tiles_30_torch_with_debris:
					editor__do(fg[(room-1)*30+i],tiles_20_wall,mark_start);
					editor__do(bg[(room-1)*30+i],0,mark_end);
					break;
				}
			}
			break;
		}
		switch(tile) {
		case tiles_11_loose:
		case tiles_0_empty: /* empty above pillar or wall. TODO: add more tiles */
			if (sanitation_level==0) {
				if (down_is(i,-1)==tiles_20_wall || down_is(i,-1)==tiles_3_pillar) {
					editor__do(fg[(room-1)*30+i],tiles_1_floor,mark_start);
					editor__do(bg[(room-1)*30+i],0,mark_end);
				}
			} else if (sanitation_level==1) {
				if (right_is(i,-1)==tiles_20_wall && left_is(i,-1)==tiles_20_wall && up_is(i,tiles_20_wall)==tiles_20_wall) {
					editor__do(fg[(room-1)*30+i],tiles_20_wall,mark_start);
					editor__do(bg[(room-1)*30+i],0,mark_end);
				}
			}
			break;
		case tiles_19_torch:
			if (right_is(i,-1)==tiles_20_wall || right_is(i,-1)==tiles_3_pillar) {
				editor__do(fg[(room-1)*30+i],tiles_1_floor,mark_start);
				editor__do(bg[(room-1)*30+i],0,mark_end);
			}
			break;
		case tiles_30_torch_with_debris:
			if (right_is(i,-1)==tiles_20_wall || right_is(i,-1)==tiles_3_pillar) {
				editor__do(fg[(room-1)*30+i],tiles_14_debris,mark_start);
				editor__do(bg[(room-1)*30+i],0,mark_end);
			}
			break;
		case tiles_4_gate:
			if (right_is(i,-1)==tiles_20_wall || left_is(i,-1)==tiles_20_wall) {
				editor__do(fg[(room-1)*30+i],tiles_20_wall,mark_start);
				editor__do(bg[(room-1)*30+i],0,mark_end);
			}
			break;
		case tiles_2_spike:
			if (sanitation_level==1 && down_is(i,tiles_20_wall)!=tiles_20_wall) {
				editor__do(fg[(room-1)*30+i],tiles_1_floor,mark_start);
				editor__do(bg[(room-1)*30+i],0,mark_end);
			}
			break;
		}

		if (left_is(i,-1)==tiles_16_level_door_left) {
				editor__do(fg[(room-1)*30+i],tiles_17_level_door_right,mark_start);
				editor__do(bg[(room-1)*30+i],0,mark_end);
		}
		if (right_is(i,-1)==tiles_17_level_door_right) {
				editor__do(fg[(room-1)*30+i],tiles_16_level_door_left,mark_start);
				editor__do(bg[(room-1)*30+i],0,mark_end);
		}


		//TODO: finish & undo/redo
	}



}


/********************************************\
*             INPUT BINDINGS!!!!             *
\********************************************/

void editor__on_refresh(surface_type* screen) {
	if (chtab_editor_sprites) {
		int x,y;
		if (!SDL_GetMouseState(&x,&y)) {
			const Uint8 *state = SDL_GetKeyboardState(NULL);
			x=x/2;
			y=y/2;
			if (state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT]) {
				SDL_Rect dest_rect = {x-5, y-5, 10, 10};
				SDL_FillRect(screen,&dest_rect,0xff804000);
			} else {
				SDL_Rect dest_rect = {x-10, y-10, 20, 20};
				SDL_FillRect(screen,&dest_rect,0xc3c3c300);
			}
		}

		/* Example
		// draw temporary layer
		image_type* image=chtab_editor_sprites->images[0];
		SDL_Rect src_rect = {0, 0, image->w, image->h};
		if (SDL_BlitSurface(image, &src_rect, screen, &dest_rect) != 0) {
			sdlperror("SDL_BlitSurface on editor");
			quit(1);
		}*/
	} else {
		printf("Sprites not loaded\n");
	}

}

void editor__handle_mouse_button(SDL_MouseButtonEvent e,int shift, int ctrl, int alt) {
	int col,row,tilepos,x;
	if (!editor_active) return;
	col=e.x/32;
	row=(e.y-4)/64;
	x=e.x*140/320+62;
	if (row<0 || row>2) return;
	tilepos=row*10+col;

	if (e.button==SDL_BUTTON_LEFT && !shift && !alt && !ctrl) { /* left click: edit tile */
		editor__do(fg[(loaded_room-1)*30+tilepos],copied.tiletype,mark_start);
		editor__do(bg[(loaded_room-1)*30+tilepos],copied.modifier,mark_end);
		redraw_screen(1);
	} else if (e.button==SDL_BUTTON_RIGHT && !shift && !alt && !ctrl) { /* right click: copy tile */
		copied.tiletype=edited.fg[(loaded_room-1)*30+tilepos];
		copied.modifier=edited.bg[(loaded_room-1)*30+tilepos];
	} else if (e.button==SDL_BUTTON_LEFT && shift && !alt && !ctrl) { /* shift+left click: move kid */
		editor__position(&Kid,col,row,loaded_room,x,6563);
	} else if (e.button==SDL_BUTTON_RIGHT && shift && !alt && !ctrl) { /* shift+right click: move move/put guard */
		editor__set_guard(tilepos,x);
		editor__position(&Guard,col,row,loaded_room,x,6569);
		redraw_screen(1);
	} else if (e.button==SDL_BUTTON_LEFT && shift && !alt && ctrl) { /* ctrl+shift+left click: randomize tile */
		randomize_tile(tilepos);
		redraw_screen(1);
	}

				/*printf("hola mundo %d %d %d %d %c%c%c\n",
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
	switch (key) {
	case SDL_SCANCODE_Z | WITH_CTRL: /* ctrl-z */
		editor__undo();
		redraw_screen(1);
		break;
	case SDL_SCANCODE_Z | WITH_CTRL | WITH_ALT: /* ctrl-alt-z */
		editor__redo();
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
			next_room=aux_int;
			snprintf(aux,50,"Added S%d",aux_int);
			*answer_text=aux;
		} else {
			*answer_text="NO MORE SCREENS AVAILABLE";
		}
		editor__do_mark_end();
		*need_show_text=1;
		break;
	case SDL_SCANCODE_D: // d for debugging purposes
		{
			*answer_text="DEBUG ACTION";
			*need_show_text=1;
		}
		break;
	case SDL_SCANCODE_Y: /* y: save starting position */
		editor__do(start_pos,Kid.curr_row*10+Kid.curr_col,mark_start);
		editor__do(start_room,Kid.room,mark_middle);
		editor__do(start_dir,Kid.direction,mark_end);
		*answer_text="New starting position set";
		*need_show_text=1;
		break;
	case SDL_SCANCODE_R | WITH_CTRL | WITH_SHIFT: /* ctrl-shift-r */
		randomize_room(loaded_room);
		redraw_screen(1);
		break;
	case SDL_SCANCODE_S | WITH_CTRL | WITH_SHIFT: /* ctrl-shift-s */
		sanitize_room(loaded_room,0);
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

	room_api__private_recurse(map,MAP_POS(MAP_CENTER,MAP_CENTER),edited.start_room);

#ifdef __SCREEN_DEBUG__
	{
		int i=0;
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
/* Randomize tiles, rooms and complete levels using statistical inference.  */


tTilePlace room_api_tile_move(const tMap* map, tTilePlace t, char col, char row) {
	int absolute_col,absolute_row,where;

	where=map->list[t.room-1];
	absolute_col= 10 * (where%MAP_SIDE) + t.tilepos%10 + col;
	absolute_row= 3  * (where/MAP_SIDE) + t.tilepos/10 + row;

	t.room=map->map[MAP_POS(absolute_col/10,absolute_row/3)];
	t.tilepos=absolute_col%10 + 10 * (absolute_row%3);

	return t;
}

int room_api__private_entropy(const tMap* map, tTilePlace t, byte* level_mask,char col, char row) {
	tTilePlace aux;
	aux=room_api_tile_move(map,t,col,row);
	return aux.room && level_mask[(aux.room-1)*30+aux.tilepos];
}

tile_packed_type room_api__private_get_tile_if_exists(const tMap* map, tTilePlace t, byte* level_mask,char col, char row) {
	tTilePlace aux;
	tile_packed_type result;
	aux=room_api_tile_move(map,t,col,row);
	if (aux.room && level_mask[(aux.room-1)*30+aux.tilepos]) {
		result.concept.fg=edited.fg[(aux.room-1)*30+aux.tilepos];
		result.concept.bg=edited.bg[(aux.room-1)*30+aux.tilepos];
	} else {
		result.number=-1;
	}
	return result;
}

float room_api_measure_entropy(const tMap* map, tTilePlace t, byte* level_mask) {
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
void room_api__private_measure_similarity(const tMap* map,tTilePlace origin, tTilePlace target, byte* level_mask, int x, int y,float* result, float* total, float weight) {
	tile_packed_type target_tile, origin_tile;
	target_tile=room_api__private_get_tile_if_exists(map,target,level_mask,x,y);
	if (target_tile.number==NO_TILE) return;
	origin_tile=room_api__private_get_tile_if_exists(map,origin,level_mask,x,y);
	if (origin_tile.number==NO_TILE) return;
	*total+=weight;
	if (origin_tile.number==target_tile.number) *result+=weight;
}

void room_api_measure_similarity(const tMap* map, tTilePlace origin, tTilePlace target, byte* level_mask, tProbability* probabilities) {
	float result=0,total=0; /* ORIGIN is iterating CURRENT */
	tile_packed_type current_tile;

/* printf("room_api_measure_similarity(tTilePlace or=(%d,%d) target=(%d,%d))\n",origin.room,origin.tilepos,target.room,target.tilepos); */

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
tile_packed_type room_api_suggest_tile(const tMap* map, tTilePlace tilepos, byte* level_mask) {
	tTilePlace origin;
	tProbability* probability;
	tProbability aux={0,0};
	word i;
	float total=0,random_prob;

	probability=malloc(PROBABILITY_SIZE*sizeof(tProbability));
	for (i=0;i!=NO_TILE;i++)
		probability[i]=aux;

	for (i=0;i<NUMBER_OF_ROOMS*30;i++) {
		origin.room=i/30;
		if (map->list[origin.room++]) {
			origin.tilepos=i%30;
			room_api_measure_similarity(map,origin,tilepos,level_mask,probability);
		}
	}

#ifdef __SCREEN_DEBUG__
	for (i=0;i!=NO_TILE;i++) {
		if (probability[i].count)
			printf("k=%d v=%f c=%d\n",i,probability[i].value,probability[i].count);
	}
#endif

	for (i=0;i!=NO_TILE;i++)
		total+=probability[i].value;
	random_prob=(total*rand())/RAND_MAX; /* what if random==0 ???? */

	if (random_prob==0) {
		free(probability);
		return (tile_packed_type)NO_TILE;
	}
	total=0;
	for (i=0;i!=NO_TILE;i++) {
		random_prob-=probability[i].value;
		if (random_prob<=0) {
			free(probability);
			return (tile_packed_type)i;
		}
	}
	free(probability);
	return (tile_packed_type)NO_TILE;
}


#endif
