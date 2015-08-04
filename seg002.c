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

// data:0E32
const word strikeprob  [] = { 61,100, 61, 61, 61, 40,100,220,  0, 48, 32, 48};
// data:0E4A
const word restrikeprob[] = {  0,  0,  0,  5,  5,175, 16,  8,  0,255,255,150};
// data:0E62
const word blockprob   [] = {  0,150,150,200,200,255,200,250,  0,255,255,255};
// data:0E7A
const word impblockprob[] = {  0, 61, 61,100,100,145,100,250,  0,145,255,175};
// data:0E92
const word advprob     [] = {255,200,200,200,255,255,200,  0,  0,255,100,100};
// data:0EAA
const word refractimer [] = { 16, 16, 16, 16,  8,  8,  8,  8,  0,  8,  0,  0};
// data:0EC2
const word extrastrength[] = {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0};
// data:0EDA
const byte tbl_guard_hp[] = {4, 3, 3, 3, 3, 4, 5, 4, 4, 5, 5, 5, 4, 6, 0, 0};

// seg002:0000
void __pascal far do_init_shad(const byte *source,int seq_index) {
	memcpy_near(&Char, source, 7);
	seqtbl_offset_char(seq_index);
	Char.charid = charid_1_shadow;
	demo_time = 0;
	guard_skill = 3;
	guardhp_delta = guardhp_curr = guardhp_max = 4;
	saveshad();
}

// seg002:0044
void __pascal far get_guard_hp() {
	guardhp_delta = guardhp_curr = guardhp_max = extrastrength[guard_skill] + tbl_guard_hp[current_level];
}

// data:0EEA
const byte init_shad_6[] = {0x0F, 0x51, 0x76, 0, 0, 1, 0, 0};
// data:0EF2
const byte init_shad_5[] = {0x0F, 0x37, 0x37, 0, 0xFF, 0, 0, 0};
// data:0EFA
const byte init_shad_12[] = {0x0F, 0x51, 0xE8, 0, 0, 0, 0, 0};

// seg002:0064
void __pascal far check_shadow() {
	word_1EFCE = 0;
	if (current_level == 12) {
		// Special event: level 12 shadow
		if (!united_with_shadow && drawn_room == 15) {
			Char.room = drawn_room;
			if (get_tile(15, 1, 0) == tiles_22_sword) {
				return;
			}
			shadow_initialized = 0;
			do_init_shad(/*&*/init_shad_12, 7 /*fall*/);
			return;
		}
	} else if (current_level == 6) {
		// Special event: level 6 shadow
		Char.room = drawn_room;
		if (Char.room == 1) {
			if (leveldoor_open != 0x4D) {
				play_sound(sound_25_presentation); // presentation (level 6 shadow)
				leveldoor_open = 0x4D;
			}
			do_init_shad(/*&*/init_shad_6, 2 /*stand*/);
			return;
		}
	} else if (current_level == 5) {
		// Special event: level 5 shadow
		Char.room = drawn_room;
		if (Char.room == 24) {
			if (get_tile(24, 3, 0) != tiles_10_potion) {
				return;
			}
			do_init_shad(/*&*/init_shad_5, 2 /*stand*/);
			return;
		}
	}
	enter_guard();
}

// seg002:0112
void __pascal far enter_guard() {
	word room_minus_1;
	word guard_tile;
	word frame;
	byte seq_hi;
	// arrays are indexed 0..23 instead of 1..24
	room_minus_1 = drawn_room - 1;
	frame = Char.frame; // hm?
	guard_tile = level.guards_tile[room_minus_1];

	if (guard_tile >= 30) return;

	Char.room = drawn_room;
	Char.curr_row = guard_tile / 10;
	Char.y = y_land[Char.curr_row + 1];
	Char.x = level.guards_x[room_minus_1];
	Char.curr_col = get_tile_div_mod_m7(Char.x);
	Char.direction = level.guards_dir[room_minus_1];
	// only regular guards have different colors (and only on VGA)
	if (graphics_mode == gmMcgaVga && tbl_guard_type[current_level] == 0) {
		curr_guard_color = level.guards_color[room_minus_1];
	} else {
		curr_guard_color = 0;
	}
	// level 3 has skeletons with infinite lives
	if (current_level == 3) {
		Char.charid = charid_4_skeleton;
	} else {
		Char.charid = charid_2_guard;
	}
	seq_hi = level.guards_seq_hi[room_minus_1];
	if (seq_hi == 0) {
		if (Char.charid == charid_4_skeleton) {
			Char.sword = sword_2_drawn;
			seqtbl_offset_char(seq_63_guard_stand_active); // stand active (when entering room) (skeleton)
		} else {
			Char.sword = sword_0_sheathed;
			seqtbl_offset_char(seq_77_guard_stand_inactive); // stand inactive (when entering room)
		}
	} else {
		Char.curr_seq = level.guards_seq_lo[room_minus_1] + (seq_hi << 8);
	}
    play_seq();
	guard_skill = level.guards_skill[room_minus_1];
	if (guard_skill >= 12) {
		guard_skill = 3;
	}
	frame = Char.frame;
	if (frame == frame_185_dead || frame == frame_177_spiked || frame == frame_178_chomped) {
		Char.alive = 1;
		draw_guard_hp(0, guardhp_curr);
		guardhp_curr = 0;
	} else {
		Char.alive = -1;
		word_1E1AA = 0;
		guard_refrac = 0;
		is_guard_notice = 0;
		get_guard_hp();
	}
	Char.fall_y = 0;
	Char.fall_x = 0;
	Char.action = actions_1_run_jump;
	saveshad();
}

// seg002:0269
void __pascal far check_guard_fallout() {
	if (Guard.direction == dir_56_none || Guard.y < 211) {
		return;
	}
	if (Guard.charid == charid_1_shadow) {
		if (Guard.action != actions_4_in_freefall) {
			return;
		}
		loadshad();
		clear_char();
		saveshad();
	} else if (Guard.charid == charid_4_skeleton &&
		(Guard.room = level.roomlinks[Guard.room - 1].down) == 3) {
		// if skeleton falls down into room 3
		Guard.x = 133;
		Guard.curr_row = 1;
		Guard.direction = dir_0_right;
		Guard.alive = -1;
		leave_guard();
	} else {
		on_guard_killed();
		level.guards_tile[drawn_room - 1] = -1;
		Guard.direction = dir_56_none;
		draw_guard_hp(0, guardhp_curr);
		guardhp_curr = 0;
	}
}

// seg002:02F5
void __pascal far leave_guard() {
	word room_minus_1;
	if (Guard.direction == dir_56_none || Guard.charid == charid_1_shadow || Guard.charid == charid_24_mouse) {
		return;
	}
	// arrays are indexed 0..23 instead of 1..24
	room_minus_1 = Guard.room - 1;
	level.guards_tile[room_minus_1] = get_tilepos(0, Guard.curr_row);
	level.guards_color[room_minus_1] = curr_guard_color;
	level.guards_x[room_minus_1] = Guard.x;
	level.guards_dir[room_minus_1] = Guard.direction;
	level.guards_skill[room_minus_1] = guard_skill;
	if (Guard.alive < 0) {
		level.guards_seq_hi[room_minus_1] = 0;
	} else {
		level.guards_seq_lo[room_minus_1] = Guard.curr_seq;
		level.guards_seq_hi[room_minus_1] = Guard.curr_seq >> 8;
	}
	Guard.direction = dir_56_none;
	draw_guard_hp(0, guardhp_curr);
	guardhp_curr = 0;
}

// seg002:039E
void __pascal far follow_guard() {
	level.guards_tile[Kid.room - 1] = 0xFF;
	level.guards_tile[Guard.room - 1] = 0xFF;
	loadshad();
	goto_other_room(roomleave_result);
	saveshad();
}

// seg002:03C7
void __pascal far exit_room() {
	word leave;
	word kid_room_m1;
	leave = 0;
	if (exit_room_timer != 0) {
		--exit_room_timer;
		return;
	}
	loadkid();
	load_frame_to_obj();
	set_char_collision();
	roomleave_result = leave_room();
	if (roomleave_result < 0) {
		return;
	}
	savekid();
	next_room = Char.room;
	if (Guard.direction == dir_56_none) return;
	if (Guard.alive < 0 && Guard.sword == sword_2_drawn) {
		kid_room_m1 = Kid.room - 1;
		if (level.guards_tile[kid_room_m1] >= 30 ||
			level.guards_seq_hi[kid_room_m1] != 0
		) {
			if (roomleave_result == 0) {
				// left
				if (Guard.x >= 91) leave = 1;
				#ifdef FIX_GUARD_FOLLOWING_THROUGH_CLOSED_GATES
				else if (Guard.x > 0) {
					get_tile(Kid.room, 9, Kid.curr_row);
					if (curr_tile2 == tiles_4_gate && can_bump_into_gate())
							leave = 1;
				}
				#endif
			} else if (roomleave_result == 1) {
				// right
				if (Guard.x < 165) leave = 1;
			} else if (roomleave_result == 2) {
				// up
				if (Guard.curr_row >= 0) leave = 1;
			} else {
				// down
				if (Guard.curr_row < 3) leave = 1;
			}
		} else {
			leave = 1;
		}
	} else {
		leave = 1;
	}
	if (leave) {
		leave_guard();
	} else {
		follow_guard();
	}
}

// seg002:0486
int __pascal far goto_other_room(short direction) {
	short opposite_dir;
	Char.room = ((byte*)&level.roomlinks[Char.room - 1])[direction];
	if (direction == 0) {
		// left
		Char.x += 140;
		opposite_dir = 1;
	} else if (direction == 1) {
		// right
		Char.x -= 140;
		opposite_dir = 0;
	} else if (direction == 2) {
		// up
		Char.y += 189;
		Char.curr_row = y_to_row_mod4(Char.y);
		opposite_dir = 3;
	} else {
		// down
		Char.y -= 189;
		Char.curr_row = y_to_row_mod4(Char.y);
		opposite_dir = 2;
	}
	return opposite_dir;
}

// seg002:0504
short __pascal far leave_room() {
	short frame;
	word action;
	short chary;
	short leave_dir;
	chary = Char.y;
	action = Char.action;
	frame = Char.frame;
	if (action != actions_5_bumped &&
		action != actions_4_in_freefall &&
		action != actions_3_in_midair &&
		(sbyte)chary < 10 && (sbyte)chary > -16
	) {
		leave_dir = 2; // up
	} else if (chary >= 211) {
		leave_dir = 3; // down
	} else if (
		// frames 135..149: climb up
		(frame >= frame_135_climbing_1 && frame < 150) ||
		// frames 110..119: standing up from crouch
		(frame >= frame_110_stand_up_from_crouch_1 && frame < 120) ||
		// frames 150..162: with sword
		(frame >= frame_150_parry && frame < 163) ||
		// frames 166..168: with sword
		(frame >= frame_166_stand_inactive && frame < 169) ||
		action == actions_7_turn // turn
	) {
		return -1;
	} else if (Char.direction != dir_0_right) {
		// looking left
		if (char_x_left <= 54) {
			leave_dir = 0; // left
		} else if (char_x_left >= 198) {
			leave_dir = 1; // right
		} else {
			return -1;
		}
	} else {
		// looking right
		get_tile(Char.room, 9, Char.curr_row);
		if (curr_tile2 != tiles_7_doortop_with_floor &&
			curr_tile2 != tiles_12_doortop &&
			char_x_right >= 201
		) {
			leave_dir = 1; // right
		} else if (char_x_right <= 57) {
			leave_dir = 0; // left
		} else {
			return -1;
		}
	}
	switch (leave_dir) {
		case 0: // left
			play_mirr_mus();
			level3_set_chkp();
			Jaffar_exit();
		break;
		case 1: // right
			sword_disappears();
			meet_Jaffar();
		break;
		//case 2: // up
		case 3: // down
			// Special event: falling exit
			if (current_level == 6 && Char.room == 1) {
				return -2;
			}
		break;
	}
	goto_other_room(leave_dir);
	return leave_dir;
}

// seg002:0643
void __pascal far Jaffar_exit() {
	if (leveldoor_open == 2) {
		get_tile(24, 0, 0);
		trigger(0, 0, -1);
	}
}

// seg002:0665
void __pascal far level3_set_chkp() {
	// Special event: set checkpoint
	if (current_level == 3 && Char.room == 7) {
		checkpoint = 1;
		hitp_beg_lev = hitp_max;
	}
}

// seg002:0680
void __pascal far sword_disappears() {
	// Special event: sword disappears
	if (current_level == 12 && Char.room == 18) {
		get_tile(15, 1, 0);
		curr_room_tiles[curr_tilepos] = tiles_1_floor;
	}
}

// seg002:06AE
void __pascal far meet_Jaffar() {
	// Special event: play music
	if (current_level == 13 && leveldoor_open == 0 && Char.room == 3) {
		play_sound(sound_29_meet_Jaffar); // meet Jaffar
		// Special event: Jaffar waits a bit (28/12=2.33 seconds)
		guard_notice_timer = 28;
	}
}

// seg002:06D3
void __pascal far play_mirr_mus() {
	// Special event: mirror music
	if (
		leveldoor_open != 0 &&
		leveldoor_open != 0x4D && // was the music played already?
		current_level == 4 &&
		Char.curr_row == 0 &&
		Char.room == 11
	) {
		play_sound(sound_25_presentation); // presentation (level 4 mirror)
		leveldoor_open = 0x4D;
	}
}

// seg002:0706
void __pascal far move_0_nothing() {
	control_shift = 0;
	control_y = 0;
	control_x = 0;
	control_shift2 = 0;
	control_down = 0;
	control_up = 0;
	control_backward = 0;
	control_forward = 0;
}

// seg002:0721
void __pascal far move_1_forward() {
	control_x = -1;
	control_forward = -1;
}

// seg002:072A
void __pascal far move_2_backward() {
	control_backward = -1;
	control_x = 1;
}

// seg002:0735
void __pascal far move_3_up() {
	control_y = -1;
	control_up = -1;
}

// seg002:073E
void __pascal far move_4_down() {
	control_down = -1;
	control_y = 1;
}

// seg002:0749
void __pascal far move_up_back() {
	control_up = -1;
	move_2_backward();
}

// seg002:0753
void __pascal far move_down_back() {
	control_down = -1;
	move_2_backward();
}

// seg002:075D
void __pascal far move_down_forw() {
	control_down = -1;
	move_1_forward();
}

// seg002:0767
void __pascal far move_6_shift() {
	control_shift = -1;
	control_shift2 = -1;
}

// seg002:0770
void __pascal far move_7() {
	control_shift = 0;
}

// seg002:0776
void __pascal far autocontrol_opponent() {
	word charid;
	move_0_nothing();
	charid = Char.charid;
	if (charid == charid_0_kid) {
		autocontrol_kid();
	} else {
		if (word_1E1AA) --word_1E1AA;
		if (kid_sword_strike) --kid_sword_strike;
		if (guard_refrac) --guard_refrac;
		if (charid == charid_24_mouse) {
			autocontrol_mouse();
		} else if (charid == charid_4_skeleton) {
			autocontrol_skeleton();
		} else if (charid == charid_1_shadow) {
			autocontrol_shadow();
		} else if (current_level == 13) {
			autocontrol_Jaffar();
		} else {
			autocontrol_guard();
		}
	}
}

// seg002:07EB
void __pascal far autocontrol_mouse() {
	if (Char.direction == dir_56_none) {
		return;
	}
	if (Char.action == actions_0_stand) {
		if (Char.x >= 200) {
			clear_char();
		}
	} else {
		if (Char.x < 166) {
			seqtbl_offset_char(seq_107_mouse_stand_up_and_go); // mouse
			play_seq();
		}
	}
}

// seg002:081D
void __pascal far autocontrol_shadow() {
	if (current_level == 4) {
		autocontrol_shadow_level4();
	} else if (current_level == 5) {
		autocontrol_shadow_level5();
	} else if (current_level == 6) {
		autocontrol_shadow_level6();
	} else if (current_level == 12) {
		autocontrol_shadow_level12();
	}
}

// seg002:0850
void __pascal far autocontrol_skeleton() {
	Char.sword = sword_2_drawn;
	autocontrol_guard();
}

// seg002:085A
void __pascal far autocontrol_Jaffar() {
	autocontrol_guard();
}

// seg002:085F
void __pascal far autocontrol_kid() {
	autocontrol_guard();
}

// seg002:0864
void __pascal far autocontrol_guard() {
	if (Char.sword < sword_2_drawn) {
		autocontrol_guard_inactive();
	} else {
		autocontrol_guard_active();
	}
}

// seg002:0876
void __pascal far autocontrol_guard_inactive() {
	short distance;
	if (Kid.alive >= 0) return;
	distance = char_opp_dist();
	if (Opp.curr_row != Char.curr_row || (word)distance < (word)-8) {
		// If Kid made a sound ...
		if (is_guard_notice) {
			is_guard_notice = 0;
			if (distance < 0) {
				// ... and Kid is behind Guard, Guard turns around.
				if ((word)distance < (word)-4) {
					move_4_down();
				}
				return;
			}
		} else if (distance < 0) {
			return;
		}
	}
	if (can_guard_see_kid) {
		// If Guard can see Kid, Guard moves to fighting pose.
		if (current_level != 13 || guard_notice_timer == 0) {
			move_down_forw();
		}
	}
}

// seg002:08DC
void __pascal far autocontrol_guard_active() {
	short opp_frame;
	short char_frame;
	short distance;
	char_frame = Char.frame;
	if (char_frame != frame_166_stand_inactive && char_frame >= 150 && can_guard_see_kid != 1) {
		if (can_guard_see_kid == 0) {
			if (word_1EA12 != 0) {
				guard_follows_kid_down();
				//return;
			} else if (Char.charid != charid_4_skeleton) {
				move_down_back();
			}
			//return;
		} else { // can_guard_see_kid == 2
			opp_frame = Opp.frame;
			distance = char_opp_dist();
			if (distance >= 12 &&
				// frames 102..117: falling and landing
				opp_frame >= frame_102_start_fall_1 && opp_frame < frame_118_stand_up_from_crouch_9 &&
				Opp.action == actions_5_bumped
			) {
				return;
			}
			if (distance < 35) {
				if ((Char.sword < sword_2_drawn && distance < 8) || distance < 12) {
					if (Char.direction == Opp.direction) {
						// turn around
						move_2_backward();
						//return;
					} else {
						move_1_forward();
						//return;
					}
				} else {
					autocontrol_guard_kid_in_sight(distance);
					//return;
				}
			} else {
				if (guard_refrac != 0) return;
				if (Char.direction != Opp.direction) {
					// frames 7..14: running
					// frames 34..43: run-jump
					if (opp_frame >= frame_7_run && opp_frame < 15) {
						if (distance < 40) move_6_shift();
						return;
					} else if (opp_frame >= frame_34_start_run_jump_1 && opp_frame < 44) {
						if (distance < 50) move_6_shift();
						return;
						//return;
					}
				}
				autocontrol_guard_kid_far();
			}
			//...
		}
		//...
	}
}

// seg002:09CB
void __pascal far autocontrol_guard_kid_far() {
	if (tile_is_floor(get_tile_infrontof_char()) ||
		tile_is_floor(get_tile_infrontof2_char())) {
		move_1_forward();
	} else {
		move_2_backward();
	}
}

// seg002:09F8
void __pascal far guard_follows_kid_down() {
	// This is called from autocontrol_guard_active, so char=Guard, Opp=Kid
	word opp_action;
	opp_action = Opp.action;
	if (opp_action == actions_2_hang_climb || opp_action == actions_6_hang_straight) {
		return;
	}
	if (// there is wall in front of Guard
		wall_type(get_tile_infrontof_char()) != 0 ||
		(! tile_is_floor(curr_tile2) && (
			(get_tile(curr_room, tile_col, ++tile_row) == tiles_2_spike ||
			// Guard would fall on loose floor
			curr_tile2 == tiles_11_loose ||
			// ... or wall (?)
			wall_type(curr_tile2) != 0 ||
			// ... or into a chasm
			! tile_is_floor(curr_tile2)) ||
			// ... or Kid is not below
			Char.curr_row + 1 != Opp.curr_row
		))
	) {
		// don't follow
		word_1EA12 = 0;
		move_2_backward();
	} else {
		// follow
		move_1_forward();
	}
}

// seg002:0A93
void __pascal far autocontrol_guard_kid_in_sight(short distance) {
	if (Opp.sword == sword_2_drawn) {
		autocontrol_guard_kid_armed(distance);
	} else if (guard_refrac == 0) {
		if (distance < 29) {
			move_6_shift();
		} else {
			move_1_forward();
		}
	}
}

// seg002:0AC1
void __pascal far autocontrol_guard_kid_armed(short distance) {
	if (distance < 10 || distance >= 29) {
		guard_advance();
	} else {
		guard_block();
		if (guard_refrac == 0) {
			if (distance < 12 || distance >= 29) {
				guard_advance();
			} else {
				guard_strike();
			}
		}
	}
}

// seg002:0AF5
void __pascal far guard_advance() {
	if (guard_skill == 0 || kid_sword_strike == 0) {
		if (advprob[guard_skill] > prandom(255)) {
			move_1_forward();
		}
	}
}

// seg002:0B1D
void __pascal far guard_block() {
	word opp_frame;
	opp_frame = Opp.frame;
	if (opp_frame == frame_152_strike_2 || opp_frame == frame_153_strike_3 || opp_frame == frame_162_block_to_strike) {
		if (word_1E1AA != 0) {
			if (impblockprob[guard_skill] > prandom(255)) {
				move_3_up();
			}
		} else {
			if (blockprob[guard_skill] > prandom(255)) {
				move_3_up();
			}
		}
	}
}

// seg002:0B73
void __pascal far guard_strike() {
	word opp_frame;
	word char_frame;
	opp_frame = Opp.frame;
	if (opp_frame == frame_169_begin_block || opp_frame == frame_151_strike_1) return;
	char_frame = Char.frame;
	if (char_frame == frame_161_parry || char_frame == frame_150_parry) {
		if (restrikeprob[guard_skill] > prandom(255)) {
			move_6_shift();
		}
	} else {
		if (strikeprob[guard_skill] > prandom(255)) {
			move_6_shift();
		}
	}
}

// seg002:0BCD
void __pascal far hurt_by_sword() {
	short distance;
	if (Char.alive >= 0) return;
	if (Char.sword != sword_2_drawn) {
		// Being hurt when not in fighting pose means death.
		take_hp(100);
		seqtbl_offset_char(seq_85_stabbed_to_death); // dying (stabbed unarmed)
		loc_4276:
		if (get_tile_behind_char() != 0 ||
			(distance = distance_to_edge_weight()) < 4
		) {
			seqtbl_offset_char(seq_85_stabbed_to_death); // dying (stabbed)
			if (Char.charid != charid_0_kid &&
				Char.direction < dir_0_right && // looking left
				(curr_tile2 == tiles_4_gate || get_tile_at_char() == tiles_4_gate)
			) {
				Char.x = x_bump[tile_col - (curr_tile2 != tiles_4_gate) + 5] + 7;
				Char.x = char_dx_forward(10);
			}
			Char.y = y_land[Char.curr_row + 1];
			Char.fall_y = 0;
		} else {
			Char.x = char_dx_forward(distance - 20);
			load_fram_det_col();
			inc_curr_row();
			seqtbl_offset_char(seq_81_kid_pushed_off_ledge); // Kid/Guard is killed and pushed off the ledge
		}
	} else {
		// You can't hurt skeletons
		if (Char.charid != charid_4_skeleton) {
			if (take_hp(1)) goto loc_4276;
		}
		seqtbl_offset_char(seq_74_hit_by_sword); // being hit with sword
		Char.y = y_land[Char.curr_row + 1];
		Char.fall_y = 0;
	}
	// sound 13: Kid hurt (by sword), sound 12: Guard hurt (by sword)
	play_sound(Char.charid == charid_0_kid ? sound_13_kid_hurt : sound_12_guard_hurt);
	play_seq();
}

// seg002:0CD4
void __pascal far check_sword_hurt() {
	if (Guard.action == actions_99_hurt) {
		if (Kid.action == actions_99_hurt) {
			Kid.action = actions_1_run_jump;
		}
		loadshad();
		hurt_by_sword();
		saveshad();
		guard_refrac = refractimer[guard_skill];
	} else {
		if (Kid.action == actions_99_hurt) {
			loadkid();
			hurt_by_sword();
			savekid();
		}
	}
}

// seg002:0D1A
void __pascal far check_sword_hurting() {
	short kid_frame;
	kid_frame = Kid.frame;
	// frames 217..228: go up on stairs
	if (kid_frame != 0 && (kid_frame < frame_219_exit_stairs_3 || kid_frame >= 229)) {
		loadshad_and_opp();
		check_hurting();
		saveshad_and_opp();
		loadkid_and_opp();
		check_hurting();
		savekid_and_opp();
	}
}

// seg002:0D56
void __pascal far check_hurting() {
	short opp_frame, char_frame, distance, min_hurt_range;
	if (Char.sword != sword_2_drawn) return;
	if (Char.curr_row != Opp.curr_row) return;
	char_frame = Char.frame;
	// frames 153..154: poking with sword
	if (char_frame != frame_153_strike_3 && char_frame != frame_154_poking) return;
	// If char is poking ...
	distance = char_opp_dist();
	opp_frame = Opp.frame;
	// frames 161 and 150: parrying
	if (distance < 0 || distance >= 29 ||
		(opp_frame != frame_161_parry && opp_frame != frame_150_parry)
	) {
		// ... and Opp is not parrying
		// frame 154: poking
		if (Char.frame == frame_154_poking) {
			if (Opp.sword < sword_2_drawn) {
				min_hurt_range = 8;
			} else {
				min_hurt_range = 12;
			}
			distance = char_opp_dist();
			if (distance >= min_hurt_range && distance < 29) {
				Opp.action = actions_99_hurt;
			}
		}
	} else {
		Opp.frame = frame_161_parry;
		if (Char.charid != charid_0_kid) {
			word_1E1AA = 4;
		}
		seqtbl_offset_char(seq_69_attack_was_parried); // attack was parried
		play_seq();
	}
	// frame 154: poking
	// frame 161: parrying
	if (Char.frame == frame_154_poking && Opp.frame != frame_161_parry && Opp.action != actions_99_hurt) {
		play_sound(sound_11_sword_moving); // sword moving
	}
}

// seg002:0E1F
void __pascal far check_skel() {
	// Special event: skeleton wakes
	if (current_level == 3 &&
		Guard.direction == dir_56_none &&
		drawn_room == 1 &&
		leveldoor_open != 0 &&
		(Kid.curr_col == 2 || Kid.curr_col == 3)
	) {
		get_tile(drawn_room, 5, 1);
		if (curr_tile2 == tiles_21_skeleton) {
			// erase skeleton
			curr_room_tiles[curr_tilepos] = tiles_1_floor;
			redraw_height = 24;
			set_redraw_full(curr_tilepos, 1);
			set_wipe(curr_tilepos, 1);
			++curr_tilepos;
			set_redraw_full(curr_tilepos, 1);
			set_wipe(curr_tilepos, 1);
			Char.room = drawn_room;
			Char.curr_row = 1;
			Char.y = y_land[Char.curr_row + 1];
			Char.curr_col = 5;
			Char.x = x_bump[Char.curr_col + 5] + 14;
			Char.direction = dir_FF_left;
			seqtbl_offset_char(seq_88_skel_wake_up); // skel wake up
			play_seq();
			play_sound(sound_44_skel_alive); // skel alive
			guard_skill = 2;
			Char.alive = -1;
			guardhp_max = guardhp_curr = 3;
			Char.fall_x = Char.fall_y = 0;
			is_guard_notice = guard_refrac = 0;
			Char.sword = sword_2_drawn;
			Char.charid = charid_4_skeleton;
			saveshad();
		}
	}
}

// seg002:0F3F
void __pascal far do_auto_moves(const auto_move_type *moves_ptr) {
	short demoindex;
	short curr_move;
	if (demo_time >= 0xFE) return;
	++demo_time;
	demoindex = demo_index;
	if (moves_ptr[demoindex].time <= demo_time) {
		++demo_index;
	} else {
		demoindex = demo_index - 1;
	}
	curr_move = moves_ptr[demoindex].move;
	switch (curr_move) {
		case -1:
		break;
		case 0:
			move_0_nothing();
		break;
		case 1:
			move_1_forward();
		break;
		case 2:
			move_2_backward();
		break;
		case 3:
			move_3_up();
		break;
		case 4:
			move_4_down();
		break;
		case 5:
			move_3_up();
			move_1_forward();
		break;
		case 6:
			move_6_shift();
		break;
		case 7:
			move_7();
		break;
	}
}

// seg002:1000
void __pascal far autocontrol_shadow_level4() {
	if (Char.room == 4) {
		if (Char.x < 80) {
			clear_char();
		} else {
			move_1_forward();
		}
	}
}

// data:0F02
const auto_move_type shad_drink_move[] = {
{0x00, 0},
{0x01, 1},
{0x0E, 0},
{0x12, 6},
{0x1D, 7},
{0x2D, 2},
{0x31, 1},
{0xFF,-2},
};

// seg002:101A
void __pascal far autocontrol_shadow_level5() {
	if (Char.room == 24) {
		if (demo_time == 0) {
			get_tile(24, 1, 0);
			// is the door open?
			if (curr_room_modif[curr_tilepos] < 80) return;
			demo_index = 0;
		}
		do_auto_moves(shad_drink_move);
		if (Char.x < 15) {
			clear_char();
		}
	}
}

// seg002:1064
void __pascal far autocontrol_shadow_level6() {
	if (Char.room == 1 &&
		Kid.frame == frame_43_running_jump_4 && // a frame in run-jump
		Kid.x < 128
	) {
		move_6_shift();
		move_1_forward();
	}
}

// seg002:1082
void __pascal far autocontrol_shadow_level12() {
	short opp_frame;
	short xdiff;
	if (Char.room == 15 && shadow_initialized == 0) {
		if (Opp.x >= 150) {
			do_init_shad(/*&*/init_shad_12, 7 /*fall*/);
			return;
		}
		shadow_initialized = 1;
	}
	if (Char.sword >= sword_2_drawn) {
		// if the Kid puts his sword away, the shadow does the same,
		// but only if the shadow was already hurt (?)
		if (word_1EFCE == 0 || guard_refrac == 0) {
			autocontrol_guard_active();
		} else {
			move_4_down();
		}
		return;
	}
	if (Opp.sword >= sword_2_drawn || word_1EFCE == 0) {
		xdiff = 0x7000; // bugfix/workaround
		// This behavior matches the DOS version but not the Apple II source.
		if (can_guard_see_kid < 2 || (xdiff = char_opp_dist()) >= 90) {
			if (xdiff < 0) {
				move_2_backward();
			}
			return;
		}
		// Shadow draws his sword
		if (Char.frame == 15) {
			move_down_forw();
		}
		return;
	}
	if (char_opp_dist() < 10) {
		// unite with the shadow
		flash_color = color_15_white; // white
		flash_time = 18;
		// get an extra HP for uniting the shadow
		add_life();
		// time of Kid-shadow flash
		united_with_shadow = 42;
		// put the Kid where the shadow was
		Char.charid = charid_0_kid;
		savekid();
		// remove the shadow
		clear_char();
		return;
	}
	if (can_guard_see_kid == 2) {
		// If Kid runs to shadow, shadow runs to Kid.
		opp_frame = Opp.frame;
		// frames 1..14: running
		// frames 121..132: stepping
		if ((opp_frame >= frame_3_start_run && opp_frame < frame_15_stand) ||
			(opp_frame >= frame_127_stepping_7 && opp_frame < 133)
		) {
			move_1_forward();
		}
	}
}
