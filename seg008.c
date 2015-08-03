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

#include "common.h"

// data:27E0
add_table_type ptr_add_table = add_backtable;

// data:259C
const piece tile_table[31] = {
{   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}, // 0x00 empty
{  41,   1,   0,  42,   1,   2, 145,   0,  43,   0,   0,   0}, // 0x01 floor
{ 127,   1,   0, 133,   1,   2, 145,   0,  43,   0,   0,   0}, // 0x02 spike
{  92,   1,   0,  93,   1,   2,   0,  94,  43,  95,   1,   0}, // 0x03 pillar
{  46,   1,   0,  47,   1,   2,   0,  48,  43,  49,   3,   0}, // 0x04 door
{  41,   1,   1,  35,   1,   3, 145,   0,  36,   0,   0,   0}, // 0x05 stuck floor
{  41,   1,   0,  42,   1,   2, 145,   0,  96,   0,   0,   0}, // 0x06 close button
{  46,   1,   0,   0,   0,   2,   0,   0,  43,  49,   3,   0}, // 0x07 door top with floor
{  86,   1,   0,  87,   1,   2,   0,   0,  43,  88,   1,   0}, // 0x08 big pillar bottom
{   0,   0,   0,  89,   0,   3,   0,  90,   0,  91,   1,   3}, // 0x09 big pillar top
{  41,   1,   0,  42,   1,   2, 145,   0,  43,  12,   2,  -3}, // 0x0A potion
{   0,   1,   0,   0,   0,   0, 145,   0,   0,   0,   0,   0}, // 0x0B loose floor
{   0,   0,   0,   0,   0,   2,   0,   0,  85,  49,   3,   0}, // 0x0C door top
{  75,   1,   0,  42,   1,   2,   0,   0,  43,  77,   0,   0}, // 0x0D mirror
{  97,   1,   0,  98,   1,   2, 145,   0,  43, 100,   0,   0}, // 0x0E debris
{ 147,   1,   0,  42,   1,   1, 145,   0, 149,   0,   0,   0}, // 0x0F open button
{  41,   1,   0,  37,   0,   0,   0,  38,  43,   0,   0,   0}, // 0x10 leveldoor left
{   0,   0,   0,  39,   1,   2,   0,  40,  43,   0,   0,   0}, // 0x11 leveldoor right
{   0,   0,   0,  42,   1,   2, 145,   0,  43,   0,   0,   0}, // 0x12 chomper
{  41,   1,   0,  42,   1,   2,   0,   0,  43,   0,   0,   0}, // 0x13 torch
{   0,   0,   0,   1,   1,   2,   0,   2,   0,   0,   0,   0}, // 0x14 wall
{  30,   1,   0,  31,   1,   2,   0,   0,  43,   0,   0,   0}, // 0x15 skeleton
{  41,   1,   0,  42,   1,   2, 145,   0,  43,   0,   0,   0}, // 0x16 sword
{  41,   1,   0,  10,   0,   0,   0,  11,  43,   0,   0,   0}, // 0x17 balcony left
{   0,   0,   0,  12,   1,   2,   0,  13,  43,   0,   0,   0}, // 0x18 balcony right
{  92,   1,   0,  42,   1,   2, 145,   0,  43,  95,   1,   0}, // 0x19 lattice pillar
{   1,   0,   0,   0,   0,   0,   0,   0,   2,   9,   0, -53}, // 0x1A lattice down
{   3,   0, -10,   0,   0,   0,   0,   0,   0,   9,   0, -53}, // 0x1B lattice small
{   4,   0, -10,   0,   0,   0,   0,   0,   0,   9,   0, -53}, // 0x1C lattice left
{   5,   0, -10,   0,   0,   0,   0,   0,   0,   9,   0, -53}, // 0x1D lattice right
{  97,   1,   0,  98,   1,   2,   0,   0,  43, 100,   0,   0}, // 0x1E debris with torch
};

// data:4334
short drawn_row;
// data:4352
short draw_bottom_y;
// data:4326
short draw_main_y;
// data:34D0
short drawn_col;

// data:6592
byte tile_left;
// data:4CCC
byte modifier_left;

// seg008:0006
void __pascal far redraw_room() {
	free_peels();
	memset_near(table_counts, 0, sizeof(table_counts));
	reset_obj_clip();
	draw_room();
	clear_tile_wipes();
}

// seg008:0035
void __pascal far load_room_links() {
	room_BR = 0;
	room_BL = 0;
	room_AR = 0;
	room_AL = 0;
	if (drawn_room) {
		get_room_address(drawn_room);
		room_L = level.roomlinks[drawn_room-1].left;
		room_R = level.roomlinks[drawn_room-1].right;
		room_A = level.roomlinks[drawn_room-1].up;
		room_B = level.roomlinks[drawn_room-1].down;
		if (room_A) {
			room_AL = level.roomlinks[room_A-1].left;
			room_AR = level.roomlinks[room_A-1].right;
		} else {
			if (room_L) {
				room_AL = level.roomlinks[room_L-1].up;
			}
			if (room_R) {
				room_AR = level.roomlinks[room_R-1].up;
			}
		}
		if (room_B) {
			room_BL = level.roomlinks[room_B-1].left;
			room_BR = level.roomlinks[room_B-1].right;
		} else {
			if (room_L) {
				room_BL = level.roomlinks[room_L-1].down;
			}
			if (room_R) {
				room_BR = level.roomlinks[room_R-1].down;
			}
		}
	} else {
		room_B = 0;
		room_A = 0;
		room_R = 0;
		room_L = 0;
	}
}

// seg008:0125
void __pascal far draw_room() {
	word saved_room;
	load_leftroom();
	for (drawn_row = 3; drawn_row--; ) { /*2,1,0*/
		load_rowbelow();
		draw_bottom_y = 63 * drawn_row + 65;
		draw_main_y = draw_bottom_y - 3;
		for (drawn_col = 0; drawn_col < 10; drawn_col++) {
			load_curr_and_left_tile();
			draw_tile();
		}
	}
	saved_room = drawn_room;
	drawn_room = room_A;
	load_room_links();
	load_leftroom();
	drawn_row = 2;
	load_rowbelow();
	for (drawn_col = 0; drawn_col < 10; ++drawn_col) {
		load_curr_and_left_tile();
		draw_main_y = -1;
		draw_bottom_y = 2;
		draw_tile_aboveroom();
	}
	drawn_room = saved_room;
	load_room_links();
}

// seg008:01C7
void __pascal far draw_tile() {
	draw_tile_floorright();
	draw_tile_anim_topright();
	draw_tile_right();
	draw_tile_anim_right();
	draw_tile_bottom(0);
	draw_loose(0);
	draw_tile_base();
	draw_tile_anim();
	draw_tile_fore();
}

// seg008:01F2
void __pascal far draw_tile_aboveroom() {
	draw_tile_floorright();
	draw_tile_anim_topright();
	draw_tile_right();
	draw_tile_bottom(0);
	draw_loose(0);
	draw_tile_fore();
}

// seg008:0211
void __pascal far redraw_needed(short tilepos) {
	if (wipe_frames[tilepos]) {
		--wipe_frames[tilepos];
		draw_tile_wipe(wipe_heights[tilepos]);
	}
	if (redraw_frames_full[tilepos]) {
		--redraw_frames_full[tilepos];
		draw_tile();
	}
	if (redraw_frames_anim[tilepos]) {
		--redraw_frames_anim[tilepos];
		draw_tile_anim_topright();
		draw_tile_anim_right();
		draw_tile_anim();
	}
	if (redraw_frames2[tilepos]) {
		--redraw_frames2[tilepos];
		draw_other_overlay();
	} else {
		if (redraw_frames_floor_overlay[tilepos]) {
			--redraw_frames_floor_overlay[tilepos];
			draw_floor_overlay();
		}
	}
	if (tile_object_redraw[tilepos]) {
		if (tile_object_redraw[tilepos] == 0xFF) {
			draw_objtable_items_at_tile(tilepos - 1);
		}
		draw_objtable_items_at_tile(tilepos);
		tile_object_redraw[tilepos] = 0;
	}
	if (redraw_frames_fore[tilepos]) {
		--redraw_frames_fore[tilepos];
		draw_tile_fore();
	}
}

// seg008:02C1
void __pascal far redraw_needed_above(int column) {
	if (redraw_frames_above[column] != 0) {
		--redraw_frames_above[column];
		draw_tile_wipe(3);
		draw_tile_floorright();
		draw_tile_anim_topright();
		draw_tile_right();
		draw_tile_bottom(1);
		draw_loose(1);
		draw_tile_fore();
	}
}

// seg008:02FE
int __pascal far get_tile_to_draw(int room, int column, int row, byte *ptr_tiletype, byte *ptr_modifier, byte tile_room0) {
	word tilepos;
	if (column == -1) {
		*ptr_tiletype = leftroom_[row].tiletype;
		*ptr_modifier = leftroom_[row].modifier;
	} else if (room) {
		tilepos = tbl_line[row] + column;
		*ptr_tiletype = curr_room_tiles[tilepos] & 0x1F;
		*ptr_modifier = curr_room_modif[tilepos];
	} else {
		*ptr_modifier = 0;
		*ptr_tiletype = tile_room0;
	}
	// Is this a pressed button?
	byte tiletype = *ptr_tiletype;
	if (tiletype == tiles_6_closer) {
		if (get_doorlink_timer(*ptr_modifier) > 1) {
			*ptr_tiletype = tiles_5_stuck;
		}
	} else if (tiletype == tiles_15_opener) {
		if (get_doorlink_timer(*ptr_modifier) > 1) {
			*ptr_modifier = 0;
			*ptr_tiletype = tiles_1_floor;
		}
	} else if (tiletype == tiles_11_loose) {
		if (*ptr_modifier == 0) {
			*ptr_tiletype = tiles_1_floor;
		}
	}
	return *ptr_tiletype;
}

// data:24C6
const word col_xh[] = {0, 4, 8, 12, 16, 20, 24, 28, 32, 36};
// seg008:03BB
void __pascal far load_curr_and_left_tile() {
	word tiletype;
	tiletype = tiles_20_wall;
	if (drawn_row == 2) {
		tiletype = tiles_1_floor; // floor at top of level
	}
	get_tile_to_draw(drawn_room, drawn_col, drawn_row, &curr_tile, &curr_modifier, tiletype);
	get_tile_to_draw(drawn_room, drawn_col - 1, drawn_row, &tile_left, &modifier_left, tiletype);
	draw_xh = col_xh[drawn_col];
}

// seg008:041A
void __pascal far load_leftroom() {
	word row;
	get_room_address(room_L);
	for (row = 0; row < 3; ++row) {
		// wall at left of level
		get_tile_to_draw(room_L, 9, row, &leftroom_[row].tiletype, &leftroom_[row].modifier, tiles_20_wall);
	}
}

// seg008:0460
void __pascal far load_rowbelow() {
	word row_below;
	word column;
	word room;
	word room_left;
	if (drawn_row == 2) {
		room = room_B;
		room_left = room_BL;
		row_below = 0;
	} else {
		room = drawn_room;
		room_left = room_L;
		row_below = drawn_row + 1;
	}
	get_room_address(room);
	for (column = 1; column < 10; ++column) {
		get_tile_to_draw(room, column - 1, row_below, &row_below_left_[column].tiletype, &row_below_left_[column].modifier, tiles_0_empty);
	}
	get_room_address(room_left);
	// wall at left of level
	get_tile_to_draw(room_left, 9, row_below, &row_below_left_[0].tiletype, &row_below_left_[0].modifier, tiles_20_wall);
	get_room_address(drawn_room);
}

// seg008:04FA
void __pascal far draw_tile_floorright() {
	if (can_see_bottomleft() == 0) return;
	draw_tile_topright();
	if (tile_table[tile_left].floor_right == 0) return;
	add_backtable(id_chtab_6_environment, 42 /*floor right part*/, draw_xh, 0, tile_table[tiles_1_floor].right_y + draw_main_y, blitters_9_black, 1);
}

// seg008:053A
int __pascal far can_see_bottomleft() {
	return curr_tile == tiles_0_empty ||
		curr_tile == tiles_9_bigpillar_top ||
		curr_tile == tiles_12_doortop ||
		curr_tile == tiles_26_lattice_down;
}

const byte doortop_fram_top[] = {0, 81, 83, 0};

// seg008:055A
void __pascal far draw_tile_topright() {
	byte tiletype;
	tiletype = row_below_left_[drawn_col].tiletype;
	if (tiletype == tiles_7_doortop_with_floor || tiletype == tiles_12_doortop) {
		if (tbl_level_type[current_level] == 0) return;
		add_backtable(id_chtab_6_environment, doortop_fram_top[row_below_left_[drawn_col].modifier], draw_xh, 0, draw_bottom_y, blitters_2_or, 0);
	} else if (tiletype == tiles_20_wall) {
		add_backtable(id_chtab_7_environmentwall, 2, draw_xh, 0, draw_bottom_y, blitters_2_or, 0);
	} else {
		add_backtable(id_chtab_6_environment, tile_table[tiletype].topright_id, draw_xh, 0, draw_bottom_y, blitters_2_or, 0);
	}
}

const byte door_fram_top[] = {60, 61, 62, 63, 64, 65, 66, 67};

// seg008:05D1
void __pascal far draw_tile_anim_topright() {
	word modifier;
	if (	(curr_tile == tiles_0_empty ||
		curr_tile == tiles_9_bigpillar_top ||
		curr_tile == tiles_12_doortop)
		&& row_below_left_[drawn_col].tiletype == tiles_4_gate
	) {
		add_backtable(id_chtab_6_environment, 68 /*gate top mask*/, draw_xh, 0, draw_bottom_y, blitters_40h_mono, 0);
		modifier = row_below_left_[drawn_col].modifier;
		if (modifier > 188) modifier = 188;
		add_backtable(id_chtab_6_environment, door_fram_top[(modifier>>2) % 8], draw_xh, 0, draw_bottom_y, blitters_2_or, 0);
	}
}

const byte blueline_fram1[] = {0, 124, 125, 126};
const sbyte blueline_fram_y[] = {0, -20, -20, 0};
const byte blueline_fram3[] = {44, 44, 45, 45};
const byte doortop_fram_bot[] = {78, 80, 82, 0};

// seg008:066A
void __pascal far draw_tile_right() {
	byte id;
	byte blit;
	byte var_2;
	if (curr_tile == tiles_20_wall) return;
	switch (tile_left) {
		default:
			id = tile_table[tile_left].right_id;
			if (id) {
				if (tile_left == tiles_5_stuck) {
					blit = blitters_10h_transp;
					if (curr_tile == tiles_0_empty || curr_tile == tiles_5_stuck) {
						id = 42; /*floor B*/
					}
				} else {
					blit = blitters_2_or;
				}
				add_backtable(id_chtab_6_environment, id, draw_xh, 0, tile_table[tile_left].right_y + draw_main_y, blit, 0);
			}
			if (tbl_level_type[current_level] != 0) {
				add_backtable(id_chtab_6_environment, tile_table[tile_left].stripe_id, draw_xh, 0, draw_main_y - 27, blitters_2_or, 0);
			}
			if (tile_left == tiles_19_torch || tile_left == tiles_30_torch_with_debris) {
				add_backtable(id_chtab_6_environment, 146 /*torch base*/, draw_xh, 0, draw_bottom_y - 28, blitters_0_no_transp, 0);
			}
			break;
		case tiles_0_empty:
			if (modifier_left > 3) return;
			add_backtable(id_chtab_6_environment, blueline_fram1[modifier_left], draw_xh, 0, blueline_fram_y[modifier_left] + draw_main_y, blitters_2_or, 0);
			break;
		case tiles_1_floor:
			ptr_add_table(id_chtab_6_environment, 42 /*floor B*/, draw_xh, 0, tile_table[tile_left].right_y + draw_main_y, blitters_10h_transp, 0);
			var_2 = modifier_left;
			if (var_2 > 3) var_2 = 0;
			if (var_2 == !!tbl_level_type[current_level]) return;
			add_backtable(id_chtab_6_environment, blueline_fram3[var_2], draw_xh, 0, draw_main_y - 20, blitters_0_no_transp, 0);
			break;
		case tiles_7_doortop_with_floor:
		case tiles_12_doortop:
			if (tbl_level_type[current_level] == 0) return;
			add_backtable(id_chtab_6_environment, doortop_fram_bot[modifier_left], draw_xh, 0, tile_table[tile_left].right_y + draw_main_y, blitters_2_or, 0);
			break;
		case tiles_20_wall:
			if (tbl_level_type[current_level] && (modifier_left & 0x80) == 0) {
				add_backtable(id_chtab_6_environment, 84 /*wall stripe*/, draw_xh + 3, 0, draw_main_y - 27, blitters_0_no_transp, 0);
			}
			add_backtable(id_chtab_7_environmentwall, 1, draw_xh, 0, tile_table[tile_left].right_y + draw_main_y, blitters_2_or, 0);
			break;
	}
}

const byte spikes_fram_right[] = {0, 134, 135, 136, 137, 138, 137, 135, 134, 0};
const byte loose_fram_right[] = {42, 71, 42, 72, 72, 42, 42, 42, 72, 72, 72, 0};

// seg008:08A0
int __pascal far get_spike_frame(byte modifier) {
	if (modifier & 0x80) {
		return 5;
	} else {
		return modifier;
	}
}

// seg008:08B5
void __pascal far draw_tile_anim_right() {
	switch (tile_left) {
		case tiles_2_spike:
			add_backtable(id_chtab_6_environment, spikes_fram_right[get_spike_frame(modifier_left)], draw_xh, 0, draw_main_y - 7, blitters_10h_transp, 0);
		break;
		case tiles_4_gate:
			draw_gate_back();
		break;
		case tiles_11_loose:
			add_backtable(id_chtab_6_environment, loose_fram_right[get_loose_frame(modifier_left)], draw_xh, 0, draw_bottom_y - 1, blitters_2_or, 0);
		break;
		case tiles_16_level_door_left:
			draw_leveldoor();
		break;
		case tiles_19_torch:
		case tiles_30_torch_with_debris:
			if (modifier_left < 9) {
				// images 1..9 are the flames
				add_backtable(id_chtab_1_flameswordpotion, modifier_left + 1, draw_xh + 1, 0, draw_main_y - 40, blitters_0_no_transp, 0);
			}
		break;
	}
}

const byte wall_fram_bottom[] = {7, 9, 5, 3};

// seg008:0971
void __pascal far draw_tile_bottom(word arg_0) {
	word chtab_id;
	byte id;
	byte blit;
	id = 0;
	blit = blitters_0_no_transp;
	chtab_id = id_chtab_6_environment;
	switch (curr_tile) {
		case tiles_20_wall:
			if (tbl_level_type[current_level] == 0 || graphics_mode != gmMcgaVga) {
				id = wall_fram_bottom[curr_modifier & 0x7F];
			}
			chtab_id = id_chtab_7_environmentwall;
			break;
		case tiles_12_doortop:
			blit = blitters_2_or;
			// fallthrough!
		default:
			id = tile_table[curr_tile].bottom_id;
			break;
	}
	if (ptr_add_table(chtab_id, id, draw_xh, 0, draw_bottom_y, blit, 0) && arg_0) {
		add_foretable(chtab_id, id, draw_xh, 0, draw_bottom_y, blit, 0);
	}
	if (chtab_id == id_chtab_7_environmentwall && graphics_mode != gmCga && graphics_mode != gmHgaHerc) {
		wall_pattern(0, 0);
	}
}

const byte loose_fram_bottom[] = {43, 73, 43, 74, 74, 43, 43, 43, 74, 74, 74, 0};

// seg008:0A38
void __pascal far draw_loose(int arg_0) {
	word id;
	if (curr_tile == tiles_11_loose) {
		id = loose_fram_bottom[get_loose_frame(curr_modifier)];
		add_backtable(id_chtab_6_environment, id, draw_xh, 0, draw_bottom_y, blitters_0_no_transp, 0);
		add_foretable(id_chtab_6_environment, id, draw_xh, 0, draw_bottom_y, blitters_0_no_transp, 0);
	}
}

const byte loose_fram_left[] = {41, 69, 41, 70, 70, 41, 41, 41, 70, 70, 70, 0};

// seg008:0A8E
void __pascal far draw_tile_base() {
	word ybottom;
	word id;
	ybottom = draw_main_y;
	if (tile_left == tiles_26_lattice_down && curr_tile == tiles_12_doortop) {
		id = 6; // Lattice + door A
		ybottom += 3;
	} else if (curr_tile == tiles_11_loose) {
		id = loose_fram_left[get_loose_frame(curr_modifier)];
	} else if (curr_tile == tiles_15_opener && tile_left == tiles_0_empty && tbl_level_type[current_level] == 0) {
		id = 148; // left half of open button with floor to the left
	} else {
		id = tile_table[curr_tile].base_id;
	}
	ptr_add_table(id_chtab_6_environment, id, draw_xh, 0, tile_table[curr_tile].base_y + ybottom, blitters_10h_transp, 0);
}

const byte spikes_fram_left[] = {0, 128, 129, 130, 131, 132, 131, 129, 128, 0};
const byte potion_fram_bubb[] = {0, 16, 17, 18, 19, 20, 21, 22};
const byte chomper_fram1[] = {3, 2, 0, 1, 4, 3, 3, 0};
const byte chomper_fram_bot[] = {101, 102, 103, 104, 105, 0};
const byte chomper_fram_top[] = {0, 0, 111, 112, 113, 0};
const byte chomper_fram_y[] = {0, 0, 0x25, 0x2F, 0x32};

// seg008:0B2B
void __pascal far draw_tile_anim() {
	word color;
	word pot_size;
	word var_4;
	pot_size = 0;
	color = 12; // red
	switch (curr_tile) {
		case tiles_2_spike:
			ptr_add_table(id_chtab_6_environment, spikes_fram_left[get_spike_frame(curr_modifier)], draw_xh, 0, draw_main_y - 2, blitters_10h_transp, 0);
			break;
		case tiles_10_potion:
			switch((curr_modifier & 0xF8) >> 3) {
				case 0:
					return; //empty
				case 5: // hurt
				case 6: // open
					color = 9; // blue
					break;
				case 3: // slow fall
				case 4: // upside down
					color = 10; // green
					// fallthrough!
				case 2: // life
					pot_size = 1;
					break;
			}
			add_backtable(id_chtab_1_flameswordpotion, 23 /*bubble mask*/, draw_xh + 3, 1, draw_main_y - (pot_size << 2) - 14, blitters_40h_mono, 0);
			add_foretable(id_chtab_1_flameswordpotion, potion_fram_bubb[curr_modifier & 0x7], draw_xh + 3, 1, draw_main_y - (pot_size << 2) - 14, color + blitters_40h_mono, 0);
			break;
		case tiles_22_sword:
			add_midtable(id_chtab_1_flameswordpotion, (curr_modifier == 1) + 10, draw_xh, 0, draw_main_y - 3, blitters_10h_transp, curr_modifier == 1);
			break;
		case tiles_18_chomper:
			var_4 = chomper_fram1[MIN(curr_modifier & 0x7F, 6)];
			add_backtable(id_chtab_6_environment, chomper_fram_bot[var_4], draw_xh, 0, draw_main_y, blitters_10h_transp, 0);
			if (curr_modifier & 0x80) { // blood
				add_backtable(id_chtab_6_environment, var_4 + 114, draw_xh + 1, 4, draw_main_y - 6, blitters_4Ch_mono_12, 0);
			}
			add_backtable(id_chtab_6_environment, chomper_fram_top[var_4], draw_xh, 0, draw_main_y - chomper_fram_y[var_4], blitters_10h_transp, 0);
			break;
	}
}

const byte spikes_fram_fore[] = {0, 139, 140, 141, 142, 143, 142, 140, 139, 0};
const byte chomper_fram_for[] = {106, 107, 108, 109, 110, 0};
const byte wall_fram_main[] = {8, 10, 6, 4};

// seg008:0D15
void __pascal far draw_tile_fore() {
	word ybottom;
	byte xh;
	word potion_type;
	word id;
	word var_2;
	if (tile_left == tiles_4_gate && Kid.curr_row == drawn_row && Kid.curr_col == drawn_col - 1 && Kid.room != room_R) {
		draw_gate_fore();
	}
	switch (curr_tile) {
		case tiles_2_spike:
			add_foretable(id_chtab_6_environment, spikes_fram_fore[get_spike_frame(curr_modifier)], draw_xh, 0, draw_main_y - 2, blitters_10h_transp, 0);
			break;
		case tiles_18_chomper:
			var_2 = chomper_fram1[MIN(curr_modifier & 0x7F, 6)];
			add_foretable(id_chtab_6_environment, chomper_fram_for[var_2], draw_xh, 0, draw_main_y, blitters_10h_transp, 0);
			if (curr_modifier & 0x80) {
				add_foretable(id_chtab_6_environment, var_2 + 119, draw_xh + 1, 4, draw_main_y - 6, blitters_4Ch_mono_12, 0);
			}
			break;
		case tiles_20_wall:
			if (tbl_level_type[current_level] == 0 || graphics_mode != gmMcgaVga) {
				add_foretable(id_chtab_7_environmentwall, wall_fram_main[curr_modifier & 0x7F], draw_xh, 0, draw_main_y, blitters_0_no_transp, 0);
			}
			if (graphics_mode != gmCga && graphics_mode != gmHgaHerc) {
				wall_pattern(1, 1);
			}
			break;
		default:
			id = tile_table[curr_tile].fore_id;
			if (id == 0) return;
			if (curr_tile == tiles_10_potion) {
				// large pots are drawn for potion types 2, 3, 4
				potion_type = (curr_modifier & 0xF8) >> 3;
				if (potion_type < 5 && potion_type >= 2) id = 13; // small pot = 12, large pot = 13
			}
			xh = tile_table[curr_tile].fore_x + draw_xh;
			ybottom = tile_table[curr_tile].fore_y + draw_main_y;
			if (curr_tile == tiles_10_potion) {
				// potions look different in the dungeon and the palace
				if (tbl_level_type[current_level] != 0) id += 2;
				add_foretable(id_chtab_1_flameswordpotion, id, xh, 6, ybottom, blitters_10h_transp, 0);
			} else {
				if ((curr_tile == tiles_3_pillar && tbl_level_type[current_level] == 0) || (curr_tile >= tiles_27_lattice_small && curr_tile < tiles_30_torch_with_debris)) {
					add_foretable(id_chtab_6_environment, id, xh, 0, ybottom, blitters_0_no_transp, 0);
				} else {
					add_foretable(id_chtab_6_environment, id, xh, 0, ybottom, blitters_10h_transp, 0);
				}
			}
			break;
	}
}

// seg008:0FF6
int __pascal far get_loose_frame(byte modifier) {
	if (modifier & 0x80) {
		modifier &= 0x7F;
		if (modifier > 10) {
			return 1;
		}
	}
	return modifier;
}

// seg008:10A8
int __pascal far add_backtable(short chtab_id, int id, sbyte xh, sbyte xl, int ybottom, byte blit, byte peel) {
	word index;
	if (id == 0) {
		return 0;
	}
	index = backtable_count;
	if (index >= 200) {
		show_dialog("BackTable Overflow");
		return 0; // added
	}
	back_table_type* backtable_item = &backtable[index];
	backtable_item->xh = xh;
	backtable_item->xl = xl;
	backtable_item->chtab_id = chtab_id;
	backtable_item->id = id - 1;
	if (chtab_addrs[chtab_id]->images[id - 1] == NULL) {
		return 0;
	}
	backtable_item->y = ybottom - chtab_addrs[chtab_id]->images[id - 1]->h/*height*/ + 1;
	backtable_item->blit = blit;
	if (draw_mode) {
		draw_back_fore(0, index);
	}
	++backtable_count;
	return 1;
}

// seg008:1017
int __pascal far add_foretable(short chtab_id, int id, sbyte xh, sbyte xl, int ybottom, byte blit, byte peel) {
	word index;
	if (id == 0) return 0;
	index = foretable_count;
	if (index >= 200) {
		show_dialog("ForeTable Overflow");
		return 0; // added
	}
	back_table_type* foretable_item = &foretable[index];
	foretable_item->xh = xh;
	foretable_item->xl = xl;
	foretable_item->chtab_id = chtab_id;
	foretable_item->id = id - 1;
	if (chtab_addrs[chtab_id]->images[id - 1] == NULL) {
		return 0;
	}
	foretable_item->y = ybottom - chtab_addrs[chtab_id]->images[id - 1]->h/*height*/ + 1;
	foretable_item->blit = blit;
	if (draw_mode) {
		draw_back_fore(1, index);
	}
	++foretable_count;
	return 1;
}

// seg008:113A
int __pascal far add_midtable(short chtab_id, int id, sbyte xh, sbyte xl, int ybottom, byte blit, byte peel) {
	word index;
	if (id == 0) {
		return 0;
	}
	index = midtable_count;
	if (index >= 50) {
		show_dialog("MidTable Overflow");
		return 0; // added
	}
	midtable_type* midtable_item = &midtable[index];
	midtable_item->xh = xh;
	midtable_item->xl = xl;
	midtable_item->chtab_id = chtab_id;
	midtable_item->id = id - 1;
	if (id > chtab_addrs[chtab_id]->n_images) {
		printf("add_midtable: Tried to use image %d of chtab %d, not in 1..%d\n", id, chtab_id, chtab_addrs[chtab_id]->n_images);
		return 0;
	}
	if (chtab_addrs[chtab_id]->images[id - 1] == NULL) {
		return 0;
	}
	midtable_item->y = ybottom - chtab_addrs[chtab_id]->images[id - 1]->h/*height*/ + 1;
	if (obj_direction == dir_0_right && chtab_flip_clip[chtab_id] != 0) {
		blit += 0x80;
	}
	midtable_item->blit = blit;
	midtable_item->peel = peel;
	midtable_item->clip.left = obj_clip_left;
	midtable_item->clip.right = obj_clip_right;
	midtable_item->clip.top = obj_clip_top;
	midtable_item->clip.bottom = obj_clip_bottom;
	if (draw_mode) {
		draw_mid(index);
	}
	++midtable_count;
	return 1;
}

// seg008:1208
void __pascal far add_peel(int left,int right,int top,int height) {
	rect_type rect;
	if (peels_count >= 50) {
		show_dialog("Peels OverFlow");
		return /*0*/; // added
	}
	rect.left = left;
	rect.right = right;
	rect.top = top;
	rect.bottom = top + height;
	peels_table[peels_count++] = read_peel_from_screen(&rect);
}

// seg008:1254
void __pascal far add_wipetable(sbyte layer,short left,short bottom,sbyte height,short width,sbyte color) {
	word index;
	index = wipetable_count;
	if (index >= 300) {
		show_dialog("WipeTable Overflow");
		return /*0*/; // added
	}
	wipetable_type* wipetable_item = &wipetable[index];
	wipetable_item->left = left;
	wipetable_item->bottom = bottom + 1;
	wipetable_item->height = height;
	wipetable_item->width = width;
	wipetable_item->color = color;
	wipetable_item->layer = layer;
	if (draw_mode) {
		draw_wipe(index);
	}
	++wipetable_count;
}

// seg008:12BB
void __pascal far draw_table(int which_table) {
	short index;
	short count;
	count = table_counts[which_table];
	for (index = 0; index < count; ++index) {
		if (which_table == 3) {
			draw_mid(index);
		} else {
			draw_back_fore(which_table, index);
		}
	}
}

// seg008:12FE
void __pascal far draw_wipes(int which) {
	word index;
	word count;
	count = wipetable_count;
	for (index = 0; index < count; ++index) {
		if (which == wipetable[index].layer) {
			draw_wipe(index);
		}
	}
}

// seg008:133B
void __pascal far draw_back_fore(int which_table,int index) {
	image_type* image;
	image_type* mask;
	back_table_type* table_entry;
	if (which_table == 0) {
		table_entry = &backtable[index];
	} else if (which_table == 1) {
		table_entry = &foretable[index];
	}
	image = mask = chtab_addrs[table_entry->chtab_id & 0xFF]->images[table_entry->id];
	if ((graphics_mode == gmCga || graphics_mode == gmHgaHerc) &&
		chtab_shift[table_entry->chtab_id] == 0) {
		chtab_type* chtab = chtab_addrs[table_entry->chtab_id];
		mask = chtab->images[chtab->n_images / 2 + table_entry->id];
	}
	draw_image(image, mask, table_entry->xh * 8 + table_entry->xl, table_entry->y, table_entry->blit);
}


SDL_Surface* hflip(SDL_Surface* input) {
	int width = input->w;
	int height = input->h;
	int source_x, target_x;

	// The simplest way to create a surface with same format as input:
	SDL_Surface* output = SDL_ConvertSurface(input, input->format, 0);
	SDL_SetSurfacePalette(output, input->format->palette);
	// The copied image will be overwritten anyway.
	if (output == NULL) {
		sdlperror("SDL_ConvertSurface");
		quit(1);
	}

	SDL_SetSurfaceBlendMode(input, SDL_BLENDMODE_NONE);
	// Temporarily turn off alpha and colorkey on input. So we overwrite the output image.
	SDL_SetColorKey(input, SDL_FALSE, 0);
	SDL_SetColorKey(output, SDL_FALSE, 0);
	SDL_SetSurfaceAlphaMod(input, 255);

	for (source_x = 0, target_x = width-1; source_x < width; ++source_x, --target_x) {
		SDL_Rect srcrect = {source_x, 0, 1, height};
		SDL_Rect dstrect = {target_x, 0, 1, height};
		if (SDL_BlitSurface(input/*32*/, &srcrect, output, &dstrect) != 0) {
			sdlperror("SDL_BlitSurface");
			quit(1);
		}
	}

	return output;
}


// seg008:140C
void __pascal far draw_mid(int index) {
	word need_free_mask;
	word image_id;
	image_type*far mask;
	word chtab_id;
	word blit_flip;
	short ypos;
	short xpos;
	midtable_type* midtable_entry;
	word blit;
	word need_free_image;
	image_type*far image;
//	word image_flipped;
	
	blit_flip = 0;
	need_free_image = 0;
	need_free_mask = 0;
	midtable_entry = &midtable[index];
	image_id = midtable_entry->id;
	chtab_id = midtable_entry->chtab_id;
	image = mask = chtab_addrs[chtab_id & 0xFF]->images[image_id];
	if ((graphics_mode == gmCga || graphics_mode == gmHgaHerc) && chtab_shift[chtab_id]) {
		mask = chtab_addrs[chtab_id]->images[image_id + chtab_addrs[chtab_id]->n_images / 2];
	}
	xpos = midtable_entry->xh * 8 + midtable_entry->xl;
	ypos = midtable_entry->y;
	blit = midtable_entry->blit;
	if (blit & 0x80) {
		blit_flip = 0x8000;
		blit &= 0x7F;
	}

	if (chtab_flip_clip[chtab_id]) {
		set_clip_rect(&midtable_entry->clip);
		if (chtab_id != id_chtab_0_sword) {
			xpos = calc_screen_x_coord(xpos);
		}
	}
	if (blit_flip) {
		xpos -= image->w/*width*/;
		// for this version:
		need_free_image = 1;
		image = hflip(image);
	}

	if (midtable_entry->peel) {
		add_peel(round_xpos_to_byte(xpos, 0), round_xpos_to_byte(image->w/*width*/ + xpos, 1), ypos, image->h/*height*/);
	}
	//printf("Midtable: drawing (chtab %d, image %d) at (x=%d, y=%d)\n",chtab_id,image_id,xpos,ypos); // debug
	draw_image(image, mask, xpos, ypos, blit);

	if (chtab_flip_clip[chtab_id]) {
		reset_clip_rect();
	}
	if (need_free_image) {
		//free_far(image);
		SDL_FreeSurface(image);
	}
	if (need_free_mask) {
		free_far(mask);
	}
}

// seg008:167B
void __pascal far draw_image(image_type far *image,image_type far *mask,int xpos,int ypos,int blit) {
	rect_type rect;
	switch (blit) {
		case blitters_10h_transp:
			draw_image_transp(image, mask, xpos, ypos);
		break;
		case blitters_9_black:
			method_6_blit_img_to_scr(mask, xpos, ypos, blitters_9_black);
		break;
		case blitters_0_no_transp:
		case blitters_2_or:
		case blitters_3_xor:
			method_6_blit_img_to_scr(image, xpos, ypos, blit);
		break;
		default:
			method_3_blit_mono(image, xpos, ypos, 0, blit & 0xBF);
		break;
	}
	if (need_drects) {
		rect.left = rect.right = xpos;
		rect.right += image->w/*width*/;
		rect.top = rect.bottom = ypos;
		rect.bottom += image->h/*height*/;
		add_drect(&rect);
	}
}

// seg008:1730
void __pascal far draw_wipe(int index) {
	rect_type rect;
	wipetable_type* ptr;
	ptr = &wipetable[index];
	rect.left = rect.right = ptr->left;
	rect.right += ptr->width;
	rect.bottom = rect.top = ptr->bottom;
	rect.top -= ptr->height;
	draw_rect(&rect, ptr->color);
	if (need_drects) {
		add_drect(&rect);
	}
}

// data:4E8C
word gate_top_y;
// data:4CB6
word gate_openness;
// data:436C
word gate_bottom_y;

// seg008:178E
void __pascal far calc_gate_pos() {
	gate_top_y = draw_bottom_y - 62;
	gate_openness = (MIN(modifier_left, 188) >> 2) + 1;
	gate_bottom_y = draw_main_y - gate_openness;
}

// data:2785
const byte door_fram_slice[] = {67, 59, 58, 57, 56, 55, 54, 53, 52};
// seg008:17B7
void __pascal far draw_gate_back() {
	short ybottom;
	word var_2;
	calc_gate_pos();
	if (gate_bottom_y + 12 < draw_main_y) {
		add_backtable(id_chtab_6_environment, 50 /*gate bottom with B*/, draw_xh, 0, gate_bottom_y, blitters_0_no_transp, 0);
	} else {
#ifndef FIX_GATE_DRAWING_BUG
		// The following line (erroneously) erases the top-right of the tile below-left (because it is drawn non-transparently).
		// -- But it draws something that was already drawn! (in draw_tile_right()).
		add_backtable(id_chtab_6_environment, tile_table[tiles_4_gate].right_id, draw_xh, 0, tile_table[tiles_4_gate].right_y + draw_main_y, blitters_0_no_transp, 0);
		// And this line tries to fix it. But it fails if it was a gate or a pillar.
		if (can_see_bottomleft()) draw_tile_topright();
		// The following 3 lines draw things that are drawn after this anyway.
		draw_tile_bottom(0);
		draw_loose(0);
		draw_tile_base();
#endif
		add_backtable(id_chtab_6_environment, 51 /*gate bottom*/, draw_xh, 0, gate_bottom_y - 2, blitters_10h_transp, 0);
	}
	ybottom = gate_bottom_y - 12;
	if (ybottom < 192) {
		for (; ybottom >= 0 && ybottom > 7 && ybottom - 7 > gate_top_y; ybottom -= 8) {
			add_backtable(id_chtab_6_environment, 52 /*gate slice 8px*/, draw_xh, 0, ybottom, blitters_0_no_transp, 0);
		}
	}
	var_2 = ybottom - gate_top_y + 1;
	if (var_2 > 0 && var_2 < 9) {
		add_backtable(id_chtab_6_environment, door_fram_slice[var_2], draw_xh, 0, ybottom, blitters_0_no_transp, 0);
	}
}

// seg008:18BE
void __pascal far draw_gate_fore() {
	short ybottom;
	calc_gate_pos();
	add_foretable(id_chtab_6_environment, 51 /*gate bottom*/, draw_xh, 0, gate_bottom_y - 2, blitters_10h_transp, 0);
	ybottom = gate_bottom_y - 12;
	if (ybottom < 192) {
		for (; ybottom >= 0 && ybottom > 7 && ybottom - 7 > gate_top_y; ybottom -= 8) {
			add_foretable(id_chtab_6_environment, 52 /*gate slice 8px*/, draw_xh, 0, ybottom, blitters_10h_transp, 0);
		}
	}
}

// seg008:1937
void __pascal far alter_mods_allrm() {
	word tilepos;
	word room;
	for (room = 1; room <= level.used_rooms; room++) {
		get_room_address(room);
		room_L = level.roomlinks[room-1].left;
		room_R = level.roomlinks[room-1].right;
		for(tilepos = 0; tilepos < 30; tilepos++) {
			load_alter_mod(tilepos);
		}
	}
}

// seg008:198E
void __pascal far load_alter_mod(int tilepos) {
	word wall_to_right;
	word tiletype;
	word wall_to_left;
	byte* curr_tile_modif;
	curr_tile_modif = tilepos + curr_room_modif;
	tiletype = curr_room_tiles[tilepos] & 0x1F;
	switch (tiletype) {
		case tiles_4_gate:
			if (*curr_tile_modif == 1) {
				*curr_tile_modif = 188;
			} else {
				*curr_tile_modif = 0;
			}
			break;
		case tiles_11_loose:
			*curr_tile_modif = 0;
			break;
		case tiles_10_potion:
			*curr_tile_modif <<= 3;
#ifdef USE_COPYPROT
			if (current_level == 15) {
				// Copy protection
				if (copyprot_room[copyprot_plac] == copyprot_plac &&
					copyprot_tile[copyprot_plac] == tilepos
				) {
					*curr_tile_modif = 0xC0; // place open potion
				}
			}
#endif
			break;
		case tiles_20_wall:
			*curr_tile_modif <<= 7;
			if (graphics_mode != gmCga && graphics_mode != gmHgaHerc) {
				wall_to_right = 1;
				wall_to_left = 1;
				if (tilepos % 10 == 0) {
					if (room_L) {
						wall_to_left = (level.fg[30*(room_L-1)+tilepos+9] & 0x1F) == tiles_20_wall;
					}
				} else {
					wall_to_left = (curr_room_tiles[tilepos-1] & 0x1F) == tiles_20_wall;
				}
				if (tilepos % 10 == 9) {
					if (room_R) {
						wall_to_right = (level.fg[30*(room_R-1)+tilepos-9] & 0x1F) == tiles_20_wall;
					}
				} else {
					wall_to_right = (curr_room_tiles[tilepos+1] & 0x1F) == tiles_20_wall;
				}
				if (wall_to_left && wall_to_right) {
					*curr_tile_modif |= 3;
				} else if (wall_to_left) {
					*curr_tile_modif |= 2;
				} else if (wall_to_right) {
					*curr_tile_modif |= 1;
				}
			} else {
				*curr_tile_modif = 3;
			}
			break;
	}
}

// seg008:1AF8
void __pascal far draw_moving() {
	draw_mobs();
	draw_people();
	redraw_needed_tiles();
}

// seg008:1B06
void __pascal far redraw_needed_tiles() {
	word saved_drawn_room;
	load_leftroom();
	draw_objtable_items_at_tile(30);
	for (drawn_row = 3; drawn_row--; ) {
		load_rowbelow();
		draw_bottom_y = 63 * drawn_row + 65;
		draw_main_y = draw_bottom_y - 3;
		for (drawn_col = 0; drawn_col < 10; ++drawn_col) {
			load_curr_and_left_tile();
			redraw_needed(tbl_line[drawn_row] + drawn_col);
		}
	}
	saved_drawn_room = drawn_room;
	drawn_room = room_A;
	load_room_links();
	load_leftroom();
	drawn_row = 2;
	load_rowbelow();
	for (drawn_col = 0; drawn_col < 10; ++drawn_col) {
		load_curr_and_left_tile();
		draw_main_y = -1;
		draw_bottom_y = 2;
		redraw_needed_above(drawn_col);
	}
	drawn_room = saved_drawn_room;
	load_room_links();
	draw_objtable_items_at_tile(-1);
}

// seg008:1BCB
void __pascal far draw_tile_wipe(byte height) {
	add_wipetable(0, draw_xh*8, draw_bottom_y, height, 4*8, 0);
}

// seg008:1BEB
void __pascal far draw_tables() {
	drects_count = 0;
	current_target_surface = offscreen_surface;
	if (is_blind_mode) {
		draw_rect(&rect_top, 0);
	}
	restore_peels();
	draw_wipes(0);
	draw_table(0); // backtable
	//printf("midtable_count = %d\n", midtable_count); // debug
	draw_table(3); // midtable
	draw_wipes(1);
	draw_table(1); // foretable
	current_target_surface = onscreen_surface_;
	show_copyprot(1);
}

// seg008:1C4E
void __pascal far restore_peels() {
	peel_type peel;
	while (peels_count--) {
		peel = peels_table[peels_count];
		if (need_drects) {
			add_drect(&peel.rect); // ?
		}
		restore_peel(peel);
	}
	peels_count = 0;
}

// seg008:1C8F
void __pascal far add_drect(rect_type *source) {
	rect_type* far current_drect;
	short index;
	rect_type target_rect;
	for (index = 0; index < drects_count; ++index) {
		if (intersect_rect(&target_rect, shrink2_rect(&target_rect, source, -1, -1), &drects[index])) {
			current_drect = &drects[index];
			union_rect(current_drect, current_drect, source);
			return;
		}
	}
	if (drects_count >= 30){
		show_dialog("DRects Overflow");
		return /*0*/; // added
	}
	drects[drects_count++] = *source;
}

// seg008:1D29
void __pascal far draw_leveldoor() {
	word var_6;
	word ybottom;
	ybottom = draw_main_y - 13;
	leveldoor_right = (draw_xh<<3)+48;
	if (tbl_level_type[current_level]) leveldoor_right += 8;
	add_backtable(id_chtab_6_environment, 99 /*leveldoor stairs bottom*/, draw_xh + 1, 0, ybottom, blitters_0_no_transp, 0);
	if (modifier_left) {
		if (level.start_room != drawn_room) {
			add_backtable(id_chtab_6_environment, 144 /*level door stairs*/, draw_xh + 1, 0, ybottom - 4, blitters_0_no_transp, 0);
		}
		else {
			short leveldoor_width = (tbl_level_type[current_level] == 0) ? 39 : 48;
			sbyte x_low = (tbl_level_type[current_level] == 0) ? 2 : 0; // dungeon level doors are shifted 2px to the right
			add_wipetable(0, 8*(draw_xh + 1) + x_low, ybottom - 4, 45, leveldoor_width, 0);
		}
	}
	leveldoor_ybottom = ybottom - (modifier_left & 3) - 48;
	for (var_6 = ybottom - modifier_left;
		add_backtable(id_chtab_6_environment, 33 /*level door bottom*/, draw_xh + 1, 0, leveldoor_ybottom, blitters_0_no_transp, 0),
			var_6 > leveldoor_ybottom;
		leveldoor_ybottom += 4) {
		;
	} // runs at least once?
	add_backtable(id_chtab_6_environment, 34 /*level door top*/, draw_xh + 1, 0, draw_main_y - 64, blitters_0_no_transp, 0);
}

// seg008:1E0C
void __pascal far get_room_address(int room) {
	if (room) {
		curr_room_tiles = &level.fg[(room-1)*30];
		curr_room_modif = &level.bg[(room-1)*30];
	}
}

// data:286A
const word floor_left_overlay[] = {32, 151, 151, 150, 150, 151, 32, 32};

// seg008:1E3A
void __pascal far draw_floor_overlay() {
#ifndef FIX_BIGPILLAR_CLIMB
	if (tile_left != tiles_0_empty) return;
#endif
	if (curr_tile == tiles_1_floor ||
		curr_tile == tiles_3_pillar ||
		curr_tile == tiles_5_stuck ||
		curr_tile == tiles_19_torch
	) {
		// frames 137..144: climb
		// index overflow here?
		if (Kid.frame >= frame_137_climbing_3 && Kid.frame <= frame_144_climbing_10) {
			add_midtable(id_chtab_6_environment, floor_left_overlay[Kid.frame - 137], draw_xh, 0, (curr_tile == tiles_5_stuck) + draw_main_y, blitters_10h_transp, 0);
		} else {
			// triggered by 02-random-broken
			printf("draw_floor_overlay: attempted to draw floor overlay with frame %d not in 137..144\n", Kid.frame);
			//quit(1);
		}
		ptr_add_table = &add_midtable;
		draw_tile_bottom(0);
		ptr_add_table = &add_backtable;
	} else {
		draw_other_overlay();
	}
}

// seg008:1EB5
void __pascal far draw_other_overlay() {
	byte tiletype;
	byte modifier;
	if (tile_left == tiles_0_empty) {
		ptr_add_table = &add_midtable;
		draw_tile2();
	} else if (curr_tile != tiles_0_empty && drawn_col > 0 &&
		get_tile_to_draw(drawn_room, drawn_col - 2, drawn_row, &tiletype, &modifier, tiles_0_empty) == tiles_0_empty
	) {
		ptr_add_table = &add_midtable;
		draw_tile2();
		ptr_add_table = &add_backtable;
		draw_tile2();
		tile_object_redraw[tbl_line[drawn_row] + drawn_col] = 0xFF;
	}
	ptr_add_table = &add_backtable;
}

// seg008:1F48
void __pascal far draw_tile2() {
	draw_tile_right();
	draw_tile_anim_right();
	draw_tile_base();
	draw_tile_anim();
	draw_tile_bottom(0);
	draw_loose(0);
}

// seg008:1F67
void __pascal far draw_objtable_items_at_tile(byte tilepos) {
	//printf("draw_objtable_items_at_tile(%d)\n",tilepos); // debug
	short obj_count;
	short obj_index;
	obj_count = objtable_count;
	if (obj_count) {
		for (obj_index = obj_count - 1, n_curr_objs = 0; obj_index >= 0; --obj_index) {
			if (objtable[obj_index].tilepos == tilepos) {
				curr_objs[n_curr_objs++] = obj_index;
			}
		}
		if (n_curr_objs) {
			sort_curr_objs();
			for (obj_index = 0; obj_index < n_curr_objs; ++obj_index) {
				draw_objtable_item(curr_objs[obj_index]);
			}
		}
	}
}

// seg008:1FDE
void __pascal far sort_curr_objs() {
	short swapped;
	short temp;
	short last;
	short index;
	// bubble sort
	last = n_curr_objs - 1;
	do {
		for (swapped = index = 0; index < last; ++index) {
			if (compare_curr_objs(index, index + 1)) {
				temp = curr_objs[index];
				curr_objs[index] = curr_objs[index + 1];
				curr_objs[index + 1] = temp;
				swapped = 1;
			}
		}
		// --last ?
	} while (swapped);
}

// seg008:203C
int __pascal far compare_curr_objs(int index1,int index2) {
	short obj_index1;
	short obj_index2;
	obj_index1 = curr_objs[index1];
	if (objtable[obj_index1].obj_type == 1) return 1;
	obj_index2 = curr_objs[index2];
	if (objtable[obj_index2].obj_type == 1) return 0;
	if (objtable[obj_index1].obj_type == 0x80 &&
		objtable[obj_index2].obj_type == 0x80
	) {
		return (objtable[obj_index1].y < objtable[obj_index2].y);
	} else {
		return (objtable[obj_index1].y > objtable[obj_index2].y);
	}
	return 1;
}

// seg008:20CA
void __pascal far draw_objtable_item(int index) {
	switch (load_obj_from_objtable(index)) {
		case 0: // Kid
		case 4: // mirror image
			//printf("index = %d, obj_id = %d\n", index, obj_id); // debug
			if (obj_id == 0xFF) return;
			// the Kid blinks a bit after uniting with shadow
			if (united_with_shadow && (united_with_shadow % 2) == 0) goto shadow;
		case 2: // Guard
		case 3: // sword
		case 5: // hurt splash
			add_midtable(obj_chtab, obj_id + 1, obj_xh, obj_xl, obj_y, blitters_10h_transp, 1);
		break;
		case 1: // shadow
		shadow:
			if (united_with_shadow == 2) {
				play_sound(sound_41_end_level_music); // united with shadow
			}
			add_midtable(obj_chtab, obj_id + 1, obj_xh, obj_xl, obj_y, blitters_2_or, 1);
			add_midtable(obj_chtab, obj_id + 1, obj_xh, obj_xl + 1, obj_y, blitters_3_xor, 1);
		break;
		case 0x80: // loose floor
			obj_direction = dir_FF_left;
			add_midtable(obj_chtab, loose_fram_left[obj_id], obj_xh, obj_xl, obj_y - 3, blitters_10h_transp, 1);
			add_midtable(obj_chtab, loose_fram_bottom[obj_id], obj_xh, obj_xl, obj_y, 0, 1);
			add_midtable(obj_chtab, loose_fram_right[obj_id], obj_x + 4, obj_xl, obj_y - 1, blitters_10h_transp, 1);
		break;
	}
}

// seg008:2228
int __pascal far load_obj_from_objtable(int index) {
	objtable_type* curr_obj;
	curr_obj = &objtable[index];
	obj_xh = obj_x = curr_obj->xh;
	obj_xl = curr_obj->xl;
	obj_y = curr_obj->y;
	obj_id = curr_obj->id;
	obj_chtab = curr_obj->chtab_id;
	obj_direction = curr_obj->direction;
	obj_clip_top = curr_obj->clip.top;
	obj_clip_bottom = curr_obj->clip.bottom;
	obj_clip_left = curr_obj->clip.left;
	obj_clip_right = curr_obj->clip.right;
	return curr_obj->obj_type;
}

// seg008:228A
void __pascal far draw_people() {
	check_mirror();
	draw_kid();
	draw_guard();
	reset_obj_clip();
	draw_hp();
}

// seg008:22A2
void __pascal far draw_kid() {
	if (Kid.room != 0 && Kid.room == drawn_room) {
		add_kid_to_objtable();
		if (hitp_delta < 0) {
			draw_hurt_splash();
		}
		add_sword_to_objtable();
	}
}

// seg008:22C9
void __pascal far draw_guard() {
	if (Guard.direction != dir_56_none && Guard.room == drawn_room) {
		add_guard_to_objtable();
		if (guardhp_delta < 0) {
			draw_hurt_splash();
		}
		add_sword_to_objtable();
	}
}

// seg008:22F0
void __pascal far add_kid_to_objtable() {
	//printf("add_kid_to_objtable\n");
	loadkid();
	load_fram_det_col();
	load_frame_to_obj();
	stuck_lower();
	set_char_collision();
	set_objtile_at_char();
	redraw_at_char();
	redraw_at_char2();
	clip_char();
	add_objtable(0); // Kid
}

// seg008:2324
void __pascal far add_guard_to_objtable() {
	word obj_type;
	loadshad();
	load_fram_det_col();
	load_frame_to_obj();
	stuck_lower();
	set_char_collision();
	set_objtile_at_char();
	redraw_at_char();
	redraw_at_char2();
	clip_char();
	if (Char.charid == charid_1_shadow) {
		// Special event: shadow is clipped: may appear only right from the mirror
		if (current_level == 4 && Char.room == 4) {
			obj_clip_left = 137;
		}
		obj_type = 1; // shadow
	} else {
		obj_type = 2; // Guard
	}
	add_objtable(obj_type);
}

// seg008:2388
void __pascal far add_objtable(byte obj_type) {
	word index;
	objtable_type* entry_addr;
	//printf("in add_objtable: objtable_count = %d\n",objtable_count); // debug
	index = objtable_count++;
	//printf("in add_objtable: objtable_count = %d\n",objtable_count); // debug
	if (index >= 50) {
		show_dialog("ObjTable Overflow");
		return /*0*/; // added
	}
	entry_addr = &objtable[index];
	entry_addr->obj_type = obj_type;
	x_to_xh_and_xl(obj_x, &entry_addr->xh, &entry_addr->xl);
	entry_addr->y = obj_y;
	entry_addr->clip.top = obj_clip_top;
	entry_addr->clip.bottom = obj_clip_bottom;
	entry_addr->clip.left = obj_clip_left;
	entry_addr->clip.right = obj_clip_right;
	entry_addr->chtab_id = obj_chtab;
	entry_addr->id = obj_id;
	entry_addr->direction = obj_direction;
	mark_obj_tile_redraw(index);
}

// seg008:2423
void __pascal far mark_obj_tile_redraw(int index) {
	//printf("mark_obj_tile_redraw: obj_tile = %d\n", obj_tile); // debug
	objtable[index].tilepos = obj_tilepos;
	if (obj_tilepos < 30) {
		tile_object_redraw[obj_tilepos] = 1;
	}
}

// seg008:2448
void __pascal far load_frame_to_obj() {
	word chtab_base;
	chtab_base = id_chtab_2_kid;
	reset_obj_clip();
	load_frame();
	obj_direction = Char.direction;
	obj_id = cur_frame.image;
	// top 6 bits of sword are the chtab
	obj_chtab = chtab_base + (cur_frame.sword >> 6);
	obj_x = (char_dx_forward(cur_frame.dx) << 1) - 116;
	obj_y = cur_frame.dy + Char.y;
	if ((sbyte)(cur_frame.flags ^ obj_direction) >= 0) {
		// 0x80: even/odd pixel
		++obj_x;
	}
}

// seg008:24A8
void __pascal far show_time() {
	char sprintf_temp[40];
	word rem_sec;
	if (Kid.alive < 0 &&
		rem_min != 0 &&
		(current_level < 13 || (current_level == 13 && leveldoor_open == 0)) &&
		current_level < 15
	) {
		// Time passes
		--rem_tick;
		if (rem_tick == 0) {
			rem_tick = 719; // 720=12*60 ticks = 1 minute
			--rem_min;
			if (rem_min != 0 && (rem_min <= 5 || rem_min % 5 == 0)) {
				is_show_time = 1;
			}
		} else {
			if (rem_min == 1 && rem_tick % 12 == 0) {
				is_show_time = 1;
				text_time_remaining = 0;
			}
		}
	}
	if (is_show_time && text_time_remaining == 0) {
		text_time_remaining = text_time_total = 24;
		if (rem_min > 0) {
			if (rem_min == 1) {
				rem_sec = (rem_tick + 1) / 12;
				if (rem_sec == 1) {
					strncpy(sprintf_temp, "1 SECOND LEFT", sizeof(sprintf_temp));
					text_time_remaining = text_time_total = 12;
				} else {
					snprintf(sprintf_temp, sizeof(sprintf_temp), "%d SECONDS LEFT", rem_sec);
				}
			} else {
				snprintf(sprintf_temp, sizeof(sprintf_temp), "%d MINUTES LEFT", rem_min);
			}
			display_text_bottom(sprintf_temp);
		} else {
			display_text_bottom("TIME HAS EXPIRED!");
		}
		is_show_time = 0;
	}
}

// seg008:25A8
void __pascal far show_level() {
	byte disp_level;
	char sprintf_temp[32];
	disp_level = current_level;
	if (disp_level != 0 && disp_level < 14 && seamless == 0) {
		if (disp_level == 13) {
			disp_level = 12;
		}
		text_time_remaining = text_time_total = 24;
		snprintf(sprintf_temp, sizeof(sprintf_temp), "LEVEL %d", disp_level);
		display_text_bottom(sprintf_temp);
		is_show_time = 1;
	}
	seamless = 0;
}

// seg008:2602
short __pascal far calc_screen_x_coord(short logical_x) {
	return logical_x*320/280;
}

// seg008:2627
void __pascal far free_peels() {
	while (peels_count > 0) {
		--peels_count;
		free_peel(&peels_table[peels_count]);
	}
}

// data:0F96
const rect_type rect_bottom_text = {193, 70, 202, 250};

// seg008:2644
void __pascal far display_text_bottom(const char near *text) {
	draw_rect(&rect_bottom_text, 0);
	show_text(&rect_bottom_text, 0, 1, text);
#ifndef USE_TEXT
	SDL_WM_SetCaption(text, NULL);
#endif
}

// seg008:266D
void __pascal far erase_bottom_text(int arg_0) {
	draw_rect(&rect_bottom_text, 0);
	if (arg_0) {
		text_time_total = 0;
		text_time_remaining = 0;
	}
#ifndef USE_TEXT
	SDL_WM_SetCaption("", NULL);
#endif
}

// Dungeon wall drawing algorithm by HTamas

#define RSET_WALL              7

#define RES_WALL_FACE_MAIN     1    //(face stack main.bmp)
#define RES_WALL_FACE_TOP      2    //(face stack top.bmp)
#define RES_WALL_CENTRE_BASE   3    //(centre stack base.bmp)
#define RES_WALL_CENTRE_MAIN   4    //(centre stack main.bmp)
#define RES_WALL_RIGHT_BASE    5    //(right stack base.bmp)
#define RES_WALL_RIGHT_MAIN    6    //(right stack main.bmp)
#define RES_WALL_SINGLE_BASE   7    //(single stack base.bmp)
#define RES_WALL_SINGLE_MAIN   8    //(single stack main.bmp)
#define RES_WALL_LEFT_BASE     9    //(left stack base.bmp)
#define RES_WALL_LEFT_MAIN    10    //(left stack main.bmp)
#define RES_WALL_DIVIDER1     11    //(divider01.bmp, the broad divider)
#define RES_WALL_DIVIDER2     12    //(divider02.bmp, the narrow divider)
#define RES_WALL_RNDBLOCK     13    //(random block.bmp)
#define RES_WALL_MARK_TL      14    //(mark01.bmp, top left mark)
#define RES_WALL_MARK_BL      15    //(mark02.bmp, bottom left mark)
#define RES_WALL_MARK_TR      16    //(mark03.bmp, top right mark)
#define RES_WALL_MARK_BR      17    //(mark04.bmp, bottom right mark)

#define BLIT_NO_TRANS          0
#define BLIT_OR                2
#define BLIT_XOR               3
#define BLIT_BLACK             9
#define BLIT_TRANS            16

#define GRAPHICS_CGA          1
#define GRAPHICS_HERCULES     2
#define GRAPHICS_EGA          3
#define GRAPHICS_TANDY        4
#define GRAPHICS_VGA          5

#define DESIGN_DUNGEON        0
#define DESIGN_PALACE         1

#define WALL_MODIFIER_SWS     0
#define WALL_MODIFIER_SWW     1
#define WALL_MODIFIER_WWS     2
#define WALL_MODIFIER_WWW     3


// seg008:268F
void __pascal far wall_pattern(int which_part,int which_table) {
	// local variables
	add_table_type saved_sim;
	word v2; word v3; byte v4; byte v5;
	byte bg_modifier;
	dword saved_prng_state;
	word is_dungeon;
	// save the value for the sprite insertion method, so that it can be restored
	saved_sim = ptr_add_table;
	// set the sprite insertion method based on the arguments
	if (which_table == 0) {
		ptr_add_table = &add_backtable;
	} else {
		ptr_add_table = &add_foretable;
	}
	// save the state of the pseudorandom number generator
	saved_prng_state = random_seed;
	// set the new seed
	random_seed = drawn_room + tbl_line[drawn_row] + drawn_col;
	prandom(1); // fetch a random number and discard it
	is_dungeon = (tbl_level_type[current_level] < DESIGN_PALACE);
	if ( (!is_dungeon) && (graphics_mode== GRAPHICS_VGA) ) {
		// I haven't traced the palace WDA
		//[...]
		if (which_part) {
			add_wipetable(which_table, 8*(draw_xh)    , draw_main_y - 40, 20, 4*8, palace_wall_colors[44 * drawn_row +      drawn_col]);
			add_wipetable(which_table, 8*(draw_xh)    , draw_main_y - 19, 21, 2*8, palace_wall_colors[44 * drawn_row + 11 + drawn_col]);
			add_wipetable(which_table, 8*(draw_xh + 2), draw_main_y - 19, 21, 2*8, palace_wall_colors[44 * drawn_row + 12 + drawn_col]);
			add_wipetable(which_table, 8*(draw_xh)    , draw_main_y     , 19, 1*8, palace_wall_colors[44 * drawn_row + 22 + drawn_col]);
			add_wipetable(which_table, 8*(draw_xh + 1), draw_main_y     , 19, 3*8, palace_wall_colors[44 * drawn_row + 23 + drawn_col]);
			ptr_add_table(id_chtab_7_environmentwall, prandom(2) +  3, draw_xh + 3, 0, draw_main_y - 53, blitters_46h_mono_6, 0);
			ptr_add_table(id_chtab_7_environmentwall, prandom(2) +  6, draw_xh    , 0, draw_main_y - 34, blitters_46h_mono_6, 0);
			ptr_add_table(id_chtab_7_environmentwall, prandom(2) +  9, draw_xh    , 0, draw_main_y - 13, blitters_46h_mono_6, 0);
			ptr_add_table(id_chtab_7_environmentwall, prandom(2) + 12, draw_xh    , 0, draw_main_y     , blitters_46h_mono_6, 0);
		}
		add_wipetable(which_table, 8*draw_xh    , draw_bottom_y   ,  3, 4*8, palace_wall_colors[44 * drawn_row + 33 + drawn_col]);
		ptr_add_table(id_chtab_7_environmentwall, prandom(2) + 15, draw_xh    , 0, draw_bottom_y   , blitters_46h_mono_6, 0);
	} else {
		v3 = prandom(1);
		v5 = prandom(4);
		v2 = prandom(1);
		v4 = prandom(4);
		// store the background modifier for the current tile in a local variable
		// apparently, for walls, the modifier stores whether there are adjacent walls
		bg_modifier = curr_modifier & 0x7F;
		switch (bg_modifier) {
			case WALL_MODIFIER_WWW:
				if (which_part != 0) {
					if (prandom(4) == 0) {
						ptr_add_table(RSET_WALL, RES_WALL_RNDBLOCK, draw_xh, 0, draw_bottom_y - 42, BLIT_NO_TRANS, 0);
					}
					ptr_add_table(RSET_WALL, RES_WALL_DIVIDER1 + v3, draw_xh + 1, v5, draw_bottom_y - 21, BLIT_TRANS, 0);
				}
				ptr_add_table(RSET_WALL, RES_WALL_DIVIDER1 + v2, draw_xh, v4, draw_bottom_y, BLIT_TRANS, 0);
				if (which_part != 0) {
					if (is_dungeon) {
						if (prandom(4) == 0) {
							draw_right_mark(prandom(3), v5);
						}
						if (prandom(4) == 0) {
							draw_left_mark(prandom(4), v5 - v3, v4 - v2);
						}
					}
				}
				break;
			case WALL_MODIFIER_SWS:
				if (is_dungeon) {
					if (which_part != 0) {
						if (prandom(6) == 0) {
							draw_left_mark(prandom(1), v5 - v3, v4 - v2);
						}
					}
				}
				break;
			case WALL_MODIFIER_SWW:
				if (which_part != 0) {
					if (prandom(4) == 0) {
						ptr_add_table(RSET_WALL, RES_WALL_RNDBLOCK, draw_xh, 0, draw_bottom_y - 42, BLIT_NO_TRANS, 0);
					}
					ptr_add_table(RSET_WALL, RES_WALL_DIVIDER1 + v3, draw_xh + 1 /*fix*/ , v5, draw_bottom_y - 21, BLIT_TRANS, 0);
					if (is_dungeon) {
						if (prandom(4) == 0) {
							draw_right_mark(prandom(3), v5);
						}
						if (prandom(4) == 0) {
							draw_left_mark(prandom(3), v5 - v3, v4 - v2);
						}
					}
				}
				break;
			case WALL_MODIFIER_WWS:
				if (which_part != 0) {
					ptr_add_table(RSET_WALL, RES_WALL_DIVIDER1 + v3, draw_xh + 1, v5, draw_bottom_y - 21, BLIT_TRANS, 0);
				}
				ptr_add_table(RSET_WALL, RES_WALL_DIVIDER1 + v2, draw_xh, v4, draw_bottom_y, BLIT_TRANS, 0);
				if (which_part != 0) {
					if (is_dungeon) {
						if (prandom(4) == 0) {
							draw_right_mark(prandom(1) + 2, v5);
						}
						if (prandom(4) == 0) {
							draw_left_mark(prandom(4), v5 - v3, v4 - v2);
						}
					}
				}
				break;
		}
	}
	random_seed = saved_prng_state;
	ptr_add_table = saved_sim;
}

void __pascal far draw_left_mark (word arg3, word arg2, word arg1) {
	word lv1; word lv2;
	static const word LPOS[] = {58, 41, 37, 20, 16};
	lv1 = RES_WALL_MARK_TL;
	lv2 = 0;
	if (arg3 % 2) {
		lv1 = RES_WALL_MARK_BL;
	}
	if (arg3 > 3) {
		lv2 = arg1 + 6;
	} else if (arg3 > 1) {
		lv2 = arg2 + 6;
	}
	ptr_add_table(RSET_WALL, lv1, draw_xh + (arg3 == 2 || arg3 == 3), lv2, draw_bottom_y - LPOS[arg3], BLIT_TRANS, 0);
}

void __pascal far draw_right_mark (word arg2, word arg1) {
	word rv;
	static const word RPOS[] = {52, 42, 31, 21};
	rv = RES_WALL_MARK_TR;
	if (arg2 % 2) {
		rv = RES_WALL_MARK_BR;
	}
	if (arg2 < 2) {
		arg1 = 24;
	} else {
		arg1 -= 3;
	}
	ptr_add_table(RSET_WALL, rv, draw_xh + (arg2 > 1), arg1, draw_bottom_y - RPOS[arg2], BLIT_TRANS, 0);
}
