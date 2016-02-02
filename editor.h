
/*
SDLPoP, a port/conversion of the DOS game Prince of Persia.
Copyright (C) 2013-2015  Dávid Nagy

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

#ifndef EDITOR_H
#define EDITOR_H

/* Useful types for level handling */

typedef short tile_global_location_type;

#define R(t) (((t)/30)+1)
#define R_(t) (((t)/30))
#define T(r,t) (((r)-1)*30+(t))
#define P(t) ((t)%30)

#define NO_TILE ((tile_packed_type){.number=(Uint16)(-1)})
#define NO_TILE_ {.number=-1}
#define TP_(f,b) ((tile_packed_type){.concept={.fg=f,.bg=b}})
/* tile_packed_type TP(level_type level, tile_global_location_type location) */
#define TP(level,location) TP_(level.fg[location],level.bg[location])

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

/* Hooks */
void editor__process_key(int key,const char** answer_text, word* need_show_text);
void editor__handle_mouse(SDL_Event e,const Uint8* states);
void editor__loading_dat();

/* Seg007 */
void get_doorlink(Uint16 value, tile_global_location_type* tp, short* next);
void set_doorlink(Uint16* value, tile_global_location_type tp, short next);

/* MAP API */
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

/* DOOR API */
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
void door_api_refresh(int* max_doorlinks, tMap* map, int* selected_door_tile);
void door_api_swap(const int* max_doorlinks, int r1,int r2);

typedef enum {
	cButton=2,
	cDoor=1,
	cOther=0
} tTileDoorType;

tTileDoorType door_api_is_related(byte tile);

#endif
