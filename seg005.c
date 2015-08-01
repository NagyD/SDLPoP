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

// data:1888
extern const word seqtbl_offsets[];

// seg005:000A
void __pascal far seqtbl_offset_char(short seq_index) {
	Char.curr_seq = seqtbl_offsets[seq_index];
}

// seg005:001D
void __pascal far seqtbl_offset_opp(int seq_index) {
	Opp.curr_seq = seqtbl_offsets[seq_index];
}

// seg005:0030
void __pascal far do_fall() {
	if (is_screaming == 0 && Char.fall_y >= 31) {
		play_sound(sound_1_falling); // falling
		is_screaming = 1;
	}
	if ((word)y_land[Char.curr_row + 1] > (word)Char.y) {
		check_grab();
	} else {
		if (get_tile_at_char() == tiles_20_wall) {
			in_wall();
		}
		if (tile_is_floor(curr_tile2)) {
			land();
		} else {
			inc_curr_row();
		}
	}
}

// seg005:0090
void __pascal far land() {
	word seq_id;
	is_screaming = 0;
	Char.y = y_land[Char.curr_row + 1];
	if (get_tile_at_char() != tiles_2_spike) {
		if (! tile_is_floor(get_tile_infrontof_char()) &&
			distance_to_edge_weight() < 3
		) {
			Char.x = char_dx_forward(-3);
		}
		start_chompers();
	} else {
		goto loc_5EE6;
	}
	if (Char.alive < 0) {
		if ((distance_to_edge_weight() >= 12 &&
			get_tile_behind_char() == tiles_2_spike) ||
			get_tile_at_char() == tiles_2_spike
		) {
			loc_5EE6:
			if (is_spike_harmful()) {
				spiked();
				return;
			}
		}
		{
			if (Char.fall_y < 22) {
				loc_5EFD:
				if (Char.charid >= charid_2_guard || Char.sword == sword_2_drawn) {
					Char.sword = sword_2_drawn;
					seq_id = seq_63_guard_stand_active; // stand active after landing
				} else {
					seq_id = seq_17_soft_land; // crouch (soft land)
				}
				if (Char.charid == charid_0_kid) {
					play_sound(sound_17_soft_land); // soft land (crouch)
					is_guard_notice = 1;
				}
			} else if (Char.fall_y < 33) {
				if (Char.charid == charid_1_shadow) goto loc_5EFD;
				if (Char.charid == charid_2_guard) goto loc_5F6C;
				if (! take_hp(1)) {
					play_sound(sound_16_medium_land); // medium land
					is_guard_notice = 1;
					seq_id = seq_20_medium_land; // medium land (lose 1 HP, crouch)
				} else {
					goto loc_5F75;
				}
			} else {
				goto loc_5F6C;
			}
		}
	} else {
		loc_5F6C:
		take_hp(100);
		loc_5F75:
		play_sound(sound_0_fell_to_death); // prince crashing into the floor
		seq_id = seq_22_crushed; // dead (after falling)
	}
	seqtbl_offset_char(seq_id);
	play_seq();
	Char.fall_y = 0;
}

// seg005:01B7
void __pascal far spiked() {
	// If someone falls into spikes, those spikes become harmless (to others).
	curr_room_modif[curr_tilepos] = 0xFF;
	Char.y = y_land[Char.curr_row + 1];
	Char.x = x_bump[tile_col + 5] + 10;
	Char.x = char_dx_forward(8);
	Char.fall_y = 0;
	play_sound(sound_48_spiked); // something spiked
	take_hp(100);
	seqtbl_offset_char(seq_51_spiked); // spiked
	play_seq();
}

// seg005:0213
void __pascal far control() {
	short char_frame;
	short char_action;
	char_frame = Char.frame;
	if (Char.alive >= 0) {
		if (char_frame == frame_15_stand || // stand
			char_frame == frame_166_stand_inactive || // stand
			char_frame == frame_158_stand_with_sword || // stand with sword
			char_frame == frame_171_stand_with_sword // stand with sword
		) {
			seqtbl_offset_char(seq_71_dying); // dying (not stabbed)
		}
	} else {
		char_action = Char.action;
		if (char_action == actions_5_bumped ||
			char_action == actions_4_in_freefall
		) {
			release_arrows();
		} else if (Char.sword == sword_2_drawn) {
			control_with_sword();
		} else if (Char.charid >= charid_2_guard) {
			control_guard_inactive();
		} else if (char_frame == frame_15_stand || // standing
			(char_frame>= frame_50_turn && char_frame<53) // end of turning
		) {
			control_standing();
		} else if (char_frame == frame_48_turn) { // a frame in turning
			control_turning();
		} else if (char_frame < 4) { // start run
			control_startrun();
		} else if (char_frame >= frame_67_start_jump_up_1 && char_frame < frame_70_jumphang) { // start jump up
			control_jumpup();
		} else if (char_frame < 15) { // running
			control_running();
		} else if (char_frame >= frame_87_hanging_1 && char_frame < 100) { // hanging
			control_hanging();
		} else if (char_frame == frame_109_crouch) { // crouching
			control_crouched();
		}

		#ifdef ALLOW_CROUCH_AFTER_CLIMBING
		// When ducking with down+forward, give time to release the forward control (prevents unintended crouch-hops)
		else if (Char.curr_seq >= seqtbl_offsets[seq_50_crouch] &&
				Char.curr_seq < seqtbl_offsets[seq_49_stand_up_from_crouch]) // while stooping
			if (control_forward < 1) control_forward = 0;
		#endif

		#ifdef FIX_MOVE_AFTER_DRINK
		if (char_frame >= frame_191_drink && char_frame <= frame_205_drink)
			release_arrows();
		#endif
	}
}

// seg005:02EB
void __pascal far control_crouched() {
	if (need_level1_music != 0 && current_level == 1) {
		// Special event: music when crouching
		if (! check_sound_playing()) {
			if (need_level1_music == 1) {
				play_sound(sound_25_presentation); // presentation (level 1 start)
				need_level1_music = 2;
			} else {
				need_level1_music = 0;
			}
		}
	} else {
		need_level1_music = 0;
		if (control_shift2 < 0 && check_get_item()) return;
		if (control_y != 1) {
			seqtbl_offset_char(seq_49_stand_up_from_crouch); // stand up from crouch
		} else {
			if (control_forward < 0) {
				control_forward = 1; // disable automatic repeat
				seqtbl_offset_char(seq_79_crouch_hop); // crouch-hop
			}
		}
	}
}

// seg005:0358
void __pascal far control_standing() {
	short var_2;
	if (control_shift2 < 0 && control_shift < 0 && check_get_item()) {
		return;
	}
	if (Char.charid != charid_0_kid && control_down < 0 && control_forward < 0) {
		draw_sword();
		return;
	} //else
	if (have_sword) {
		if (word_1EFCE != 0 && control_shift >= 0) goto loc_6213;
		if (can_guard_see_kid >= 2) {
			var_2 = char_opp_dist();
			if (var_2 >= -10 && var_2 < 90) {
				holding_sword = 1;
				if ((word)var_2 < (word)-6) {
					if (Opp.charid == charid_1_shadow &&
						(Opp.action == actions_3_in_midair || (Opp.frame >= frame_107_fall_land_1 && Opp.frame < 118))
					) {
						word_1EFCE = 0;
					} else {
						draw_sword();
						return;
					}
				} else {
					back_pressed();
					return;
				}
			}
		} else {
			word_1EFCE = 0;
		}
	}
	if (control_shift < 0) {
		if (control_backward < 0) {
			back_pressed();
		} else if (control_up < 0) {
			up_pressed();
		} else if (control_down < 0) {
			down_pressed();
		} else if (control_x < 0 && control_forward < 0) {
			safe_step();
		}
	} else loc_6213: if (control_forward < 0) {
		if (is_keyboard_mode && control_up < 0) {
			standing_jump();
		} else {
			forward_pressed();
		}
	} else if (control_backward < 0) {
		back_pressed();
	} else if (control_up < 0) {
		if (is_keyboard_mode && control_forward < 0) {
			standing_jump();
		} else {
			up_pressed();
		}
	} else if (control_down < 0) {
		down_pressed();
	} else if (control_x < 0) {
		forward_pressed();
	}
}

// seg005:0482
void __pascal far up_pressed() {
	int leveldoor_tilepos = -1;
	if (get_tile_at_char() == tiles_16_level_door_left) leveldoor_tilepos = curr_tilepos;
	else if (get_tile_behind_char() == tiles_16_level_door_left) leveldoor_tilepos = curr_tilepos;
	else if (get_tile_infrontof_char() == tiles_16_level_door_left) leveldoor_tilepos = curr_tilepos;
	if ((leveldoor_tilepos != -1) &&
		level.start_room != drawn_room &&
		curr_room_modif[leveldoor_tilepos] >= 42 // this door must be fully open
	){
		go_up_leveldoor();
	} else {
		if (control_x < 0) {
			standing_jump();
		} else {
			check_jump_up();
		}
	}
}

// seg005:04C7
void __pascal far down_pressed() {
	control_down = 1; // disable automatic repeat
	if (! tile_is_floor(get_tile_infrontof_char()) &&
		distance_to_edge_weight() < 3
	) {
		Char.x = char_dx_forward(5);
		load_fram_det_col();
	} else {
		if (! tile_is_floor(get_tile_behind_char()) &&
			distance_to_edge_weight() >= 8
		) {
			through_tile = get_tile_behind_char();
			get_tile_at_char();
			if (can_grab() &&
				#ifdef ALLOW_CROUCH_AFTER_CLIMBING
				control_forward != -1 &&
				#endif
				(Char.direction >= dir_0_right ||
				get_tile_at_char() != tiles_4_gate ||
				curr_room_modif[curr_tilepos] >> 2 >= 6)
			) {
				Char.x = char_dx_forward(distance_to_edge_weight() - 9);
				seqtbl_offset_char(seq_68_climb_down); // climb down
			} else {
				crouch();
			}
		} else {
			crouch();
		}
	}
}

// seg005:0574
void __pascal far go_up_leveldoor() {
	Char.x = x_bump[tile_col + 5] + 10;
	Char.direction = dir_FF_left; // right
	seqtbl_offset_char(seq_70_go_up_on_level_door); // go up on level door
}

// seg005:058F
void __pascal far control_turning() {
	if (control_shift >= 0 && control_x < 0 && control_y >= 0) {
		seqtbl_offset_char(seq_43_start_run_after_turn); // start run and run (after turning)
	}
}

// seg005:05AD
void __pascal far crouch() {
	seqtbl_offset_char(seq_50_crouch); // crouch
	control_down = release_arrows();
}

// seg005:05BE
void __pascal far back_pressed() {
	word seq_id;
	control_backward = release_arrows();
	// After turn, Kid will draw sword if ...
	if (have_sword == 0 || // if Kid has sword
		can_guard_see_kid < 2 || // and can see Guard
		char_opp_dist() > 0 || // and Guard was behind him
		distance_to_edge_weight() < 2
	) {
		seq_id = seq_5_turn; // turn
	} else {
		Char.sword = sword_2_drawn;
		word_1EFCE = 0;
		seq_id = seq_89_turn_draw_sword; // turn and draw sword
	}
	seqtbl_offset_char(seq_id);
}

// seg005:060F
void __pascal far forward_pressed() {
	short distance;
	distance = get_edge_distance();
	#ifdef ALLOW_CROUCH_AFTER_CLIMBING
	if (control_down < 0) {
		down_pressed();
		control_forward = 0;
		return;
	}
	#endif

	if (edge_type == 1 && curr_tile2 != tiles_18_chomper && distance < 8) {
		// If char is near a wall, step instead of run.
		if (control_forward < 0) {
			safe_step();
		}
	} else {
		seqtbl_offset_char(seq_1_start_run); // start run and run
	}
}

// seg005:0649
void __pascal far control_running() {
	if (control_x == 0 && (Char.frame == frame_7_run || Char.frame == frame_11_run)) {
		control_forward = release_arrows();
		seqtbl_offset_char(seq_13_stop_run); // stop run
	} else if (control_x > 0) {
		control_backward = release_arrows();
		seqtbl_offset_char(seq_6_run_turn); // run-turn
	} else if (control_y < 0 && control_up < 0) {
		run_jump();
	} else if (control_down < 0) {
		control_down = 1; // disable automatic repeat
		seqtbl_offset_char(seq_26_crouch_while_running); // crouch while running
	}
}

// seg005:06A8
void __pascal far safe_step() {
	short distance;
	control_shift2 = 1; // disable automatic repeat
	control_forward = 1; // disable automatic repeat
	distance = get_edge_distance();
	if (distance) {
		Char.repeat = 1;
		seqtbl_offset_char(distance + 28); // 29..42: safe step to edge
	} else if (edge_type != 1 && Char.repeat != 0) {
		Char.repeat = 0;
		seqtbl_offset_char(seq_44_step_on_edge); // step on edge
	} else {
		seqtbl_offset_char(seq_39_safe_step_11); // unsafe step (off ledge)
	}
}

// seg005:06F0
int __pascal far check_get_item() {
	if (get_tile_at_char() == tiles_10_potion ||
		curr_tile2 == tiles_22_sword
	) {
		if (! tile_is_floor(get_tile_behind_char())) {
			return 0;
		}
		Char.x = char_dx_forward(-14);
		load_fram_det_col();
	}
	if (get_tile_infrontof_char() == tiles_10_potion ||
		curr_tile2 == tiles_22_sword
	) {
		get_item();
		return 1;
	}
	return 0;
}

// seg005:073E
void __pascal far get_item() {
	short distance;
	if (Char.frame != frame_109_crouch) { // crouching
		distance = get_edge_distance();
		if (edge_type != 2) {
			Char.x = char_dx_forward(distance);
		}
		if (Char.direction >= dir_0_right) {
			Char.x = char_dx_forward((curr_tile2 == tiles_10_potion) - 2);
		}
		crouch();
	} else if (curr_tile2 == tiles_22_sword) {
		do_pickup(-1);
		seqtbl_offset_char(seq_91_get_sword); // get sword
	} else { // potion
		do_pickup(curr_room_modif[curr_tilepos] >> 3);
		seqtbl_offset_char(seq_78_drink); // drink
#ifdef USE_COPYPROT
		if (current_level == 15) {
			short index;
			for (index = 0; index < 14; ++index) {
				// remove letter on potions level
				if (copyprot_room[index] == curr_room &&
					copyprot_tile[index] == curr_tilepos
				) {
					copyprot_room[index] = 0;
					break;
				}
			}
		}
#endif
	}
}

// seg005:07FF
void __pascal far control_startrun() {
	if (control_y < 0 && control_x < 0) {
		standing_jump();
	}
}

// seg005:0812
void __pascal far control_jumpup() {
	if (control_x < 0 || control_forward < 0) {
		standing_jump();
	}
}

// seg005:0825
void __pascal far standing_jump() {
	control_up = control_forward = 1; // disable automatic repeat
	seqtbl_offset_char(seq_3_standing_jump); // standing jump
}

// seg005:0836
void __pascal far check_jump_up() {
	control_up = release_arrows();
	through_tile = get_tile_above_char();
	get_tile_front_above_char();
	if (can_grab()) {
		grab_up_with_floor_behind();
	} else {
		through_tile = get_tile_behind_above_char();
		get_tile_above_char();
		if (can_grab()) {
			jump_up_or_grab();
		} else {
			jump_up();
		}
	}
}

// seg005:087B
void __pascal far jump_up_or_grab() {
	short distance;
	distance = distance_to_edge_weight();
	if (distance < 6) {
		jump_up();
	} else if (! tile_is_floor(get_tile_behind_char())) {
		// There is not floor behind char.
		grab_up_no_floor_behind();
	} else {
		// There is floor behind char, go back a bit.
		Char.x = char_dx_forward(distance - 14);
		load_fram_det_col();
		grab_up_with_floor_behind();
	}
}

// seg005:08C7
void __pascal far grab_up_no_floor_behind() {
	get_tile_above_char();
	Char.x = char_dx_forward(distance_to_edge_weight() - 10);
	seqtbl_offset_char(seq_16_jump_up_and_grab); // jump up and grab (no floor behind)
}

// seg005:08E6
void __pascal far jump_up() {
	short distance;
	control_up = release_arrows();
	distance = get_edge_distance();
	if (distance < 4 && edge_type == 1) {
		Char.x = char_dx_forward(distance - 3);
	}
	#ifdef FIX_JUMP_DISTANCE_AT_EDGE
	// When climbing up two floors, turning around and jumping upward, the kid falls down.
	// This fix makes the workaround of Trick 25 unnecessary.
	if (distance == 3 && edge_type == 0) {
		Char.x = char_dx_forward(-1);
	}
	#endif
	get_tile(Char.room, get_tile_div_mod(back_delta_x(0) + dx_weight() - 6), Char.curr_row - 1);
	if (curr_tile2 != tiles_20_wall && ! tile_is_floor(curr_tile2)) {
		seqtbl_offset_char(seq_28_jump_up_with_nothing_above); // jump up with nothing above
	} else {
		seqtbl_offset_char(seq_14_jump_up_into_ceiling); // jump up with wall or floor above
	}
}

// seg005:0968
void __pascal far control_hanging() {
	if (Char.alive < 0) {
		if (grab_timer == 0 && control_y < 0) {
			can_climb_up();
		} else if (control_shift < 0) {
			// hanging against a wall or a doortop
			if (Char.action != actions_6_hang_straight &&
				(get_tile_at_char() == tiles_20_wall ||
				(Char.direction == dir_FF_left && ( // facing left
					curr_tile2 == tiles_7_doortop_with_floor ||
					curr_tile2 == tiles_12_doortop
				)))
			) {
				if (grab_timer == 0) {
					play_sound(sound_8_bumped); // touching a wall (hang against wall)
				}
				seqtbl_offset_char(seq_25_hang_against_wall); // hang against wall (straight)
			} else {
				if (! tile_is_floor(get_tile_above_char())) {
					hang_fall();
				}
			}
		} else {
			hang_fall();
		}
	} else {
		hang_fall();
	}
}

// seg005:09DF
void __pascal far can_climb_up() {
	short seq_id;
	seq_id = seq_10_climb_up; // climb up
	control_up = control_shift2 = release_arrows();
	get_tile_above_char();
	if (((curr_tile2 == tiles_13_mirror || curr_tile2 == tiles_18_chomper) &&
		Char.direction == dir_0_right) ||
		(curr_tile2 == tiles_4_gate && Char.direction != dir_0_right &&
		curr_room_modif[curr_tilepos] >> 2 < 6)
	) {
		seq_id = seq_73_climb_up_to_closed_gate; // climb up to closed gate and down
	}
	seqtbl_offset_char(seq_id);
}

// seg005:0A46
void __pascal far hang_fall() {
	control_down = release_arrows();
	if (! tile_is_floor(get_tile_behind_char()) &&
		! tile_is_floor(get_tile_at_char())
	) {
		seqtbl_offset_char(seq_23_release_ledge_and_fall); // release ledge and fall
	} else {
		if (get_tile_at_char() == tiles_20_wall ||
			(Char.direction < dir_0_right && ( // looking left
				curr_tile2 == tiles_7_doortop_with_floor ||
				curr_tile2 == tiles_12_doortop
			))
		) {
			Char.x = char_dx_forward(-7);
		}
		seqtbl_offset_char(seq_11_release_ledge_and_land); // end of climb down
	}
}

// seg005:0AA8
void __pascal far grab_up_with_floor_behind() {
	short distance;
	distance = distance_to_edge_weight();

	// The global variable edge_type (which we need!) gets set as a side effect of get_edge_distance()
	short edge_distance = get_edge_distance();
	//printf("Distance to edge weight: %d\tedge type: %d\tedge distance: %d\n", distance, edge_type, edge_distance);

	#ifdef FIX_EDGE_DISTANCE_CHECK_WHEN_CLIMBING
	// When climbing to a higher floor, the game unnecessarily checks how far away the edge below is;
	// This contributes to sometimes "teleporting" considerable distances when climbing from firm ground
	#define JUMP_STRAIGHT_CONDITION distance < 4 && edge_type != 1
	#else
	#define JUMP_STRAIGHT_CONDITION distance < 4 && edge_distance < 4 && edge_type != 1
	#endif

	if (JUMP_STRAIGHT_CONDITION) {
		Char.x = char_dx_forward(distance);
		seqtbl_offset_char(seq_8_jump_up_and_grab_straight); // jump up and grab (when?)
	} else {
		Char.x = char_dx_forward(distance - 4);
		seqtbl_offset_char(seq_24_jump_up_and_grab_forward); // jump up and grab (with floor behind)
	}
}

// seg005:0AF7
void __pascal far run_jump() {
	short var_2;
	short xpos;
	short col;
	short var_8;
	if (Char.frame >= frame_7_run) {
		// Align Kid to edge of floor.
		xpos = char_dx_forward(4);
		col = get_tile_div_mod_m7(xpos);
		for (var_2 = 0; var_2 < 2; ++var_2) {
			col += dir_front[Char.direction + 1];
			get_tile(Char.room, col, Char.curr_row);
			if (curr_tile2 == tiles_2_spike || ! tile_is_floor(curr_tile2)) {
				var_8 = distance_to_edge(xpos) + 14 * var_2 - 14;
				if ((word)var_8 < (word)-8 || var_8 >= 2) {
					if (var_8 < 128) return;
					var_8 = -3;
				}
				Char.x = char_dx_forward(var_8 + 4);
				break;
			}
		}
		control_up = release_arrows(); // disable automatic repeat
		seqtbl_offset_char(seq_4_run_jump); // run-jump
	}
}

// sseg005:0BB5
void __pascal far back_with_sword() {
	short frame;
	frame = Char.frame;
	if (frame == frame_158_stand_with_sword || frame == frame_170_stand_with_sword || frame == frame_171_stand_with_sword) {
		control_backward = 1; // disable automatic repeat
		seqtbl_offset_char(seq_57_back_with_sword); // back with sword
	}
}

// seg005:0BE3
void __pascal far forward_with_sword() {
	short frame;
	frame = Char.frame;
	if (frame == frame_158_stand_with_sword || frame == frame_170_stand_with_sword || frame == frame_171_stand_with_sword) {
		control_forward = 1; // disable automatic repeat
		if (Char.charid != charid_0_kid) {
			seqtbl_offset_char(seq_56_guard_forward_with_sword); // forward with sword (Guard)
		} else {
			seqtbl_offset_char(seq_86_forward_with_sword); // forward with sword (Kid)
		}
	}
}

// seg005:0C1D
void __pascal far draw_sword() {
	word seq_id;
	seq_id = seq_55_draw_sword; // draw sword
	control_forward = control_shift2 = release_arrows();
	if (Char.charid == charid_0_kid) {
		play_sound(sound_19_draw_sword); // taking out the sword
		word_1EFCE = 0;
	} else if (Char.charid != charid_1_shadow) {
		seq_id = seq_90_en_garde; // stand active
	}
	Char.sword = sword_2_drawn;
	seqtbl_offset_char(seq_id);
}

// seg005:0C67
void __pascal far control_with_sword() {
	short distance;
	if (Char.action < actions_2_hang_climb) {
		if (get_tile_at_char() == tiles_11_loose || can_guard_see_kid >= 2) {
			distance = char_opp_dist();
			if ((word)distance < (word)90) {
				swordfight();
				return;
			} else if (distance < 0) {
				if ((word)distance < (word)-4) {
					seqtbl_offset_char(seq_60_turn_with_sword); // turn with sword (after switching places)
					return;
				} else {
					swordfight();
					return;
				}
			}
		} /*else*/ {
			if (Char.charid == charid_0_kid && Char.alive < 0) {
				holding_sword = 0;
			}
			if (Char.charid < charid_2_guard) {
				// frame 171: stand with sword
				if (Char.frame == frame_171_stand_with_sword) {
					Char.sword = sword_0_sheathed;
					seqtbl_offset_char(seq_92_put_sword_away); // put sword away (Guard died)
				}
			} else {
				swordfight();
			}
		}
	}
}

// seg005:0CDB
void __pascal far swordfight() {
	short frame;
	short seq_id;
	short charid;
	frame = Char.frame;
	charid = Char.charid;
	// frame 161: parry
	if (frame == frame_161_parry && control_shift2 >= 0) {
		seqtbl_offset_char(seq_57_back_with_sword); // back with sword (when parrying)
		return;
	} else if (control_shift2 < 0) {
		if (charid == charid_0_kid) {
			kid_sword_strike = 15;
		}
		sword_strike();
		if (control_shift2 == 1) return;
	}
	if (control_down < 0) {
		if (frame == frame_158_stand_with_sword || frame == frame_170_stand_with_sword || frame == frame_171_stand_with_sword) {
			control_down = 1; // disable automatic repeat
			Char.sword = sword_0_sheathed;
			if (charid == charid_0_kid) {
				word_1EFCE = 1;
				guard_refrac = 9;
				holding_sword = 0;
				seq_id = seq_93_put_sword_away_fast; // put sword away fast (down pressed)
			} else if (charid == charid_1_shadow) {
				seq_id = seq_92_put_sword_away; // put sword away
			} else {
				seq_id = seq_87_guard_become_inactive; // stand inactive (when Kid leaves sight)
			}
			seqtbl_offset_char(seq_id);
		}
	} else if (control_up < 0) {
		parry();
	} else if (control_forward < 0) {
		forward_with_sword();
	} else if (control_backward < 0) {
		back_with_sword();
	}
}

// seg005:0DB0
void __pascal far sword_strike() {
	short frame;
	short seq_id;
	frame = Char.frame;
	if (frame == frame_157_walk_with_sword || // walk with sword
		frame == frame_158_stand_with_sword || // stand with sword
		frame == frame_170_stand_with_sword || // stand with sword
		frame == frame_171_stand_with_sword || // stand with sword
		frame == frame_165_walk_with_sword // walk with sword
	) {
		if (Char.charid == charid_0_kid) {
			seq_id = seq_75_strike; // strike with sword (Kid)
		} else {
			seq_id = seq_58_guard_strike; // strike with sword (Guard)
		}
	} else if (frame == frame_150_parry || frame == frame_161_parry) { // parry
		seq_id = seq_66_strike_after_parry; // strike with sword after parrying
	} else {
		return;
	}
	control_shift2 = 1; // disable automatic repeat
	seqtbl_offset_char(seq_id);
}

// seg005:0E0F
void __pascal far parry() {
	short opp_frame;
	short char_frame;
	short var_6;
	short seq_id;
	short char_charid;
	char_frame = Char.frame;
	opp_frame = Opp.frame;
	char_charid = Char.charid;
	seq_id = seq_62_parry; // defend (parry) with sword
	var_6 = 0;
	if (
		char_frame == frame_158_stand_with_sword || // stand with sword
		char_frame == frame_170_stand_with_sword || // stand with sword
		char_frame == frame_171_stand_with_sword || // stand with sword
		char_frame == frame_168_back || // back?
		char_frame == frame_165_walk_with_sword // walk with sword
	) {
		if (char_opp_dist() >= 32 && char_charid != charid_0_kid) {
			back_with_sword();
			return;
		} else if (char_charid == charid_0_kid) {
			if (opp_frame == frame_168_back) return;
			if (opp_frame != frame_151_strike_1 &&
				opp_frame != frame_152_strike_2 &&
				opp_frame != frame_162_block_to_strike
			) {
				if (opp_frame == frame_153_strike_3) { // strike
					var_6 = 1;
				} else
				if (char_charid != charid_0_kid) {
					back_with_sword();
					return;
				}
			}
		} else {
			if (opp_frame != frame_152_strike_2) return;
		}
	} else {
		if (char_frame != frame_167_blocked) return;
		seq_id = seq_61_parry_after_strike; // parry after striking with sword
	}
	control_up = 1; // disable automatic repeat
	seqtbl_offset_char(seq_id);
	if (var_6) {
		play_seq();
	}
}
