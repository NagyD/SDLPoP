/*
SDLPoP, a port/conversion of the DOS game Prince of Persia.
Copyright (C) 2013-2021  Dávid Nagy

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

// data:3D1A
sbyte distance_mirror;

// seg003:0000
void __pascal far init_game(int level) {
	if(offscreen_surface) {
		free_surface(offscreen_surface); // missing in original
		offscreen_surface = NULL;
	}
	offscreen_surface = make_offscreen_buffer(&rect_top);
	load_kid_sprite();
	text_time_remaining = 0;
	text_time_total = 0;
	is_show_time = 0;
	checkpoint = 0;
	upside_down = 0; // N.B. upside_down is also reset in set_start_pos()
	resurrect_time = 0;
	if (!dont_reset_time) {
		rem_min = custom->start_minutes_left;   // 60
		rem_tick = custom->start_ticks_left;    // 719
		hitp_beg_lev = custom->start_hitp;      // 3
	}
	need_level1_music = (level == /*1*/ custom->intro_music_level);
	play_level(level);
}

// seg003:005C
void __pascal far play_level(int level_number) {
	cutscene_ptr_type cutscene_func;
#ifdef USE_COPYPROT
	if (enable_copyprot && level_number == custom->copyprot_level) {
		level_number = 15;
	}
#endif
	for (;;) {
		if (demo_mode && level_number > 2) {
			start_level = -1;
			need_quotes = 1;
			start_game();
		}
		if (level_number != current_level) {
			if (level_number <0 || level_number >15) {
				printf("Tried to load cutscene for level %d, not in 0..15\n", level_number);
				quit(1);
			}
			cutscene_func = tbl_cutscenes[custom->tbl_cutscenes_by_index[level_number]];
			if (cutscene_func != NULL

				#ifdef USE_REPLAY
				&& !(recording || replaying)
				#endif
#ifdef USE_SCREENSHOT
				&& !want_auto_screenshot()
#endif
					) {
				load_intro(level_number > 2, cutscene_func, 1);
			}
		}
		if (level_number != current_level) {
			load_lev_spr(level_number);
		}
		load_level();
		pos_guards();
		clear_coll_rooms();
		clear_saved_ctrl();
		drawn_room = 0;
		mobs_count = 0;
		trobs_count = 0;
		next_sound = -1;
		holding_sword = 0;
		grab_timer = 0;
		can_guard_see_kid = 0;
		united_with_shadow = 0;
		flash_time = 0;
		leveldoor_open = 0;
		demo_index = 0;
		demo_time = 0;
		guardhp_curr = 0;
		hitp_delta = 0;
		Guard.charid = charid_2_guard;
		Guard.direction = dir_56_none;
		do_startpos();
		have_sword = /*(level_number != 1)*/ (level_number == 0 || level_number >= custom->have_sword_from_level);
		find_start_level_door();
		// busy waiting?
		while (check_sound_playing() && !do_paused()) idle();
		stop_sounds();
		#ifdef USE_REPLAY
		if (replaying) replay_restore_level();
		if (skipping_replay) {
			if (replay_seek_target == replay_seek_0_next_room ||
				replay_seek_target == replay_seek_1_next_level
			)
				skipping_replay = 0; // resume replay from here
		}
		#endif
		draw_level_first();
		show_copyprot(0);
		level_number = play_level_2();
		// hacked...
#ifdef USE_COPYPROT
		if (enable_copyprot && level_number == custom->copyprot_level && !demo_mode) {
			level_number = 15;
		} else {
			if (level_number == 16) {
				level_number = custom->copyprot_level;
				custom->copyprot_level = -1;
			}
		}
#endif
		free_peels();
	}
}

// seg003:01A3
void __pascal far do_startpos() {
	word x;
	// Special event: start at checkpoint
	if (current_level == /*3*/ custom->checkpoint_level && checkpoint) {
		level.start_dir = /*dir_FF_left*/ custom->checkpoint_respawn_dir;
		level.start_room = /*2*/ custom->checkpoint_respawn_room;
		level.start_pos = /*6*/ custom->checkpoint_respawn_tilepos;
		// Special event: remove loose floor
		get_tile(/*7*/ custom->checkpoint_clear_tile_room,
				/*4*/ custom->checkpoint_clear_tile_col,
				/*0*/ custom->checkpoint_clear_tile_row);
		curr_room_tiles[curr_tilepos] = tiles_0_empty;
	}
	next_room = Char.room = level.start_room;
	x = level.start_pos;
	Char.curr_col = x % 10;
	Char.curr_row = x / 10;
	Char.x = x_bump[Char.curr_col + 5] + 14;
	// Start in the opposite direction (and turn into the correct one).
	Char.direction = ~ level.start_dir;
	if (seamless == 0) {
		if (current_level != 0) {
			x = hitp_beg_lev;
		} else {
			// HP on demo level
			x = /*4*/ custom->demo_hitp;
		}
		hitp_max = hitp_curr = x;
	}
	if (/*current_level == 1*/ custom->tbl_entry_pose[current_level] == 1) {
		// Special event: press button + falling entry
		get_tile(5, 2, 0);
		trigger_button(0, 0, -1);
		seqtbl_offset_char(seq_7_fall); // fall
	} else if (/*current_level == 13*/ custom->tbl_entry_pose[current_level] == 2) {
		// Special event: running entry
		seqtbl_offset_char(seq_84_run); // run
	} else {
		seqtbl_offset_char(seq_5_turn); // turn
	}
	set_start_pos();
}

// seg003:028A
void __pascal far set_start_pos() {
	Char.y = y_land[Char.curr_row + 1];
	Char.alive = -1;
	Char.charid = charid_0_kid;
	is_screaming = 0;
	knock = 0;
	upside_down = custom->start_upside_down; // 0
	is_feather_fall = 0;
	Char.fall_y = 0;
	Char.fall_x = 0;
	offguard = 0;
	Char.sword = sword_0_sheathed;
	droppedout = 0;
	play_seq();
	if (current_level == /*7*/ custom->falling_entry_level && Char.room == /*17*/ custom->falling_entry_room) {
		// Special event: level 7 falling entry
		// level 7, room 17: show room below
		goto_other_room(3);
	}
	savekid();
}

// seg003:02E6
void __pascal far find_start_level_door() {
	short tilepos;
	get_room_address(Kid.room);
	for (tilepos = 0; tilepos < 30; ++tilepos) {
		if ((curr_room_tiles[tilepos] & 0x1F) == tiles_16_level_door_left) {
			start_level_door(Kid.room, tilepos);
		}
	}
}

// seg003:0326
void __pascal far draw_level_first() {
	next_room = Kid.room;
	check_the_end();
	if (custom->tbl_level_type[current_level]) {
		gen_palace_wall_colors();
	}
	draw_rect(&screen_rect, 0);
	show_level();
	redraw_screen(0);
	draw_kid_hp(hitp_curr, hitp_max);

#ifdef USE_QUICKSAVE
	check_quick_op();
#endif

#ifdef USE_SCREENSHOT
	auto_screenshot();
#endif

	// Busy waiting!
	start_timer(timer_1, 5);
	do_simple_wait(1);
}

// seg003:037B
void __pascal far redraw_screen(int drawing_different_room) {
	//remove_flash();
	if (drawing_different_room) {
		draw_rect(&rect_top, 0);
	}

	different_room = 0;
	if (is_blind_mode) {
		draw_rect(&rect_top, 0);
	} else {
		if (curr_guard_color) {
			// Moved *before* drawings.
			set_chtab_palette(chtab_addrs[id_chtab_5_guard], &guard_palettes[0x30 * curr_guard_color - 0x30], 0x10);
		}
		need_drects = 0;
		redraw_room();
#ifdef USE_LIGHTING
	redraw_lighting();
#endif
		if (is_keyboard_mode) {
			clear_kbd_buf();
		}
		is_blind_mode = 1;
		draw_tables();
		if (is_keyboard_mode) {
			clear_kbd_buf();
		}
#ifdef USE_COPYPROT
		if (current_level == 15) {
			// letters on potions level
			current_target_surface = offscreen_surface;
			short var_2;
			for (var_2 = 0; var_2 < 14; ++var_2) {
				if (copyprot_room[var_2] == drawn_room) {
					set_curr_pos((copyprot_tile[var_2] % 10 << 5) + 24, copyprot_tile[var_2] / 10 * 63 + 38);
					draw_text_character(copyprot_letter[cplevel_entr[var_2]]);
				}
			}
			current_target_surface = onscreen_surface_;
		}
#endif
		is_blind_mode = 0;
		memset_near(table_counts, 0, sizeof(table_counts));
		draw_moving();
		draw_tables();
		if (is_keyboard_mode) {
			clear_kbd_buf();
		}
		need_drects = 1;
		if (curr_guard_color) {
			//set_pal_arr(0x80, 0x10, &guard_palettes[0x30 * curr_guard_color - 0x30], 1);
		}
		if (upside_down) {
			flip_screen(offscreen_surface);
		}
		copy_screen_rect(&rect_top);
		if (upside_down) {
			flip_screen(offscreen_surface);
		}
		if (is_keyboard_mode) {
			clear_kbd_buf();
		}
	}
	exit_room_timer = 2;

}

#ifdef CHECK_TIMING
typedef struct test_timing_state_type {
	bool already_had_first_frame;
	Uint64 level_start_counter;
	int ticks_left_at_level_start;
	float seconds_left_at_level_start;
} test_timing_state_type;

void test_timings(test_timing_state_type* state) {

	if (!state->already_had_first_frame) {
		state->level_start_counter = SDL_GetPerformanceCounter();
		state->ticks_left_at_level_start = (rem_min-1)*720 + rem_tick;
		state->seconds_left_at_level_start = (1.0f / 60.0f) * (5 * state->ticks_left_at_level_start);
		printf("Seconds left = %f\n", state->seconds_left_at_level_start);
		state->already_had_first_frame = true;
	} else if (rem_tick % 12 == 11) {
		Uint64 current_counter = SDL_GetPerformanceCounter();
		float actual_seconds_elapsed = (float)(current_counter - state->level_start_counter) / (float)SDL_GetPerformanceFrequency();

		int ticks_left = (rem_min-1)*720 + rem_tick;
		float game_seconds_left = (1.0f / 60.0f) * (5 * ticks_left);
		float game_seconds_elapsed = state->seconds_left_at_level_start - game_seconds_left;

		printf("rem_min: %d   game elapsed (s): %.2f    actual elapsed (s): %.2f     delta: %.2f\n",
		       rem_min, game_seconds_elapsed, actual_seconds_elapsed, actual_seconds_elapsed - game_seconds_elapsed);
	}

}
#endif

// seg003:04F8
// Returns a level number:
// - The current level if it was restarted.
// - The next level if the level was completed.
int __pascal far play_level_2() {
	reset_timer(timer_1);
#ifdef CHECK_TIMING
	test_timing_state_type test_timing_state = {0};
#endif
	while (1) { // main loop
#ifdef USE_QUICKSAVE
		check_quick_op();
#endif
#ifdef CHECK_TIMING
		test_timings(&test_timing_state);
#endif

#ifdef USE_REPLAY
		if (need_replay_cycle) replay_cycle();
#endif
		if (Kid.sword == sword_2_drawn) {
			// speed when fighting (smaller is faster)
			set_timer_length(timer_1, /*6*/ custom->fight_speed);
		} else {
			// speed when not fighting (smaller is faster)
			set_timer_length(timer_1, /*5*/ custom->base_speed);
		}
		guardhp_delta = 0;
		hitp_delta = 0;
		timers();
		play_frame();

#ifdef USE_REPLAY
		// At the exact "end of level" frame, preserve the seed to ensure reproducibility,
		// regardless of how long the sound is still playing *after* this frame (torch animation modifies the seed!)
		if (keep_last_seed == 1) {
			preserved_seed = random_seed;
			keep_last_seed = -1; // disable repeat
		}
#endif

		if (is_restart_level) {
			is_restart_level = 0;
			return current_level;
		} else {
			if (next_level == current_level || check_sound_playing()) {
				draw_game_frame();
				flash_if_hurt();
				remove_flash_if_hurt();
				do_simple_wait(timer_1);
			} else {
				stop_sounds();
				hitp_beg_lev = hitp_max;
				checkpoint = 0;

				#ifdef USE_REPLAY
				if (keep_last_seed == -1) {
					random_seed = preserved_seed; // Ensure reproducibility in the next level.
					keep_last_seed = 0;
				}
				#endif

				return next_level;
			}
		}
	}
}

// seg003:0576
void __pascal far redraw_at_char() {
	short x_top_row;
	short tile_col;
	short tile_row;
	short x_col_left;
	short x_col_right;
	if (Char.sword >= sword_2_drawn) {
		// If char is holding sword, it makes redraw-area bigger.
		if (Char.direction >= dir_0_right) {
			if (++char_col_right > 9) char_col_right = 9;
			// char_col_right = MIN(char_col_right + 1, 9);
		} else {
			if (--char_col_left < 0) char_col_left = 0;
			// char_col_left = MAX(char_col_left - 1, 0);
		}
	}
	if (Char.charid == charid_0_kid) {
		x_top_row = MIN(char_top_row, prev_char_top_row);
		x_col_right = MAX(char_col_right, prev_char_col_right);
		x_col_left = MIN(char_col_left, prev_char_col_left);
	} else {
		x_top_row = char_top_row;
		x_col_right = char_col_right;
		x_col_left = char_col_left;
	}
	for (tile_row = x_top_row; tile_row <= char_bottom_row; ++tile_row) {
		for (tile_col = x_col_left; tile_col <= x_col_right; ++tile_col) {
			set_redraw_fore(get_tilepos(tile_col, tile_row), 1);
		}
	}
	if (Char.charid == charid_0_kid) {
		prev_char_top_row = char_top_row;
		prev_char_col_right = char_col_right;
		prev_char_col_left = char_col_left;
	}
}

// seg003:0645
void __pascal far redraw_at_char2() {
	short char_action;
	short char_frame;
	void __pascal (* redraw_func)(short, byte);
	char_action = Char.action;
	char_frame = Char.frame;
	redraw_func = &set_redraw2;
	// frames 78..80: grab
	if (char_frame < frame_78_jumphang || char_frame >= frame_80_jumphang) {
		// frames 135..149: climb up
		if (char_frame >= frame_137_climbing_3 && char_frame < frame_145_climbing_11) {
			redraw_func = &set_redraw_floor_overlay;
		} else {
			// frames 102..106: fall
			if (char_action != actions_2_hang_climb && char_action != actions_3_in_midair &&
					char_action != actions_4_in_freefall && char_action != actions_6_hang_straight &&
					(char_action != actions_5_bumped || char_frame < frame_102_start_fall_1 || char_frame > frame_106_fall)) {
				return;
			}
		}
	}
	for (tile_col = char_col_right; tile_col >= char_col_left; --tile_col) {
		if (char_action != 2) {
			redraw_func(get_tilepos(tile_col, char_bottom_row), 1);
		}
		if (char_top_row != char_bottom_row) {
			redraw_func(get_tilepos(tile_col, char_top_row), 1);
		}
	}
}

// seg003:0706
void __pascal far check_knock() {
	if (knock) {
		do_knock(Char.room, Char.curr_row - (knock>0));
		knock = 0;
	}
}

// seg003:0735
void __pascal far timers() {
	if (united_with_shadow > 0) {
		--united_with_shadow;
		if (united_with_shadow == 0) {
			--united_with_shadow;
		}
	}
	if (guard_notice_timer > 0) {
		--guard_notice_timer;
	}
	if (resurrect_time > 0) {
		--resurrect_time;
	}

	if (fixes->fix_quicksave_during_feather) {
		if (is_feather_fall > 0) {
			--is_feather_fall;
			if (is_feather_fall == 0) {
				if (check_sound_playing()) {
					stop_sounds();
				}
	#ifdef USE_SUPER_HIGH_JUMP
				if (fixes->enable_super_high_jump) {
					super_jump_fall = 0;
				}
	#endif

				//printf("slow fall ended at: rem_min = %d, rem_tick = %d\n", rem_min, rem_tick);
				//printf("length = %d ticks\n", is_feather_fall);
	#ifdef USE_REPLAY
				if (recording) special_move = MOVE_EFFECT_END;
	#endif
			}
		}
	} else {
		if (is_feather_fall) is_feather_fall++;

		if (is_feather_fall && (!check_sound_playing() || is_feather_fall > 225)) {
			//printf("slow fall ended at: rem_min = %d, rem_tick = %d\n", rem_min, rem_tick);
			//printf("length = %d ticks\n", is_feather_fall);
	#ifdef USE_REPLAY
			if (recording) special_move = MOVE_EFFECT_END;
			if (!replaying) // during replays, feather effect gets cancelled in do_replay_move()
	#endif
			is_feather_fall = 0;
	#ifdef USE_SUPER_HIGH_JUMP
			if (fixes->enable_super_high_jump) {
				super_jump_fall = 0;
			}
	#endif
		}
	}

	// Special event: mouse
	if (current_level == /*8*/ custom->mouse_level && Char.room == /*16*/ custom->mouse_room && leveldoor_open) {
		++leveldoor_open;
		// time before mouse comes: 150/12=12.5 seconds
		if (leveldoor_open == /*150*/ custom->mouse_delay) {
			do_mouse();
		}
	}
#ifdef USE_SUPER_HIGH_JUMP
	if (fixes->enable_super_high_jump && super_jump_timer > 0) {
		--super_jump_timer;
		if (super_jump_timer == 0 && Kid.frame == frame_79_jumphang) {
		    if (get_tile(super_jump_room, super_jump_col, super_jump_row) == tiles_11_loose &&
		            (curr_room_tiles[curr_tilepos] & 0x20) == 0) {
		        make_loose_fall(1); // knocks the true loose tile above
		        do_knock(super_jump_room, super_jump_row); // shakes the rest of loose tiles
		    } else if (curr_tile2 == tiles_20_wall || tile_is_floor(curr_tile2)) {
		        if (super_jump_row < 2) {
		            Kid.curr_row = super_jump_row + 1;
		            Kid.y = y_land[super_jump_row + 2] + 10;
		        }
		        do_knock(super_jump_room, super_jump_row); // shakes loose tiles in the row
		    } else if (!tile_is_floor(curr_tile2)) {
		        if (super_jump_row == 2) {
		            Kid.room = level.roomlinks[Kid.room - 1].up;
		        }
		        if (Kid.room != 0) { // there is a room above
					// positions kid for grabbing
		            Kid.curr_row = super_jump_row + 1;
		            Kid.y = y_land[super_jump_row + 2] - 10;
		            Kid.fall_x = 0;
		            Kid.fall_y = 0;
		            super_jump_fall = 1;
		            // gives kid an ability to grab the above front tile
		            seqtbl_offset_kid_char(seq_19_fall);
		            play_seq();
		        }
		    }
		}
	}
#endif
}

// seg003:0798
void __pascal far check_mirror() {
	word clip_top;
	if (jumped_through_mirror == -1) {
		jump_through_mirror();
	} else {
		if (get_tile_at_char() == tiles_13_mirror) {
			loadkid();
			load_frame();
			check_mirror_image();
			if (distance_mirror >= 0 && custom->show_mirror_image && Char.room == drawn_room) {
				load_frame_to_obj();
				reset_obj_clip();
				clip_top = y_clip[Char.curr_row + 1];
				if (clip_top < obj_y) {
					obj_clip_top = clip_top;
					obj_clip_left = (Char.curr_col << 5) + 9;
					add_objtable(4); // mirror image
				}
			}
		}
	}
}

// seg003:080A
void __pascal far jump_through_mirror() {
	loadkid();
	load_frame();
	check_mirror_image();
	jumped_through_mirror = 0;
	Char.charid = charid_1_shadow;
	play_sound(sound_45_jump_through_mirror); // jump through mirror
	saveshad();
	guardhp_max = guardhp_curr = hitp_max;
	hitp_curr = 1;
	draw_kid_hp(1, hitp_max);
	draw_guard_hp(guardhp_curr, guardhp_max);
}

// seg003:085B
void __pascal far check_mirror_image() {
	short distance;
	short xpos;
	xpos = x_bump[Char.curr_col + 5] + 10;
	distance = distance_to_edge_weight();
	if (Char.direction >= dir_0_right) {
		distance = (~distance) + 14;
	}
	distance_mirror = distance - 2;
	Char.x = (xpos << 1) - Char.x;
	Char.direction = ~Char.direction;
}

// seg003:08AA
void __pascal far bump_into_opponent() {
	// This is called from play_kid_frame, so char=Kid, Opp=Guard
	short distance;
	if (can_guard_see_kid >= 2 &&
		Char.sword == sword_0_sheathed && // Kid must not be in fighting pose
		Opp.sword != sword_0_sheathed && // but Guard must
		Opp.action < 2 &&
		Char.direction != Opp.direction // must be facing toward each other
	) {
		distance = char_opp_dist();
		if (ABS(distance) <= 15) {

			#ifdef FIX_PAINLESS_FALL_ON_GUARD
			if (fixes->fix_painless_fall_on_guard) {
				if (Char.fall_y >= 33) return; // don't bump; dead
				else if (Char.fall_y >= 22) { // medium land
					take_hp(1);
					play_sound(sound_16_medium_land);
				}
			}
			#endif

			Char.y = y_land[Char.curr_row + 1];
			Char.fall_y = 0;
			seqtbl_offset_char(seq_47_bump); // bump into opponent
			play_seq();
		}
	}
}

// seg003:0913
void __pascal far pos_guards() {
	short guard_tile;
	short room1;
	for (room1 = 0; room1 < 24; ++room1) {
		guard_tile = level.guards_tile[room1];
		if (guard_tile < 30) {
			level.guards_x[room1] = x_bump[guard_tile % 10 + 5] + 14;
			level.guards_seq_hi[room1] = 0;
		}
	}
}

// seg003:0959
void __pascal far check_can_guard_see_kid() {
/*
Possible results in can_guard_see_kid:
0: Guard can't see Kid
1: Guard can see Kid, but won't come
2: Guard can see Kid, and will come
*/
	short kid_frame;
	short left_pos;
	short temp;
	short right_pos;
	kid_frame = Kid.frame;
	if (Guard.charid == charid_24_mouse) {
		// If the prince is fighting a guard, and the player does a quickload to a state where the prince is near the mouse, the prince would draw the sword.
		// The following line prevents this.
		can_guard_see_kid = 0;
		return;
	}
	if ((Guard.charid != charid_1_shadow || current_level == 12) &&
		// frames 217..228: going up on stairs
		kid_frame != 0 && (kid_frame < frame_219_exit_stairs_3 || kid_frame >= 229) &&
		Guard.direction != dir_56_none && Kid.alive < 0 && Guard.alive < 0 && Kid.room == Guard.room && Kid.curr_row == Guard.curr_row
	) {
		can_guard_see_kid = 2;
		left_pos = x_bump[Kid.curr_col + 5] + 7;
#ifdef FIX_DOORTOP_DISABLING_GUARD
		if (fixes->fix_doortop_disabling_guard) {
			// When the kid is hanging on the right side of a doortop, Kid.curr_col points at the doortop tile and a guard on the left side will see the prince.
			// This fixes that.
			if (Kid.action == actions_2_hang_climb || Kid.action == actions_6_hang_straight) left_pos += 14;
		}
#endif
		//printf("Kid.curr_col = %d, Kid.action = %d\n", Kid.curr_col, Kid.action);
		right_pos = x_bump[Guard.curr_col + 5] + 7;
		if (left_pos > right_pos) {
			temp = left_pos;
			left_pos = right_pos;
			right_pos = temp;
		}
		// A chomper is on the left side of a tile, so it doesn't count.
		if (get_tile_at_kid(left_pos) == tiles_18_chomper) {
			left_pos += 14;
		}
		// A gate is on the right side of a tile, so it doesn't count.
		if (get_tile_at_kid(right_pos) == tiles_4_gate
#ifdef FIX_DOORTOP_DISABLING_GUARD
			|| (fixes->fix_doortop_disabling_guard && (get_tile_at_kid(right_pos) == tiles_7_doortop_with_floor || get_tile_at_kid(right_pos) == tiles_12_doortop))
#endif
		) {
			right_pos -= 14;
		}
		if (right_pos >= left_pos) {
			while (left_pos <= right_pos) {
				// Can't see through these tiles.
				if (get_tile_at_kid(left_pos) == tiles_20_wall ||
					curr_tile2 == tiles_7_doortop_with_floor ||
					curr_tile2 == tiles_12_doortop
				) {
					can_guard_see_kid = 0; return;
				}
				// Can see through these, but won't go through them.
				if (curr_tile2 == tiles_11_loose ||
					curr_tile2 == tiles_18_chomper ||
					(curr_tile2 == tiles_4_gate && curr_room_modif[curr_tilepos] < 112) ||
					!tile_is_floor(curr_tile2)
				) {
					can_guard_see_kid = 1;
				}
				left_pos += 14;
			}
		}
	} else {
		can_guard_see_kid = 0;
	}
}

// seg003:0A99
byte __pascal far get_tile_at_kid(int xpos) {
	return get_tile(Kid.room, get_tile_div_mod_m7(xpos), Kid.curr_row);
}

// seg003:0ABA
void __pascal far do_mouse() {
	loadkid();
	Char.charid = /*charid_24_mouse*/ custom->mouse_object;
	Char.x = /*200*/ custom->mouse_start_x;
	Char.curr_row = 0;
	Char.y = y_land[Char.curr_row + 1];
	Char.alive = -1;
	Char.direction = dir_FF_left;
	guardhp_curr = 1;
	seqtbl_offset_char(seq_105_mouse_forward); // mouse forward
	play_seq();
	saveshad();
}

// seg003:0AFC
int __pascal far flash_if_hurt() {
	if (flash_time != 0) {
		do_flash(flash_color);
		return 1;
	} else if (hitp_delta < 0) {
		if (is_joyst_mode && enable_controller_rumble) {
			if (sdl_haptic != NULL) {
				SDL_HapticRumblePlay(sdl_haptic, 1.0, 100); // rumble at full strength for 100 milliseconds
#if SDL_VERSION_ATLEAST(2,0,9)
			} else if (sdl_controller_ != NULL) {
				SDL_GameControllerRumble(sdl_controller_, 0xFFFF, 0xFFFF, 100);
			} else {
				SDL_JoystickRumble(sdl_joystick_, 0xFFFF, 0xFFFF, 100);
#endif
			}
		}
		do_flash(color_12_brightred); // red
		return 1;
	}
	return 0; // not flashed
}

// seg003:0B1A
void __pascal far remove_flash_if_hurt() {
	if (flash_time != 0) {
		--flash_time;
	} else {
		if (hitp_delta >= 0) return;
	}
	remove_flash();
}
