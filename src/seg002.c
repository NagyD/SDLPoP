/*
SDLPoP, a port/conversion of the DOS game Prince of Persia.
Copyright (C) 2013-2025  Dávid Nagy

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

The authors of this program may be contacted at https://forum.princed.org
*/

#include "common.h"

// These were moved to custom_options_type.
/*
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
*/

// seg002:0000
void do_init_shad(const byte *source,int seq_index) {
	memcpy(&Char, source, 7);
	seqtbl_offset_char(seq_index);
	Char.charid = charid_1_shadow;
	demo_time = 0;
	guard_skill = 3;
	guardhp_delta = guardhp_curr = guardhp_max = 4;
	saveshad();
}

// seg002:0044
void get_guard_hp() {
	guardhp_delta = guardhp_curr = guardhp_max = custom->extrastrength[guard_skill] + custom->tbl_guard_hp[current_level];
}

// These were moved to custom_options_type.
/*
// data:0EEA
const byte init_shad_6[] = {0x0F, 0x51, 0x76, 0, 0, 1, 0, 0};
// data:0EF2
const byte init_shad_5[] = {0x0F, 0x37, 0x37, 0, 0xFF, 0, 0, 0};
// data:0EFA
const byte init_shad_12[] = {0x0F, 0x51, 0xE8, 0, 0, 0, 0, 0};
*/

// seg002:0064
void check_shadow() {
	offguard = 0;
	if (current_level == 12) {
		// Special event: level 12 shadow
		if (!united_with_shadow && drawn_room == 15) {
			Char.room = drawn_room;
			if (get_tile(15, 1, 0) == tiles_22_sword) {
				return;
			}
			shadow_initialized = 0;
			do_init_shad(/*&*/custom->init_shad_12, 7 /*fall*/);
			return;
		}
	} /*else*/
	if (current_level == /*6*/ custom->shadow_step_level) {
		// Special event: level 6 shadow
		Char.room = drawn_room;
		if (Char.room == /*1*/ custom->shadow_step_room) {
			if (leveldoor_open != 0x4D) {
				play_sound(sound_25_presentation); // presentation (level 6 shadow)
				leveldoor_open = 0x4D;
			}
			do_init_shad(/*&*/custom->init_shad_6, 2 /*stand*/);
			return;
		}
	} /*else*/
	if (current_level == /*5*/ custom->shadow_steal_level) {
		// Special event: level 5 shadow
		Char.room = drawn_room;
		if (Char.room == /*24*/ custom->shadow_steal_room) {
			if (get_tile(/*24*/ custom->shadow_steal_room, 3, 0) != tiles_10_potion) {
				return;
			}
			do_init_shad(/*&*/custom->init_shad_5, 2 /*stand*/);
			return;
		}
	}
	enter_guard();
}

// seg002:0112
void enter_guard() {
	// arrays are indexed 0..23 instead of 1..24
	word room_minus_1 = drawn_room - 1;
	word frame = Char.frame; // hm?
	word guard_tile = level.guards_tile[room_minus_1];
#ifndef FIX_OFFSCREEN_GUARDS_DISAPPEARING
	if (guard_tile >= 30) return;
#else
	if (guard_tile >= 30) {
		if (!fixes->fix_offscreen_guards_disappearing) return;

		// try to see if there are offscreen guards in the left and right rooms that might be visible from this room
		short left_guard_tile = 31;
		short right_guard_tile = 31;
		if (room_L > 0) left_guard_tile = level.guards_tile[room_L-1];
		if (room_R > 0) right_guard_tile = level.guards_tile[room_R-1];

		int other_guard_x;
		sbyte other_guard_dir;
		int delta_x;
		int other_room_minus_1;
		if (right_guard_tile >= 0 && right_guard_tile < 30) {
			other_room_minus_1 = room_R - 1;
			other_guard_x = level.guards_x[other_room_minus_1];
			other_guard_dir = level.guards_dir[other_room_minus_1];
			// left edge of the guard matters
			if (other_guard_dir == dir_0_right) other_guard_x -= 9; // only retrieve a guard if they will be visible
			if (other_guard_dir == dir_FF_left) other_guard_x += 1; // getting these right was mostly trial and error
			// only retrieve offscreen guards
			if (!(other_guard_x < 58 + 4)) {
				// check the left offscreen guard
				if (left_guard_tile >= 0 && left_guard_tile < 30) {
					goto loc_left_guard_tile;
				}
				return;
			}
			delta_x = 140; // guard leaves to the left
			guard_tile = right_guard_tile;
		}
		else if (left_guard_tile >= 0 && left_guard_tile < 30) {
loc_left_guard_tile:
			other_room_minus_1 = room_L - 1;
			other_guard_x = level.guards_x[other_room_minus_1];
			other_guard_dir = level.guards_dir[other_room_minus_1];
			// right edge of the guard matters
			if (other_guard_dir == dir_0_right) other_guard_x -= 9;
			if (other_guard_dir == dir_FF_left) other_guard_x += 1;
			// only retrieve offscreen guards
			if (!(other_guard_x > 190 - 4)) return;
			delta_x = -140; // guard leaves to the right
			guard_tile = left_guard_tile;
		}
		else return;

		// retrieve guard from adjacent room
		level.guards_x[room_minus_1] = level.guards_x[other_room_minus_1] + delta_x;
		level.guards_color[room_minus_1] = level.guards_color[other_room_minus_1];
		level.guards_dir[room_minus_1] = level.guards_dir[other_room_minus_1];
		level.guards_seq_hi[room_minus_1] = level.guards_seq_hi[other_room_minus_1];
		level.guards_seq_lo[room_minus_1] = level.guards_seq_lo[other_room_minus_1];
		level.guards_skill[room_minus_1] = level.guards_skill[other_room_minus_1];

		level.guards_tile[other_room_minus_1] = 0xFF;
		level.guards_seq_hi[other_room_minus_1] = 0;
	}
#endif

	Char.room = drawn_room;
	Char.curr_row = guard_tile / SCREEN_TILECOUNTX;
	Char.y = y_land[Char.curr_row + 1];
	Char.x = level.guards_x[room_minus_1];
	Char.curr_col = get_tile_div_mod_m7(Char.x);
	Char.direction = level.guards_dir[room_minus_1];
	// only regular guards have different colors (and only on VGA)
	if (graphics_mode == gmMcgaVga && custom->tbl_guard_type[current_level] == 0) {
		curr_guard_color = level.guards_color[room_minus_1];
	} else {
		curr_guard_color = 0;
	}

	#ifdef REMEMBER_GUARD_HP
	int remembered_hp = (level.guards_color[room_minus_1] & 0xF0) >> 4;
	#endif
	curr_guard_color &= 0x0F; // added; only least significant 4 bits are used for guard color

	// level 3 has skeletons with infinite lives
	//if (current_level == 3) {
	if (custom->tbl_guard_type[current_level] == 2) {
		Char.charid = charid_4_skeleton;
	} else {
		Char.charid = charid_2_guard;
	}
	byte seq_hi = level.guards_seq_hi[room_minus_1];
	if (seq_hi == 0) {
		if (Char.charid == charid_4_skeleton) {
			Char.sword = sword_2_drawn;
			seqtbl_offset_char(seq_63_guard_active_after_fall); // stand active (when entering room) (skeleton)
		} else {
			Char.sword = sword_0_sheathed;
			seqtbl_offset_char(seq_77_guard_stand_inactive); // stand inactive (when entering room)
		}
	} else {
		Char.curr_seq = level.guards_seq_lo[room_minus_1] + (seq_hi << 8);
	}
	play_seq();
	guard_skill = level.guards_skill[room_minus_1];
	if (guard_skill >= NUM_GUARD_SKILLS) {
		guard_skill = 3;
	}
	frame = Char.frame;
	if (frame == frame_185_dead || frame == frame_177_spiked || frame == frame_178_chomped) {
		Char.alive = 1;
		draw_guard_hp(0, guardhp_curr);
		guardhp_curr = 0;
	} else {
		Char.alive = -1;
		justblocked = 0;
		guard_refrac = 0;
		is_guard_notice = 0;
		get_guard_hp();
		#ifdef REMEMBER_GUARD_HP
		if (fixes->enable_remember_guard_hp && remembered_hp > 0)
			guardhp_delta = guardhp_curr = (word) remembered_hp;
		#endif
	}
	Char.fall_y = 0;
	Char.fall_x = 0;
	Char.action = actions_1_run_jump;
	saveshad();
}

// seg002:0269
void check_guard_fallout() {
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
		// should the level number be checked too?
		level.roomlinks[Guard.room - 1].down == /*3*/ custom->skeleton_reappear_room
	) {
		// if skeleton falls down into room 3
		Guard.room = level.roomlinks[Guard.room - 1].down;
		Guard.x = /*133*/ custom->skeleton_reappear_x;
		Guard.curr_row = /*1*/ custom->skeleton_reappear_row;
		Guard.direction = /*dir_0_right*/ custom->skeleton_reappear_dir;
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
void leave_guard() {
	if (Guard.direction == dir_56_none || Guard.charid == charid_1_shadow || Guard.charid == charid_24_mouse) {
		return;
	}
	// arrays are indexed 0..23 instead of 1..24
	word room_minus_1 = Guard.room - 1;
	level.guards_tile[room_minus_1] = get_tilepos(0, Guard.curr_row);

	level.guards_color[room_minus_1] = curr_guard_color & 0x0F; // restriction to 4 bits added
#ifdef REMEMBER_GUARD_HP
	if (fixes->enable_remember_guard_hp && guardhp_curr < 16) // can remember 1..15 hp
		level.guards_color[room_minus_1] |= (guardhp_curr << 4);
#endif

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
void follow_guard() {
	level.guards_tile[Kid.room - 1] = 0xFF;
	level.guards_tile[Guard.room - 1] = 0xFF;
	loadshad();
	goto_other_room(roomleave_result);
	saveshad();
}

// seg002:03C7
void exit_room() {
	short leave = 0;
	if (exit_room_timer != 0) {
		--exit_room_timer;
#ifdef FIX_HANG_ON_TELEPORT
		if (!(fixes->fix_hang_on_teleport && Char.y >= 211 && Char.curr_row >= 2))
#endif
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
#ifdef FIX_DISAPPEARING_GUARD_B
	if (next_room == drawn_room) return;
#endif
#ifdef USE_SUPER_HIGH_JUMP
	// Do not change the room during super high jumps from row 1.
	// Kid's "y" coordinate keeps him in the room below (in timers()).
	if (fixes->enable_super_high_jump && super_jump_fall && next_room == drawn_room) {
		return;
	}
#endif
	if (Guard.direction == dir_56_none) return;
	if (Guard.alive < 0 && Guard.sword == sword_2_drawn) {
		short kid_room_m1 = Kid.room - 1;
		// kid_room_m1 might be 65535 (-1) when the prince fell out of the level (to room 0) while a guard was active.
		// In this case, the indexing in the following condition crashes on Linux.
		if ((kid_room_m1 >= 0 && kid_room_m1 <= 23) &&
			(level.guards_tile[kid_room_m1] >= 30 || level.guards_seq_hi[kid_room_m1] != 0)
		) {
			if (roomleave_result == 0) {
				// left
				if (Guard.x >= 91) leave = 1;
				#ifdef FIX_GUARD_FOLLOWING_THROUGH_CLOSED_GATES
				else if (fixes->fix_guard_following_through_closed_gates && can_guard_see_kid != 2 &&
						Kid.sword != sword_2_drawn) {
					leave = 1;
				}
				#endif
			} else if (roomleave_result == 1) {
				// right
				if (Guard.x < 165) leave = 1;
				#ifdef FIX_GUARD_FOLLOWING_THROUGH_CLOSED_GATES
				else if (fixes->fix_guard_following_through_closed_gates && can_guard_see_kid != 2 &&
						 Kid.sword != sword_2_drawn) {
					leave = 1;
				}
				#endif
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

#ifdef FIX_DISAPPEARING_GUARD_A
	if (next_room == drawn_room) drawn_room = 0;
#endif
}

// seg002:0486
int goto_other_room(short direction) {
	//printf("goto_other_room: direction = %d, Char.room = %d\n", direction, Char.room);
	short opposite_dir;
	byte other_room = ((byte*)&level.roomlinks[Char.room - 1])[direction];
#ifdef FIX_ENTERING_GLITCHED_ROOMS
	if (Char.room == 0) {
		other_room = 0;
	}
#endif
	Char.room = other_room;
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
short leave_room() {
	short leave_dir;
	short chary = Char.y;
	word action = Char.action;
	short frame = Char.frame;
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
		(frame >= frame_150_parry && frame < 163

#ifdef FIX_RETREAT_WITHOUT_LEAVING_ROOM
			// By repeatedly pressing 'back' in a swordfight, you can retreat out of a room without the room changing. (Trick 35)

			// The game waits for a 'legal frame' (e.g. frame_170_stand_with_sword) until leaving is possible;
			// However, this frame is never reached if you press 'back' in the correct pattern!

			// Solution: also allow the room to be changed on frame_157_walk_with_sword
			// Note that this means that the delay for leaving rooms in a swordfight becomes noticably shorter.

			&& (frame != frame_157_walk_with_sword || !fixes->fix_retreat_without_leaving_room)
#endif

		) ||
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
			if (current_level == custom->falling_exit_level /*6*/ && Char.room == custom->falling_exit_room /*1*/) {
				return -2;
			}
		break;
	}
	goto_other_room(leave_dir);
#ifdef USE_REPLAY
	if (skipping_replay && replay_seek_target == replay_seek_0_next_room) skipping_replay = 0;
#endif
	return leave_dir;
}

// seg002:0643
void Jaffar_exit() {
	if (leveldoor_open == 2) {
		get_tile(24, 0, 0);
		trigger_button(0, 0, -1);
	}
}

// seg002:0665
void level3_set_chkp() {
	// Special event: set checkpoint
	if (current_level == /*3*/ custom->checkpoint_level && Char.room == 7 /* TODO: add a custom option */) {
		checkpoint = 1;
		hitp_beg_lev = hitp_max;
	}
}

// seg002:0680
void sword_disappears() {
	// Special event: sword disappears
	if (current_level == 12 && Char.room == 18) {
		get_tile(15, 1, 0);
		curr_room_tiles[curr_tilepos] = tiles_1_floor;
		curr_room_modif[curr_tilepos] = 0; // added, a nonzero modifier may show fake tiles
	}
}

// seg002:06AE
void meet_Jaffar() {
	// Special event: play music
	if (current_level == 13 && leveldoor_open == 0 && Char.room == 3) {
		play_sound(sound_29_meet_Jaffar); // meet Jaffar
		// Special event: Jaffar waits a bit (28/12=2.33 seconds)
		guard_notice_timer = 28;
	}
}

// seg002:06D3
void play_mirr_mus() {
	// Special event: mirror music
	if (
		leveldoor_open != 0 &&
		leveldoor_open != 0x4D && // was the music played already?
		current_level == /*4*/ custom->mirror_level &&
		Char.curr_row == /*0*/ custom->mirror_row &&
		Char.room == 11 /* TODO: add a custom option */
	) {
		play_sound(sound_25_presentation); // presentation (level 4 mirror)
		leveldoor_open = 0x4D;
	}
}

// seg002:0706
void move_0_nothing() {
	control_shift = CONTROL_RELEASED;
	control_y = CONTROL_RELEASED;
	control_x = CONTROL_RELEASED;
	control_shift2 = CONTROL_RELEASED;
	control_down = CONTROL_RELEASED;
	control_up = CONTROL_RELEASED;
	control_backward = CONTROL_RELEASED;
	control_forward = CONTROL_RELEASED;
}

// seg002:0721
void move_1_forward() {
	control_x = CONTROL_HELD_FORWARD;
	control_forward = CONTROL_HELD;
}

// seg002:072A
void move_2_backward() {
	control_backward = CONTROL_HELD;
	control_x = CONTROL_HELD_BACKWARD;
}

// seg002:0735
void move_3_up() {
	control_y = CONTROL_HELD_UP;
	control_up = CONTROL_HELD;
}

// seg002:073E
void move_4_down() {
	control_down = CONTROL_HELD;
	control_y = CONTROL_HELD_DOWN;
}

// seg002:0749
void move_up_back() {
	control_up = CONTROL_HELD;
	move_2_backward();
}

// seg002:0753
void move_down_back() {
	control_down = CONTROL_HELD;
	move_2_backward();
}

// seg002:075D
void move_down_forw() {
	control_down = CONTROL_HELD;
	move_1_forward();
}

// seg002:0767
void move_6_shift() {
	control_shift = CONTROL_HELD;
	control_shift2 = CONTROL_HELD;
}

// seg002:0770
void move_7() {
	control_shift = CONTROL_RELEASED;
}

// seg002:0776
void autocontrol_opponent() {
	move_0_nothing();
	word charid = Char.charid;
	if (charid == charid_0_kid) {
		autocontrol_kid();
	} else {
		if (justblocked) --justblocked;
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
void autocontrol_mouse() {
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
void autocontrol_shadow() {
	if (current_level == /*4*/ custom->mirror_level) {
		autocontrol_shadow_level4();
	} /*else*/
	if (current_level == /*5*/ custom->shadow_steal_level) {
		autocontrol_shadow_level5();
	} /*else*/
	if (current_level == /*6*/ custom->shadow_step_level) {
		autocontrol_shadow_level6();
	} /*else*/
	if (current_level == 12) {
		autocontrol_shadow_level12();
	}
}

// seg002:0850
void autocontrol_skeleton() {
	Char.sword = sword_2_drawn;
	autocontrol_guard();
}

// seg002:085A
void autocontrol_Jaffar() {
	autocontrol_guard();
}

// seg002:085F
void autocontrol_kid() {
	autocontrol_guard();
}

// seg002:0864
void autocontrol_guard() {
	if (Char.sword < sword_2_drawn) {
		autocontrol_guard_inactive();
	} else {
		autocontrol_guard_active();
	}
}

// seg002:0876
void autocontrol_guard_inactive() {
	if (Kid.alive >= 0) return;
	short distance = char_opp_dist();
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
void autocontrol_guard_active() {
	short char_frame = Char.frame;
	if (char_frame != frame_166_stand_inactive && char_frame >= 150 && can_guard_see_kid != 1) {
		if (can_guard_see_kid == 0) {
			if (droppedout != 0) {
				guard_follows_kid_down();
				//return;
			} else if (Char.charid != charid_4_skeleton) {
				move_down_back();
			}
			//return;
		} else { // can_guard_see_kid == 2
			short opp_frame = Opp.frame;
			short distance = char_opp_dist();
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
void autocontrol_guard_kid_far() {
	if (tile_is_floor(get_tile_infrontof_char()) ||
		tile_is_floor(get_tile_infrontof2_char())) {
		move_1_forward();
	} else {
		move_2_backward();
	}
}

// seg002:09F8
void guard_follows_kid_down() {
	// This is called from autocontrol_guard_active, so char=Guard, Opp=Kid
	word opp_action = Opp.action;
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
		droppedout = 0;
		move_2_backward();
	} else {
		// follow
		move_1_forward();
	}
}

// seg002:0A93
void autocontrol_guard_kid_in_sight(short distance) {
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
void autocontrol_guard_kid_armed(short distance) {
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
void guard_advance() {
	if (guard_skill == 0 || kid_sword_strike == 0) {
		if (custom->advprob[guard_skill] > prandom(255)) {
			move_1_forward();
		}
	}
}

// seg002:0B1D
void guard_block() {
	word opp_frame = Opp.frame;
	if (opp_frame == frame_152_strike_2 || opp_frame == frame_153_strike_3 || opp_frame == frame_162_block_to_strike) {
		if (justblocked != 0) {
			if (custom->impblockprob[guard_skill] > prandom(255)) {
				move_3_up();
			}
		} else {
			if (custom->blockprob[guard_skill] > prandom(255)) {
				move_3_up();
			}
		}
	}
}

// seg002:0B73
void guard_strike() {
	word opp_frame = Opp.frame;
	if (opp_frame == frame_169_begin_block || opp_frame == frame_151_strike_1) return;
	word char_frame = Char.frame;
	if (char_frame == frame_161_parry || char_frame == frame_150_parry) {
		if (custom->restrikeprob[guard_skill] > prandom(255)) {
			move_6_shift();
		}
	} else {
		if (custom->strikeprob[guard_skill] > prandom(255)) {
			move_6_shift();
		}
	}
}

// seg002:0BCD
void hurt_by_sword() {
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
				#ifdef FIX_OFFSCREEN_GUARDS_DISAPPEARING
				// a guard can get teleported to the other side of kid's room
				// when fighting between rooms and hitting a gate
				if (fixes->fix_offscreen_guards_disappearing) {
					short gate_col = tile_col;
					if (curr_room != Char.room)	{
						if (curr_room == level.roomlinks[Char.room - 1].right) {
							gate_col += SCREEN_TILECOUNTX;
						} else if (curr_room == level.roomlinks[Char.room - 1].left) {
							gate_col -= SCREEN_TILECOUNTX;
						}
					}
					Char.x = x_bump[gate_col - (curr_tile2 != tiles_4_gate) + FIRST_ONSCREEN_COLUMN] + TILE_MIDX;
				} else {
				#endif
					Char.x = x_bump[tile_col - (curr_tile2 != tiles_4_gate) + FIRST_ONSCREEN_COLUMN] + TILE_MIDX;
				#ifdef FIX_OFFSCREEN_GUARDS_DISAPPEARING
				}
				#endif
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
void check_sword_hurt() {
	if (Guard.action == actions_99_hurt) {
		if (Kid.action == actions_99_hurt) {
			Kid.action = actions_1_run_jump;
		}
		loadshad();
		hurt_by_sword();
		saveshad();
		guard_refrac = custom->refractimer[guard_skill];
	} else {
		if (Kid.action == actions_99_hurt) {
			loadkid();
			hurt_by_sword();
			savekid();
		}
	}
}

// seg002:0D1A
void check_sword_hurting() {
	short kid_frame = Kid.frame;
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
void check_hurting() {
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
			justblocked = 4;
		}
		seqtbl_offset_char(seq_69_attack_was_parried); // attack was parried
		play_seq();
	}
	if (Char.direction == dir_56_none) return; // Fix looping "sword moving" sound.
	// frame 154: poking
	// frame 161: parrying
	if (Char.frame == frame_154_poking && Opp.frame != frame_161_parry && Opp.action != actions_99_hurt) {
		play_sound(sound_11_sword_moving); // sword moving
	}
}

// seg002:0E1F
void check_skel() {
	// Special event: skeleton wakes
	if (current_level == /*3*/ custom->skeleton_level &&
		Guard.direction == dir_56_none &&
		drawn_room == /*1*/ custom->skeleton_room &&
		(leveldoor_open != 0 || !custom->skeleton_require_open_level_door) &&
		(Kid.curr_col == /*2*/ custom->skeleton_trigger_column_1 ||
				Kid.curr_col == /*3*/ custom->skeleton_trigger_column_2)
	) {
		get_tile(drawn_room, /*5*/ custom->skeleton_column, /*1*/ custom->skeleton_row);
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
			Char.curr_row = /*1*/ custom->skeleton_row;
			Char.y = y_land[Char.curr_row + 1];
			Char.curr_col = /*5*/ custom->skeleton_column;
			Char.x = x_bump[Char.curr_col + FIRST_ONSCREEN_COLUMN] + TILE_SIZEX;
			Char.direction = dir_FF_left;
			seqtbl_offset_char(seq_88_skel_wake_up); // skel wake up
			play_seq();
			play_sound(sound_44_skel_alive); // skel alive
			guard_skill = /*2*/ custom->skeleton_skill;
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
void do_auto_moves(const auto_move_type *moves_ptr) {
	if (demo_time >= 0xFE) return;
	++demo_time;
	short demoindex = demo_index;
	if (moves_ptr[demoindex].time <= demo_time) {
		++demo_index;
	} else {
		demoindex = demo_index - 1;
	}
	short curr_move = moves_ptr[demoindex].move;
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
void autocontrol_shadow_level4() {
	if (Char.room == /*4*/ custom->mirror_room) {
		if (Char.x < 80) {
			clear_char();
		} else {
			move_1_forward();
		}
	}
}

// This was moved to custom_options_type.
/*
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
*/

// seg002:101A
void autocontrol_shadow_level5() {
	if (Char.room == /*24*/ custom->shadow_steal_room) {
		if (demo_time == 0) {
			get_tile(/*24*/ custom->shadow_steal_room, 1, 0);
			// is the door open?
			if (curr_room_modif[curr_tilepos] < 80) return;
			demo_index = 0;
		}
		do_auto_moves(custom->shad_drink_move);
		if (Char.x < 15) {
			clear_char();
		}
	}
}

// seg002:1064
void autocontrol_shadow_level6() {
	if (Char.room == /*1*/ custom->shadow_step_room &&
		Kid.frame == frame_43_running_jump_4 && // a frame in run-jump
		Kid.x < 128
	) {
		move_6_shift();
		move_1_forward();
	}
}

// seg002:1082
void autocontrol_shadow_level12() {
	if (Char.room == 15 && shadow_initialized == 0) {
		if (Opp.x >= 150) {
			do_init_shad(/*&*/custom->init_shad_12, 7 /*fall*/);
			return;
		}
		shadow_initialized = 1;
	}
	if (Char.sword >= sword_2_drawn) {
		// if the Kid puts his sword away, the shadow does the same,
		// but only if the shadow was already hurt (?)
		if (offguard == 0 || guard_refrac == 0) {
			autocontrol_guard_active();
		} else {
			move_4_down();
		}
		return;
	}
	if (Opp.sword >= sword_2_drawn || offguard == 0) {
		short xdiff = 0x7000; // bugfix/workaround
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
		flash_color = color_15_brightwhite; // white
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
		short opp_frame = Opp.frame;
		// frames 1..14: running
		// frames 121..132: stepping
		if ((opp_frame >= frame_3_start_run && opp_frame < frame_15_stand) ||
			(opp_frame >= frame_127_stepping_7 && opp_frame < 133)
		) {
			move_1_forward();
		}
	}
}
