/*
SDLPoP, a port/conversion of the DOS game Prince of Persia.
Copyright (C) 2013-2018  Dávid Nagy

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
		rem_min = start_minutes_left;   // 60
		rem_tick = start_ticks_left;    // 719
		hitp_beg_lev = start_hitp;      // 3
	}
	need_level1_music = (level == 1);
	play_level(level);
}

// seg003:005C
void __pascal far play_level(int level_number) {
	cutscene_ptr_type cutscene_func;
#ifdef USE_COPYPROT
	if (enable_copyprot && level_number == copyprot_level) {
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
			cutscene_func = tbl_cutscenes[level_number];
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
		have_sword = (level_number != 1);
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
		if (enable_copyprot && level_number == copyprot_level && !demo_mode) {
			level_number = 15;
		} else {
			if (level_number == 16) {
				level_number = copyprot_level;
				copyprot_level = -1;
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
	if (current_level == 3 && checkpoint) {
		level.start_dir = dir_FF_left;
		level.start_room = 2;
		level.start_pos = 6;
		// Special event: remove loose floor
		get_tile(7, 4, 0);
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
			x = 4;
		}
		hitp_max = hitp_curr = x;
	}
	if (current_level == 1) {
		// Special event: press button + falling entry
		get_tile(5, 2, 0);
		trigger_button(0, 0, -1);
		seqtbl_offset_char(seq_7_fall); // fall
	} else if (current_level == 13) {
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
	upside_down = start_upside_down; // 0
	is_feather_fall = 0;
	Char.fall_y = 0;
	Char.fall_x = 0;
	offguard = 0;
	Char.sword = sword_0_sheathed;
	droppedout = 0;
	play_seq();
	if (current_level == 7 && Char.room == 17) {
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
	screen_updates_suspended = 1;

	next_room = Kid.room;
	check_the_end();
	if (tbl_level_type[current_level]) {
		gen_palace_wall_colors();
	}
	draw_rect(&screen_rect, 0);
	show_level();
	redraw_screen(0);
	draw_kid_hp(hitp_curr, hitp_max);

	screen_updates_suspended = 0;
	request_screen_update();

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

// seg003:04F8
// Returns a level number:
// - The current level if it was restarted.
// - The next level if the level was completed.
int __pascal far play_level_2() {
	while (1) { // main loop
#ifdef USE_QUICKSAVE
		check_quick_op();
#endif

#ifdef USE_REPLAY
		if (need_replay_cycle) replay_cycle();
#endif

		if (Kid.sword == sword_2_drawn) {
			// speed when fighting (smaller is faster)
			start_timer(timer_1, 6);
		} else {
			// speed when not fighting (smaller is faster)
			start_timer(timer_1, 5);
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
				screen_updates_suspended = 1;
				draw_game_frame();
				if (flash_if_hurt()) {
					screen_updates_suspended = 0;
					request_screen_update(); // display the flash
					delay_ticks(2);          // and add a short delay
				}
				screen_updates_suspended = 0;
				remove_flash_if_hurt();

				#ifdef USE_DEBUG_CHEATS
				if (debug_cheats_enabled && is_timer_displayed) {
					char timer_text[16];
					if (rem_min < 0) {
						snprintf(timer_text, 16, "%02d:%02d:%02d", -(rem_min + 1), (719 - rem_tick) / 12, (719 - rem_tick) % 12);
					} else {
						snprintf(timer_text, 16, "%02d:%02d:%02d", rem_min - 1, rem_tick / 12, rem_tick % 12);
					}
					screen_updates_suspended = 1;
					draw_rect(&timer_rect, color_0_black);
					show_text(&timer_rect, -1, -1, timer_text);
					screen_updates_suspended = 0;
				}
				#endif

				request_screen_update(); // request screen update manually
				do_simple_wait(1);
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
	if (is_feather_fall && !check_sound_playing()) {
#ifdef USE_REPLAY
		if (recording) special_move = MOVE_EFFECT_END;
		if (!replaying) // during replays, feather effect gets cancelled in do_replay_move()
#endif
		is_feather_fall = 0;
	}
	// Special event: mouse
	if (current_level == 8 && Char.room == 16 && leveldoor_open) {
		++leveldoor_open;
		// time before mouse comes: 150/12=12.5 seconds
		if (leveldoor_open == 150) {
			do_mouse();
		}
	}
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
			if (distance_mirror >= 0) {
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
			if (fix_painless_fall_on_guard) {
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
		return;
	}
	if ((Guard.charid != charid_1_shadow || current_level == 12) &&
		// frames 217..228: going up on stairs
		kid_frame != 0 && (kid_frame < frame_219_exit_stairs_3 || kid_frame >= 229) &&
		Guard.direction != dir_56_none && Kid.alive < 0 && Guard.alive < 0 && Kid.room == Guard.room && Kid.curr_row == Guard.curr_row
	) {
		can_guard_see_kid = 2;
		left_pos = x_bump[Kid.curr_col + 5] + 7;
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
		if (get_tile_at_kid(right_pos) == tiles_4_gate) {
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
	Char.charid = charid_24_mouse;
	Char.x = 200;
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
		do_flash_no_delay(flash_color); // don't add delay to the flash
		return 1;
	} else if (hitp_delta < 0) {
		if (is_joyst_mode && enable_controller_rumble && sdl_haptic != NULL) {
			SDL_HapticRumblePlay(sdl_haptic, 1.0, 100); // rumble at full strength for 100 milliseconds
		}
		do_flash_no_delay(color_12_brightred); // red
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
