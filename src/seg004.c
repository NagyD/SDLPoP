/*
SDLPoP, a port/conversion of the DOS game Prince of Persia.
Copyright (C) 2013-2017  Dávid Nagy

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

// data:432F
sbyte bump_col_left_of_wall;
// data:436E
sbyte bump_col_right_of_wall;
// data:4C0A
sbyte right_checked_col;
// data:408A
sbyte left_checked_col;


// data:4C0C
short coll_tile_left_xpos;
// These two arrays are indexed with the return value of wall_type.
// data:24BA
const sbyte wall_dist_from_left[] = {0, 10, 0, -1, 0, 0};
// data:24C0
const sbyte wall_dist_from_right[] = {0, 0, 10, 13, 0, 0};

// seg004:0004
void __pascal far check_collisions() {
	short column;
	bump_col_left_of_wall = bump_col_right_of_wall = -1;
	if (Char.action == actions_7_turn) return;
	collision_row = Char.curr_row;
	move_coll_to_prev();
	prev_collision_row = collision_row;
	right_checked_col = MIN(get_tile_div_mod_m7(char_x_right_coll) + 2, 11);
	left_checked_col = get_tile_div_mod_m7(char_x_left_coll) - 1;
	get_row_collision_data(collision_row    , curr_row_coll_room, curr_row_coll_flags);
	get_row_collision_data(collision_row + 1, below_row_coll_room, below_row_coll_flags);
	get_row_collision_data(collision_row - 1, above_row_coll_room, above_row_coll_flags);
	for (column = 9; column >= 0; --column) {
		if (curr_row_coll_room[column] >= 0 &&
			prev_coll_room[column] == curr_row_coll_room[column]
		) {
			// char bumps into left of wall
			if (
				(prev_coll_flags[column] & 0x0F) == 0 &&
				(curr_row_coll_flags[column] & 0x0F) != 0
			) {
				bump_col_left_of_wall = column;
			}
			// char bumps into right of wall
			if (
				(prev_coll_flags[column] & 0xF0) == 0 &&
				(curr_row_coll_flags[column] & 0xF0) != 0
			) {
				bump_col_right_of_wall = column;
			}
		}
	}
}

// seg004:00DF
void __pascal far move_coll_to_prev() {
	sbyte* row_coll_room_ptr;
	byte* row_coll_flags_ptr;
	short column;
	if (collision_row     == prev_collision_row ||
		collision_row + 3 == prev_collision_row ||
		collision_row - 3 == prev_collision_row
	) {
		row_coll_room_ptr = curr_row_coll_room;
		row_coll_flags_ptr = curr_row_coll_flags;
	} else if (
		collision_row + 1 == prev_collision_row ||
		collision_row - 2 == prev_collision_row
	) {
		row_coll_room_ptr = above_row_coll_room;
		row_coll_flags_ptr = above_row_coll_flags;
	} else {
		row_coll_room_ptr = below_row_coll_room;
		row_coll_flags_ptr = below_row_coll_flags;
	}
	for (column = 0; column < 10; ++column) {
		prev_coll_room[column] = row_coll_room_ptr[column];
		prev_coll_flags[column] = row_coll_flags_ptr[column];
		below_row_coll_room[column] = -1;
		above_row_coll_room[column] = -1;
		curr_row_coll_room[column] = -1;
		// bugfix:
		curr_row_coll_flags[column] = 0;
		below_row_coll_flags[column] = 0;
		above_row_coll_flags[column] = 0;
	}
}

// seg004:0185
void __pascal far get_row_collision_data(short row, sbyte *row_coll_room_ptr, byte *row_coll_flags_ptr) {
	short right_wall_xpos;
	byte curr_flags;
	short room;
	short column;
	short left_wall_xpos;
	room = Char.room;
	coll_tile_left_xpos = x_bump[left_checked_col + 5] + 7;
	for (column = left_checked_col; column <= right_checked_col; ++column) {
		left_wall_xpos = get_left_wall_xpos(room, column, row);
		right_wall_xpos = get_right_wall_xpos(room, column, row);
		// char bumps into left of wall
		curr_flags = (left_wall_xpos < char_x_right_coll) * 0x0F;
		// char bumps into right of wall
		curr_flags |= (right_wall_xpos > char_x_left_coll) * 0xF0;
		row_coll_flags_ptr[tile_col] = curr_flags;
		row_coll_room_ptr[tile_col] = curr_room;
		coll_tile_left_xpos += 14;
	}
}

// seg004:0226
int __pascal far get_left_wall_xpos(int room,int column,int row) {
	short type;
	type = wall_type(get_tile(room, column, row));
	if (type) {
		return wall_dist_from_left[type] + coll_tile_left_xpos;
	} else {
		return 0xFF;
	}
}

// seg004:025F
int __pascal far get_right_wall_xpos(int room,int column,int row) {
	short type;
	type = wall_type(get_tile(room, column, row));
	if (type) {
		return coll_tile_left_xpos - wall_dist_from_right[type] + 13;
	} else {
		return 0;
	}
}

// seg004:029D
void __pascal far check_bumped() {
	if (
		Char.action != actions_2_hang_climb &&
		Char.action != actions_6_hang_straight &&
		// frames 135..149: climb up
		(Char.frame < frame_135_climbing_1 || Char.frame >= 149)
	) {
#ifdef FIX_TWO_COLL_BUG
		if (bump_col_left_of_wall >= 0) {
			check_bumped_look_right();
			if (!fix_two_coll_bug) return; // check for the left-oriented collision only with the fix enabled
		}
		if (bump_col_right_of_wall >= 0) {
			check_bumped_look_left();
		}
#else
		if (bump_col_left_of_wall >= 0) {
			check_bumped_look_right();
		}
		else
		if (bump_col_right_of_wall >= 0) {
			check_bumped_look_left();
		}
#endif // FIX_TWO_COLL_BUG

	}
}

// seg004:02D2
void __pascal far check_bumped_look_left() {
	if ((Char.sword == sword_2_drawn || Char.direction < dir_0_right) && // looking left
		is_obstacle_at_col(bump_col_right_of_wall)
	) {
		bumped(get_right_wall_xpos(curr_room, tile_col, tile_row) - char_x_left_coll, dir_0_right);
	}
}

// seg004:030A
void __pascal far check_bumped_look_right() {
	if ((Char.sword == sword_2_drawn || Char.direction == dir_0_right) && // looking right
		is_obstacle_at_col(bump_col_left_of_wall)
	) {
		bumped(get_left_wall_xpos(curr_room, tile_col, tile_row) - char_x_right_coll, dir_FF_left);
	}
}

// seg004:0343
int __pascal far is_obstacle_at_col(int tile_col) {
	short tile_row;
	tile_row = Char.curr_row;
	if (tile_row < 0) {
		tile_row += 3;
	}
	if (tile_row >= 3) {
		tile_row -= 3;
	}
	get_tile(curr_row_coll_room[tile_col], tile_col, tile_row);
	return is_obstacle();
}

// seg004:037E
int __pascal far is_obstacle() {
	if (curr_tile2 == tiles_10_potion) {
		return 0;
	} else if (curr_tile2 == tiles_4_gate) {
		if (! can_bump_into_gate()) return 0;
	} else if (curr_tile2 == tiles_18_chomper) {
		// is the chomper closed?
		if (curr_room_modif[curr_tilepos] != 2) return 0;
	} else if (
		curr_tile2 == tiles_13_mirror &&
		Char.charid == charid_0_kid &&
		Char.frame >= frame_39_start_run_jump_6 && Char.frame < frame_44_running_jump_5 && // run-jump
		Char.direction < dir_0_right // right-to-left only
	) {
		curr_room_modif[curr_tilepos] = 0x56; // broken mirror or what?
		jumped_through_mirror = -1;
		return 0;
	}
	coll_tile_left_xpos = xpos_in_drawn_room(x_bump[tile_col + 5]) + 7;
	return 1;
}

// seg004:0405
int __pascal far xpos_in_drawn_room(int xpos) {
	if (curr_room != drawn_room) {
		if (curr_room == room_L || curr_room == room_BL) {
			xpos -= 140;
		} else if (curr_room == room_R || curr_room == room_BR) {
			xpos += 140;
		}
	}
	return xpos;
}

// seg004:0448
void __pascal far bumped(sbyte delta_x,sbyte push_direction) {
	// frame 177: spiked
	if (Char.alive < 0 && Char.frame != frame_177_spiked) {
		Char.x += delta_x;
		if (push_direction < dir_0_right) {
			// pushing left
			if (curr_tile2 == tiles_20_wall) {
				get_tile(curr_room, --tile_col, tile_row);
			}
		} else {
			// pushing right
			if (curr_tile2 == tiles_12_doortop ||
				curr_tile2 == tiles_7_doortop_with_floor ||
				curr_tile2 == tiles_20_wall
			) {
				++tile_col;
				if (curr_room == 0 && tile_col == 10) {
					curr_room = Char.room;
					tile_col = 0;
				}
				get_tile(curr_room, tile_col, tile_row);
			}
		}
		if (tile_is_floor(curr_tile2)) {
			bumped_floor(push_direction);
		} else {
			bumped_fall();
		}
	}
}

// seg004:04E4
void __pascal far bumped_fall() {
	short action;
	action = Char.action;
	Char.x = char_dx_forward(-4);
	if (action == actions_4_in_freefall) {
		Char.fall_x = 0;
	} else {
		seqtbl_offset_char(seq_45_bumpfall); // fall after bumped
		play_seq();
	}
	bumped_sound();
}

// seg004:0520
void __pascal far bumped_floor(sbyte push_direction) {
	short frame;
	short seq_index;
	if (Char.sword != sword_2_drawn && (word)(y_land[Char.curr_row + 1] - Char.y) >= (word)15) {
		bumped_fall();
	} else {
		Char.y = y_land[Char.curr_row + 1];
		if (Char.fall_y >= 22) {
			Char.x = char_dx_forward(-5);
		} else {
			Char.fall_y = 0;
			if (Char.alive) {
				if (Char.sword == sword_2_drawn) {
					if (push_direction == Char.direction) {
						seqtbl_offset_char(seq_65_bump_forward_with_sword); // pushed forward with sword (Kid)
						play_seq();
						Char.x = char_dx_forward(1);
						return;
					} else {
						seq_index = seq_64_pushed_back_with_sword; // pushed back with sword
					}
				} else {
					frame = Char.frame;
					if (frame == 24 || frame == 25 ||
						(frame >= 40 && frame < 43) ||
						(frame >= frame_102_start_fall_1 && frame < 107)
					) {
						seq_index = seq_46_hardbump; // bump into wall after run-jump (crouch)
					} else {
						seq_index = seq_47_bump; // bump into wall
					}
				}
				seqtbl_offset_char(seq_index);
				play_seq();
				bumped_sound();
			}
		}
	}
}

// seg004:05F1
void __pascal far bumped_sound() {
	is_guard_notice = 1;
	play_sound(sound_8_bumped); // touching a wall
}

// seg004:0601
void __pascal far clear_coll_rooms() {
	memset_near(prev_coll_room, -1, sizeof(prev_coll_room));
	memset_near(curr_row_coll_room, -1, sizeof(curr_row_coll_room));
	memset_near(below_row_coll_room, -1, sizeof(below_row_coll_room));
	memset_near(above_row_coll_room, -1, sizeof(above_row_coll_room));
	// workaround
	memset_near(prev_coll_flags, 0, sizeof(prev_coll_flags));
	memset_near(curr_row_coll_flags, 0, sizeof(curr_row_coll_flags));
	memset_near(below_row_coll_flags, 0, sizeof(below_row_coll_flags));
	memset_near(above_row_coll_flags, 0, sizeof(above_row_coll_flags));
	prev_collision_row = -1;
}

// seg004:0657
int __pascal far can_bump_into_gate() {
	return (curr_room_modif[curr_tilepos] >> 2) + 6 < char_height;
}

// seg004:067C
int __pascal far get_edge_distance() {
/*
Possible results in edge_type:
0: closer/sword/potion
1: edge
2: floor (nothing near char)
*/
	short distance;
	byte tiletype;
	determine_col();
	load_frame_to_obj();
	set_char_collision();
	tiletype = get_tile_at_char();
	if (wall_type(tiletype) != 0) {
		tile_col = Char.curr_col;
		distance = dist_from_wall_forward(tiletype);
		if (distance >= 0) {
			loc_59DD:
			if (distance < 14) {
				edge_type = 1;
			} else {
				edge_type = 2;
				distance = 11;
			}
		} else {
			goto loc_59E8;
		}
	} else {
		loc_59E8:
		tiletype = get_tile_infrontof_char();
		if (tiletype == tiles_12_doortop && Char.direction >= dir_0_right) {
			loc_59FB:
			edge_type = 0;
			distance = distance_to_edge_weight();
		} else {
			if (wall_type(tiletype) != 0) {
				tile_col = infrontx;
				distance = dist_from_wall_forward(tiletype);
				if (distance >= 0) goto loc_59DD;
			}
			if (tiletype == tiles_11_loose) goto loc_59FB;
			if (
				tiletype == tiles_6_closer ||
				tiletype == tiles_22_sword ||
				tiletype == tiles_10_potion
			) {
				distance = distance_to_edge_weight();
				if (distance != 0) {
					edge_type = 0;
				} else {
					edge_type = 2;
					distance = 11;
				}
			} else {
				if (tile_is_floor(tiletype)) {
					edge_type = 2;
					distance = 11;
				} else {
					goto loc_59FB;
				}
			}
		}
	}
	curr_tile2 = tiletype;
	return distance;
}

// seg004:076B
void __pascal far check_chomped_kid() {
	short tile_col;
	short tile_row;
	tile_row = Char.curr_row;
	for (tile_col = 0; tile_col < 10; ++tile_col) {
		if (curr_row_coll_flags[tile_col] == 0xFF &&
			get_tile(curr_row_coll_room[tile_col], tile_col, tile_row) == tiles_18_chomper &&
			(curr_room_modif[curr_tilepos] & 0x7F) == 2 // closed chomper
		) {
			chomped();
		}
	}
}

// seg004:07BF
void __pascal far chomped() {
	#ifdef FIX_SKELETON_CHOMPER_BLOOD
	if (!(fix_skeleton_chomper_blood && Char.charid == charid_4_skeleton))
	#endif
		curr_room_modif[curr_tilepos] |= 0x80; // put blood
	if (Char.frame != frame_178_chomped && Char.room == curr_room) {
		Char.x = x_bump[tile_col + 5] + 7;
		Char.x = char_dx_forward(7 - !Char.direction);
		Char.y = y_land[Char.curr_row + 1];
		take_hp(100);
		play_sound(sound_46_chomped); // something chomped
		seqtbl_offset_char(seq_54_chomped); // chomped
		play_seq();
	}
}

// seg004:0833
void __pascal far check_gate_push() {
	// Closing gate pushes Kid
	short frame;
	short var_4;
	frame = Char.frame;
	if (Char.action == actions_7_turn ||
		frame == frame_15_stand || // stand
		(frame >= frame_108_fall_land_2 && frame < 111) // crouch
	) {
		get_tile_at_char();
		var_4 = tile_col;
		if ((curr_tile2 == tiles_4_gate ||
			get_tile(curr_room, --tile_col, tile_row) == tiles_4_gate) &&
			(curr_row_coll_flags[tile_col] & prev_coll_flags[tile_col]) == 0xFF &&
			can_bump_into_gate()
		) {
			bumped_sound();
			// push Kid left if var_4 <= tile_col, gate at char's tile
			// push Kid right if var_4 > tile_col, gate is left from char's tile
			Char.x += 5 - (var_4 <= tile_col) * 10;
		}
	}
}

// seg004:08C3
void __pascal far check_guard_bumped() {
	if (
		Char.action == actions_1_run_jump &&
		Char.alive < 0 &&
		Char.sword >= sword_2_drawn
	) {
		if (

			#ifdef FIX_PUSH_GUARD_INTO_WALL
			// Should also check for a wall BEHIND the guard, instead of only the current tile
			(fix_push_guard_into_wall && get_tile_behind_char() == tiles_20_wall) ||
			#endif

			get_tile_at_char() == tiles_20_wall ||
			curr_tile2 == tiles_7_doortop_with_floor ||
			(curr_tile2 == tiles_4_gate && can_bump_into_gate()) ||
			(Char.direction >= dir_0_right && (
				get_tile(curr_room, --tile_col, tile_row) == tiles_7_doortop_with_floor ||
				(curr_tile2 == tiles_4_gate && can_bump_into_gate())
			))
		) {
			load_frame_to_obj();
			set_char_collision();
			if (is_obstacle()) {
				short delta_x;
				delta_x = dist_from_wall_behind(curr_tile2);
				if (delta_x < 0 && delta_x > -13) {
					Char.x = char_dx_forward(-delta_x);
					seqtbl_offset_char(seq_65_bump_forward_with_sword); // pushed to wall with sword (Guard)
					play_seq();
					load_fram_det_col();
				}
			}
		}
	}
}

// seg004:0989
void __pascal far check_chomped_guard() {
	get_tile_at_char();
	if ( ! check_chomped_here()) {
		get_tile(curr_room, ++tile_col, tile_row);
		check_chomped_here();
	}
}

// seg004:09B0
int __pascal far check_chomped_here() {
	if (curr_tile2 == tiles_18_chomper &&
		(curr_room_modif[curr_tilepos] & 0x7F) == 2
	) {
		coll_tile_left_xpos = x_bump[tile_col + 5] + 7;
		if (get_left_wall_xpos(curr_room, tile_col, tile_row) < char_x_right_coll &&
			get_right_wall_xpos(curr_room, tile_col, tile_row) > char_x_left_coll
		) {
			chomped();
			return 1;
		} else {
			return 0;
		}
	} else {
		return 0;
	}
}

// seg004:0A10
int __pascal far dist_from_wall_forward(byte tiletype) {
	short type;
	if (tiletype == tiles_4_gate && ! can_bump_into_gate()) {
		return -1;
	} else {
		coll_tile_left_xpos = x_bump[tile_col + 5] + 7;
		type = wall_type(tiletype);
		if (type == 0) return -1;
		if (Char.direction < dir_0_right) {
			// looking left
			//return wall_dist_from_right[type] + char_x_left_coll - coll_tile_left_xpos - 13;
			return char_x_left_coll - (coll_tile_left_xpos + 13 - wall_dist_from_right[type]);
		} else {
			// looking right
			return wall_dist_from_left[type] + coll_tile_left_xpos - char_x_right_coll;
		}
	}
}

// seg004:0A7B
int __pascal far dist_from_wall_behind(byte tiletype) {
	short type;
	type = wall_type(tiletype);
	if (type == 0) {
		return 99;
	} else {
		if (Char.direction >= dir_0_right) {
			// looking right
			//return wall_dist_from_right[type] + char_x_left_coll - coll_tile_left_xpos - 13;
			return char_x_left_coll - (coll_tile_left_xpos + 13 - wall_dist_from_right[type]);
		} else {
			// looking left
			return wall_dist_from_left[type] + coll_tile_left_xpos - char_x_right_coll;
		}
	}
}
