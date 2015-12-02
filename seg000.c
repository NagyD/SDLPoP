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
#include <fcntl.h>
#include <setjmp.h>

// data:009C
word cheats_enabled = 0;

// data:461E
dat_type * dathandle;

// data:4CE2
word need_full_redraw;
// data:4C08
word need_redraw_because_flipped;

// seg000:0000
void far pop_main() {
	// debug only: check that the sequence table deobfuscation did not mess things up
	#ifdef CHECK_SEQTABLE_MATCHES_ORIGINAL
	check_seqtable_matches_original();
	#endif

	load_options();
	apply_seqtbl_patches();

	char sprintf_temp[100];
	int i;

	dathandle = open_dat("PRINCE.DAT", 0);

	/*video_mode =*/ parse_grmode();

	init_timer(60);
	parse_cmdline_sound();

	set_hc_pal();

	current_target_surface = rect_sthg(onscreen_surface_, &screen_rect);
	show_loading();
	set_joy_mode();
	cheats_enabled = check_param("megahit") != 0;
#ifdef __DEBUG__
	cheats_enabled = 1; // debug
#endif
	draw_mode = check_param("draw") && cheats_enabled;
	demo_mode = check_param("demo");

#ifdef USE_REPLAY
	init_record_replay();
#endif

	if (cheats_enabled
		#ifdef USE_REPLAY
		|| recording
        #endif
			) {
		for (i = 14; i >= 0; --i) {
			snprintf(sprintf_temp, sizeof(sprintf_temp), "%d", i);
			if (check_param(sprintf_temp)) {
				start_level = i;
				break;
			}
		}
	}

	init_game_main();
}

// seg000:024F
void __pascal far init_game_main() {
	doorlink1_ad = /*&*/level.doorlinks1;
	doorlink2_ad = /*&*/level.doorlinks2;
	prandom(1);
	if (graphics_mode == gmMcgaVga) {
		// Guard palettes
		guard_palettes = (byte*) load_from_opendats_alloc(10, "bin", NULL, NULL);
		// (blood, hurt flash) #E00030 = red
		set_pal(12, 0x38, 0x00, 0x0C, 1);
		// (palace wall pattern) #C09850 = light brown
		set_pal( 6, 0x30, 0x26, 0x14, 0);
	}
	// PRINCE.DAT: sword
	chtab_addrs[id_chtab_0_sword] = load_sprites_from_file(700, 1<<2, 1);
	// PRINCE.DAT: flame, sword on floor, potion
	chtab_addrs[id_chtab_1_flameswordpotion] = load_sprites_from_file(150, 1<<3, 1);
	close_dat(dathandle);
	load_sounds(0, 43);
	load_opt_sounds(43, 56); //added
	hof_read();
	show_disable_fixes_prompt(); // added
	start_game();
}


// data:02C2
word first_start = 1;
// data:4C38
jmp_buf setjmp_buf;
// seg000:0358
void __pascal far start_game() {
#ifdef USE_COPYPROT
	word which_entry;
	word pos;
	word entry_used[40];
	byte letts_used[26];
#endif
	screen_updates_suspended = 0;
	// Prevent filling of stack.
	// start_game is called from many places to restart the game, for example:
	// process_key, play_frame, draw_game_frame, play_level, control_kid, end_sequence, expired
	if (first_start) {
		first_start = 0;
		setjmp(/*&*/setjmp_buf);
	} else {
		draw_rect(&screen_rect, 0);
		show_quotes();
		clear_screen_and_sounds();
		longjmp(/*&*/setjmp_buf,-1);
	}
	release_title_images(); // added
	free_optsnd_chtab(); // added
#ifdef USE_COPYPROT
	copyprot_plac = prandom(13);
	memset(&entry_used, 0, sizeof(entry_used));
	memset(&letts_used, 0, sizeof(letts_used));
	for (pos = 0; pos < 14; ++pos) {
		do {
			if (pos == copyprot_plac) {
				which_entry = copyprot_idx = prandom(39);
			} else {
				which_entry = prandom(39);
			}
		} while (entry_used[which_entry] || letts_used[copyprot_letter[which_entry]-'A']);
		cplevel_entr[pos] = which_entry;
		entry_used[which_entry] = 1;
		letts_used[copyprot_letter[which_entry]-'A'] = 1;
	}
#endif
	if (start_level == 0) {
		show_title();
	} else {
		init_game(start_level);
	}
}

#ifdef USE_QUICKSAVE
// All these functions return true on success, false otherwise.

FILE* quick_fp;

int process_save(void* data, size_t data_size) {
	return fwrite(data, data_size, 1, quick_fp) == 1;
}

int process_load(void* data, size_t data_size) {
	return fread(data, data_size, 1, quick_fp) == 1;
}

typedef int process_func_type(void* data, size_t data_size);

int quick_process(process_func_type process_func) {
	int ok = 1;
#define process(x) ok = ok && process_func(&(x), sizeof(x))
	// level
	process(level);
	process(checkpoint);
	process(upside_down);
	process(drawn_room);
	process(current_level);
	process(next_level);
	process(mobs_count);
	process(mobs);
	process(trobs_count);
	process(trobs);
	process(leveldoor_open);
	//process(exit_room_timer);
	// kid
	process(Kid);
	process(hitp_curr);
	process(hitp_max);
	process(hitp_beg_lev);
	process(grab_timer);
	process(holding_sword);
	process(united_with_shadow);
	process(have_sword);
	/*process(ctrl1_forward);
	process(ctrl1_backward);
	process(ctrl1_up);
	process(ctrl1_down);
	process(ctrl1_shift2);*/
	process(kid_sword_strike);
	process(pickup_obj_type);
	process(word_1EFCE);
	// guard
	process(Guard);
	process(guardhp_curr);
	process(guardhp_max);
	process(demo_index);
	process(demo_time);
	process(curr_guard_color);
	process(guard_notice_timer);
	process(guard_skill);
	process(shadow_initialized);
	process(guard_refrac);
	process(word_1E1AA);
	process(word_1EA12);
	// collision
	/*process(curr_row_coll_room);
	process(curr_row_coll_flags);
	process(below_row_coll_room);
	process(below_row_coll_flags);
	process(above_row_coll_room);
	process(above_row_coll_flags);
	process(prev_collision_row);*/
	// flash
	process(flash_color);
	process(flash_time);
	// sounds
	process(need_level1_music);
	process(is_screaming);
	process(is_feather_fall);
	process(last_loose_sound);
	//process(next_sound);
	//process(current_sound);
	// random
	process(random_seed);
	// remaining time
	process(rem_min);
	process(rem_tick);
	// saved controls
	process(control_x);
	process(control_y);
	process(control_shift);
	process(control_forward);
	process(control_backward);
	process(control_up);
	process(control_down);
	process(control_shift2);
	process(ctrl1_forward);
	process(ctrl1_backward);
	process(ctrl1_up);
	process(ctrl1_down);
	process(ctrl1_shift2);
#undef process
	return ok;
}

const char* const quick_file = "QUICKSAVE.SAV";
const char const quick_version[] = "V1.16b3 ";
char quick_control[] = "........";

int quick_save() {
	int ok = 0;
	quick_fp = fopen(quick_file, "wb");
	if (quick_fp != NULL) {
		process_save((void*) quick_version, COUNT(quick_version));
		ok = quick_process(process_save);
		fclose(quick_fp);
		quick_fp = NULL;
	}
	return ok;
}

void restore_room_after_quick_load() {
	int temp1 = curr_guard_color;
	int temp2 = next_level;
	load_lev_spr(current_level);
	curr_guard_color = temp1;
	next_level = temp2;

	//need_full_redraw = 1;
	different_room = 1;
	next_room = drawn_room;
	load_room_links();
	//draw_level_first();
	//gen_palace_wall_colors();
	draw_game_frame(); // for falling
	//redraw_screen(1); // for room_L

	hitp_delta = guardhp_delta = 1; // force HP redraw
	draw_hp();
	loadkid_and_opp();
	// Get rid of "press button" message if kid was dead before quickload.
	text_time_total = text_time_remaining = 0;
	//next_sound = current_sound = -1;
	exit_room_timer = 0;
}

int quick_load() {

	int ok = 0;
	quick_fp = fopen(quick_file, "rb");
	if (quick_fp != NULL) {
		// check quicksave version is compatible
		process_load(quick_control, COUNT(quick_control));
		if (strcmp(quick_control, quick_version) != 0) {
			fclose(quick_fp);
			quick_fp = NULL;
			return 0;
		}

		stop_sounds();
		start_timer(timer_0, 5); // briefly display a black screen as a visual cue
		draw_rect(&screen_rect, 0);
		screen_updates_suspended = 0;
		request_screen_update();
		screen_updates_suspended = 1;

		ok = quick_process(process_load);
		fclose(quick_fp);
		quick_fp = NULL;

		restore_room_after_quick_load();

		do_wait(timer_0);
		screen_updates_suspended = 0;
		request_screen_update();

		#ifdef USE_QUICKLOAD_PENALTY
		// Subtract one minute from the remaining time (if it is above 5 minutes)
		if (options.enable_quicksave_penalty) {
			if (rem_min == 6) rem_tick = 719; // crop to "5 minutes" exactly, if hitting the threshold in <1 minute
			if (rem_min > 5) --rem_min;
		}
		#endif
	}
	return ok;
}

int need_quick_save = 0;
int need_quick_load = 0;

void check_quick_op() {
	if (!options.enable_quicksave) return;
	if (need_quick_save) {
		if (!is_feather_fall && quick_save()) {
			display_text_bottom("QUICKSAVE");
		} else {
			display_text_bottom("NO QUICKSAVE");
		}
		need_quick_save = 0;
		text_time_total = 24;
		text_time_remaining = 24;
	}
	if (need_quick_load) {
		if (quick_load()) {
			display_text_bottom("QUICKLOAD");
		} else {
			display_text_bottom("NO QUICKLOAD");
		}
		need_quick_load = 0;
		text_time_total = 24;
		text_time_remaining = 24;
	}
}


#endif // USE_QUICKSAVE

Uint32 temp_shift_release_callback(Uint32 interval, void *param) {
	const Uint8* state = SDL_GetKeyboardState(NULL);
	if (state[SDL_SCANCODE_LSHIFT]) key_states[SDL_SCANCODE_LSHIFT] = 1;
	if (state[SDL_SCANCODE_RSHIFT]) key_states[SDL_SCANCODE_RSHIFT] = 1;
	return 0; // causes the timer to be removed
}

// seg000:04CD
int __pascal far process_key() {
	char sprintf_temp[80];
	int key;
	const char* answer_text;
	word need_show_text;
	need_show_text = 0;
	key = key_test_quit();

	if (start_level == 0) {
		if (key || control_shift) {
			#ifdef USE_QUICKSAVE
			if (key == SDL_SCANCODE_F9) need_quick_load = 1;
			#endif
			#ifdef USE_REPLAY
			if (key == SDL_SCANCODE_TAB) {
				start_replay();
			}
			else
			#endif
			if (key == (SDL_SCANCODE_L | WITH_CTRL)) { // ctrl-L
				if (!load_game()) return 0;
			} else {
				start_level = 1;
			}
			draw_rect(&screen_rect, 0);
#ifdef USE_FADE
			if (is_global_fading) {
				fade_palette_buffer->proc_restore_free(fade_palette_buffer);
				is_global_fading = 0;
			}
#endif
			start_game();
		}
	}
	// If the Kid died, enter or shift will restart the level.
	if (rem_min != 0 && Kid.alive > 6 && (control_shift || key == SDL_SCANCODE_RETURN)) {
		key = SDL_SCANCODE_A | WITH_CTRL; // ctrl-a
	}
#ifdef USE_REPLAY
	if (recording) key_press_while_recording(&key);
	else if (replaying) key_press_while_replaying(&key);
#endif
	if (key == 0) return 0;
	if (is_keyboard_mode) clear_kbd_buf();

	switch(key) {
		case SDL_SCANCODE_ESCAPE: // esc
		case SDL_SCANCODE_ESCAPE | WITH_SHIFT: // allow pause while grabbing
			is_paused = 1;
		break;
		case SDL_SCANCODE_SPACE: // space
			is_show_time = 1;
		break;
		case SDL_SCANCODE_A | WITH_CTRL: // ctrl-a
			if (current_level != 15) {
				stop_sounds();
				is_restart_level = 1;
			}
		break;
		case SDL_SCANCODE_G | WITH_CTRL: // ctrl-g
			if (current_level > 2 && current_level < 14) {
				save_game();
			}
		break;
		case SDL_SCANCODE_J | WITH_CTRL: // ctrl-j
			if ((sound_flags & sfDigi) && sound_mode == smTandy) {
				answer_text = "JOYSTICK UNAVAILABLE";
			} else {
				if (set_joy_mode()) {
					answer_text = "JOYSTICK MODE";
				} else {
					answer_text = "JOYSTICK NOT FOUND";
				}
			}
			need_show_text = 1;
		break;
		case SDL_SCANCODE_K | WITH_CTRL: // ctrl-k
			answer_text = "KEYBOARD MODE";
			is_joyst_mode = 0;
			is_keyboard_mode = 1;
			need_show_text = 1;
		break;
		case SDL_SCANCODE_R | WITH_CTRL: // ctrl-r
			start_level = 0;
			start_game();
		break;
		case SDL_SCANCODE_S | WITH_CTRL: // ctrl-s
			turn_sound_on_off((!is_sound_on) * 15);
			answer_text = "SOUND OFF";
			if (is_sound_on) {
				answer_text = "SOUND ON";
			}
			//
			need_show_text = 1;
		break;
		case SDL_SCANCODE_V | WITH_CTRL: // ctrl-v
			answer_text = "PRINCE OF PERSIA  V1.0";
			need_show_text = 1;
		break;
		case SDL_SCANCODE_L | WITH_SHIFT: // shift-l
			if (current_level <= 3 || cheats_enabled) {
				// if shift is not released within the delay, the cutscene is skipped
				Uint32 delay = 250;
				key_states[SDL_SCANCODE_LSHIFT] = 0;
				key_states[SDL_SCANCODE_RSHIFT] = 0;
				SDL_TimerID timer;
				timer = SDL_AddTimer(delay, temp_shift_release_callback, NULL);
				if (timer == 0) {
					sdlperror("SDL_AddTimer");
					quit(1);
				}
				if (current_level == 14) {
					next_level = 1;
				} else {
					if (current_level == 15 && cheats_enabled) {
#ifdef USE_COPYPROT
                        if (options.enable_copyprot) {
                        	next_level = copyprot_level;
                        	copyprot_level = -1;
                        }
#endif
					} else {
						next_level = current_level + 1;
						if (!cheats_enabled && rem_min > 15) {
							rem_min = 15;
							rem_tick = 719;
						}
					}
				}
				stop_sounds();
			}
		break;
#ifdef USE_QUICKSAVE
		case SDL_SCANCODE_F6:
		case SDL_SCANCODE_F6 | WITH_SHIFT:
			if (Kid.alive < 0) need_quick_save = 1;
		break;
		case SDL_SCANCODE_F9:
		case SDL_SCANCODE_F9 | WITH_SHIFT:
			need_quick_load = 1;
		break;
#ifdef USE_REPLAY
		case SDL_SCANCODE_TAB | WITH_CTRL:
		case SDL_SCANCODE_TAB | WITH_CTRL | WITH_SHIFT:
			if (recording) { // finished recording
				stop_recording();
			}
			else { // should start recording
				start_recording();
			}
			break;
#endif // USE_RECORD_REPLAY
#endif // USE_QUICKSAVE
	}
	if (cheats_enabled) {
		switch (key) {
			case SDL_SCANCODE_C: // c
				snprintf(sprintf_temp, sizeof(sprintf_temp), "S%d L%d R%d A%d B%d", drawn_room, room_L, room_R, room_A, room_B);
				answer_text = /*&*/sprintf_temp;
				need_show_text = 1;
			break;
			case SDL_SCANCODE_C | WITH_SHIFT: // shift-c
				snprintf(sprintf_temp, sizeof(sprintf_temp), "AL%d AR%d BL%d BR%d", room_BR, room_BL, room_AR, room_AL);
				answer_text = /*&*/sprintf_temp;
				need_show_text = 1;
			break;
			case SDL_SCANCODE_MINUS:
			case SDL_SCANCODE_KP_MINUS:		// '-' --> subtract time cheat
				if (rem_min > 1) --rem_min;
				text_time_total = 0;
				text_time_remaining = 0;
				is_show_time = 1;
			break;
			case SDL_SCANCODE_EQUALS | WITH_SHIFT: // '+'
			case SDL_SCANCODE_KP_PLUS:	   // '+' --> add time cheat
				++rem_min;
				text_time_total = 0;
				text_time_remaining = 0;
				is_show_time = 1;
			break;
			case SDL_SCANCODE_R: // R --> revive kid cheat
				if (Kid.alive > 0) {
					resurrect_time = 20;
					Kid.alive = -1;
					erase_bottom_text(1);
				}
			break;
			case SDL_SCANCODE_K: // K --> kill guard cheat
				guardhp_delta = -guardhp_curr;
				Guard.alive = 0;
			break;
			case SDL_SCANCODE_I | WITH_SHIFT: // shift+I --> invert cheat
				toggle_upside();
			break;
			case SDL_SCANCODE_W | WITH_SHIFT: // shift+W --> feather fall cheat
				feather_fall();
			break;
			case SDL_SCANCODE_H: // H --> view room to the left
				draw_guard_hp(0, 10);
				next_room = room_L;
			break;
			case SDL_SCANCODE_J: // J --> view room to the right
				draw_guard_hp(0, 10);
				next_room = room_R;
			break;
			case SDL_SCANCODE_U: // U --> view room above
				draw_guard_hp(0, 10);
				next_room = room_A;
			break;
			case SDL_SCANCODE_N: // N --> view room below
				draw_guard_hp(0, 10);
				next_room = room_B;
			break;
			case SDL_SCANCODE_B | WITH_SHIFT: // shift-b
				is_blind_mode = !is_blind_mode;
				if (is_blind_mode) {
					draw_rect(&rect_top, 0);
				} else {
					need_full_redraw = 1;
				}
			break;
			case SDL_SCANCODE_S | WITH_SHIFT: // shift-s
				if (hitp_curr != hitp_max) {
					play_sound(sound_33_small_potion); // small potion (cheat)
					hitp_delta = 1;
					flash_color = 4; // red
					flash_time = 2;
				}
			break;
			case SDL_SCANCODE_T | WITH_SHIFT: // shift-t
				play_sound(sound_30_big_potion); // big potion (cheat)
				flash_color = 4; // red
				flash_time = 4;
				add_life();
			break;
			#ifdef USE_DEBUG_CHEATS
			case SDL_SCANCODE_T:
				printf("Remaining minutes: %d\tticks:%d\n", rem_min, rem_tick);
				snprintf(sprintf_temp, sizeof(sprintf_temp), "M:%d S:%d T:%d", rem_min, rem_tick / 12, rem_tick);
				answer_text = sprintf_temp;
				need_show_text = 1;
			break;
			#endif
		}
	}

	if (need_show_text) {
		display_text_bottom(answer_text);
		text_time_total = 24;
		text_time_remaining = 24;
	}
	return 1;
}

// seg000:08EB
void __pascal far play_frame() {
	do_mobs();
	process_trobs();
	check_skel();
	check_can_guard_see_kid();
	// if level is restarted, return immediately
	if (play_kid_frame()) return;
	play_guard_frame();
	if (0 == resurrect_time) {
		check_sword_hurting();
		check_sword_hurt();
	}
	check_sword_vs_sword();
	do_delta_hp();
	exit_room();
	check_the_end();
	check_guard_fallout();
	if (current_level == 0) {
		// Special event: level 0 running exit
		if (Kid.room == 24) {
			draw_rect(&screen_rect, 0);
			start_level = 0;
			word_1F05E = 1;
			start_game();
		}
	} else if(current_level == 6) {
		// Special event: level 6 falling exit
		if (roomleave_result == -2) {
			Kid.y = -1;
			stop_sounds();
			++next_level;
		}
	} else if(current_level == 12) {
		// Special event: level 12 running exit
		if (Kid.room == 23) {
			++next_level;
// Sounds must be stopped, because play_level_2() checks next_level only if there are no sounds playing. 
			stop_sounds();
			seamless = 1;
		}
	}
	show_time();
	// expiring doesn't count on Jaffar/princess level
	if (current_level < 13 && rem_min == 0) {
		expired();
	}
}

// seg000:09B6
void __pascal far draw_game_frame() {
	short var_2;
	if (need_full_redraw) {
		redraw_screen(0);
		need_full_redraw = 0;
	} else {
		if (different_room) {
			drawn_room = next_room;
			if (tbl_level_type[current_level]) {
				gen_palace_wall_colors();
			}
			redraw_screen(1);
		} else {
			if (need_redraw_because_flipped) {
				need_redraw_because_flipped = 0;
				redraw_screen(0);
			} else {
				memset_near(&table_counts, 0, sizeof(table_counts));
				draw_moving();
				draw_tables();
				if (is_blind_mode) {
					draw_rect(&rect_top, 0);
				}
				if (upside_down) {
					flip_screen(offscreen_surface);
				}
				while (drects_count--) {
					copy_screen_rect(&drects[drects_count]);
				}
				if (upside_down) {
					flip_screen(offscreen_surface);
				}
				drects_count = 0;
			}
		}
	}

	play_next_sound();
	// Note: texts are identified by their total time!
	if (text_time_remaining == 1) {
		// If the text's is about to expire:
		if (text_time_total == 36 || text_time_total == 288) {
			// 36: died on demo/potions level
			// 288: press button to continue
			// In this case, restart the game.
			start_level = 0;
			word_1F05E = 1;
			start_game();
		} else {
			// Otherwise, just clear it.
			erase_bottom_text(1);
		}
	} else {
		if (text_time_remaining != 0 && text_time_total != 1188) {
			// 1188: potions level (page/line/word) -- this one does not disappear
			--text_time_remaining;
			if (text_time_total == 288 && text_time_remaining < 72) {
				// 288: press button to continue
				// Blink the message:
				var_2 = text_time_remaining % 12;
				if (var_2 > 3) {
					erase_bottom_text(0);
				} else {
					if (var_2 == 3) {
						display_text_bottom("Press Button to Continue");
						play_sound_from_buffer(sound_pointers[sound_38_blink]); // press button blink
					}
				}
			}
		}
	}
}

// seg000:0B12
void __pascal far anim_tile_modif() {
	word tilepos;
	for (tilepos = 0; tilepos < 30; ++tilepos) {
		switch (get_curr_tile(tilepos)) {
			case tiles_10_potion:
				start_anim_potion(drawn_room, tilepos);
			break;
			case tiles_19_torch:
			case tiles_30_torch_with_debris:
				start_anim_torch(drawn_room, tilepos);
			break;
			case tiles_22_sword:
				start_anim_sword(drawn_room, tilepos);
			break;
		}
	}
}

// seg000:0B72
void __pascal far load_sounds(int first,int last) {
	dat_type* ibm_dat = NULL;
	dat_type* digi1_dat = NULL;
//	dat_type* digi2_dat = NULL;
	dat_type* digi3_dat = NULL;
	dat_type* midi_dat = NULL;
	short current;
	ibm_dat = open_dat("IBM_SND1.DAT", 0);
	if (sound_flags & sfDigi) {
		digi1_dat = open_dat("DIGISND1.DAT", 0);
//		digi2_dat = open_dat("DIGISND2.DAT", 0);
		digi3_dat = open_dat("DIGISND3.DAT", 0);
	}
	if (sound_flags & sfMidi) {
		midi_dat = open_dat("MIDISND1.DAT", 0);
	}

	#ifdef USE_MIXER
	load_sound_names();
	#endif

	for (current = first; current <= last; ++current) {
		if (sound_pointers[current] != NULL) continue;
		/*if (demo_mode) {
			sound_pointers[current] = decompress_sound((sound_buffer_type*) load_from_opendats_alloc(current + 10000));
		} else*/ {
			//sound_pointers[current] = (sound_buffer_type*) load_from_opendats_alloc(current + 10000, "bin", NULL, NULL);
			//printf("overwriting sound_pointers[%d] = %p\n", current, sound_pointers[current]);


			sound_pointers[current] = load_sound(current);
		}
	}
	if (midi_dat) close_dat(midi_dat);
	if (digi1_dat) close_dat(digi1_dat);
//	if (digi2_dat) close_dat(digi2_dat);
	if (digi3_dat) close_dat(digi3_dat);
	close_dat(ibm_dat);
}

// seg000:0C5E
void __pascal far load_opt_sounds(int first,int last) {
	// stub
	dat_type* ibm_dat = NULL;
	dat_type* digi_dat = NULL;
	dat_type* midi_dat = NULL;
	short current;
	ibm_dat = open_dat("IBM_SND2.DAT", 0);
	if (sound_flags & sfDigi) {
		digi_dat = open_dat("DIGISND2.DAT", 0);
	}
	if (sound_flags & sfMidi) {
		midi_dat = open_dat("MIDISND2.DAT", 0);
	}
	for (current = first; current <= last; ++current) {
		//We don't free sounds, so load only once.
		if (sound_pointers[current] != NULL) continue;
		/*if (demo_mode) {
			sound_pointers[current] = decompress_sound((sound_buffer_type*) load_from_opendats_alloc(current + 10000));
		} else*/ {
			//sound_pointers[current] = (sound_buffer_type*) load_from_opendats_alloc(current + 10000, "bin", NULL, NULL);
			//printf("overwriting sound_pointers[%d] = %p\n", current, sound_pointers[current]);
			sound_pointers[current] = load_sound(current);
		}
	}
	if (midi_dat) close_dat(midi_dat);
	if (digi_dat) close_dat(digi_dat);
	close_dat(ibm_dat);
}

// data:03BA
const char*const tbl_guard_dat[] = {"GUARD.DAT", "FAT.DAT", "SKEL.DAT", "VIZIER.DAT", "SHADOW.DAT"};
// data:03C4
const char*const tbl_envir_gr[] = {"", "C", "C", "E", "E", "V"};
// data:03D0
const char*const tbl_envir_ki[] = {"DUNGEON", "PALACE"};
// seg000:0D20
void __pascal far load_lev_spr(int level) {
	dat_type* dathandle;
	short guardtype;
	char filename[20];
	dathandle = NULL;
	current_level = next_level = level;
	draw_rect(&screen_rect, 0);
	free_optsnd_chtab();
	snprintf(filename, sizeof(filename), "%s%s.DAT",
		tbl_envir_gr[graphics_mode],
		tbl_envir_ki[tbl_level_type[current_level]]
	);
	load_chtab_from_file(id_chtab_6_environment, 200, filename, 1<<5);
	load_more_opt_graf(filename);
	guardtype = tbl_guard_type[current_level];
	if (guardtype != -1) {
		if (guardtype == 0) {
			dathandle = open_dat(tbl_level_type[current_level] ? "GUARD1.DAT" : "GUARD2.DAT", 0);
		}
		load_chtab_from_file(id_chtab_5_guard, 750, tbl_guard_dat[guardtype], 1<<8);
		if (dathandle) {
			close_dat(dathandle);
		}
	}
	curr_guard_color = 0;
	load_chtab_from_file(id_chtab_7_environmentwall, 360, filename, 1<<6);
	/*if (comp_skeleton[current_level])*/ {
		load_opt_sounds(44, 44); // skel alive
	}
	/*if (comp_mirror[current_level])*/ {
		load_opt_sounds(45, 45); // mirror
	}
	/*if (comp_chomper[current_level])*/ {
		load_opt_sounds(46, 47); // something chopped, chomper
	}
	/*if (comp_spike[current_level])*/ {
		load_opt_sounds(48, 49); // something spiked, spikes
	}
}

// seg000:0E6C
void __pascal far load_level() {
	dat_type* dathandle;
	dathandle = open_dat("LEVELS.DAT", 0);
	load_from_opendats_to_area(current_level + 2000, &level, sizeof(level), "bin");
	close_dat(dathandle);

	alter_mods_allrm();
}

// seg000:0EA8
// returns 1 if level is restarted, 0 otherwise
int __pascal far play_kid_frame() {
	loadkid_and_opp();
	load_fram_det_col();
	check_killed_shadow();
	play_kid();
	if (upside_down && Char.alive >= 0) {
		upside_down = 0;
		need_redraw_because_flipped = 1;
	}
	if (is_restart_level) {
		return 1;
	}
	if (Char.room != 0) {
		play_seq();
		fall_accel();
		fall_speed();
		load_frame_to_obj();
		load_fram_det_col();
		set_char_collision();
		bump_into_opponent();
		check_collisions();
		check_bumped();
		check_gate_push();
		check_action();
		check_press();
		check_spike_below();
		if (resurrect_time == 0) {
			check_spiked();
			check_chomped_kid();
		}
		check_knock();
	}
	savekid();
	return 0;
}

// seg000:0F48
void __pascal far play_guard_frame() {
	if (Guard.direction != dir_56_none) {
		loadshad_and_opp();
		load_fram_det_col();
		check_killed_shadow();
		play_guard();
		if (Char.room == drawn_room) {
			play_seq();
			if (Char.x >= 44 && Char.x < 211) {
				fall_accel();
				fall_speed();
				load_frame_to_obj();
				load_fram_det_col();
				set_char_collision();
				check_guard_bumped();
				check_action();
				check_press();
				check_spike_below();
				check_spiked();
				check_chomped_guard();
			}
		}
		saveshad();
	}
}

// seg000:0FBD
void __pascal far check_the_end() {
	if (next_room != 0 && next_room != drawn_room) {
		drawn_room = next_room;
		load_room_links();
		if (current_level == 14 && drawn_room == 5) {
			// Special event: end of game
			end_sequence();
		}
		different_room = 1;
		loadkid();
		anim_tile_modif();
		start_chompers();
		check_fall_flo();
		check_shadow();
	}
}

// seg000:1009
void __pascal far check_fall_flo() {
	// Special event: falling floors
	if (current_level == 13 && (drawn_room == 23 || drawn_room == 16)) {
		get_room_address(curr_room = room_A);
		for (curr_tilepos = 22; curr_tilepos <= 27; ++curr_tilepos) {
			make_loose_fall(-(prandom(0xFF) & 0x0F));
		}
	}
}

// seg000:1051
void __pascal far read_joyst_control() {
	// stub
	if (joy_states[0] == -1) 
		control_x = -1;
	
	if (joy_states[0] == 1)
		control_x = 1;

	if (joy_states[1] == -1)
		control_y = -1;

	if (joy_states[1] == 1)
		control_y = 1;

	if (joy_states[2])
		control_shift = -1;
}

// seg000:10EA
void __pascal far draw_kid_hp(short curr_hp,short max_hp) {
	short drawn_hp_index;
	for (drawn_hp_index = curr_hp; drawn_hp_index < max_hp; ++drawn_hp_index) {
		// empty HP
		method_6_blit_img_to_scr(chtab_addrs[id_chtab_2_kid]->images[217], drawn_hp_index * 7, 194, blitters_0_no_transp);
	}
	for (drawn_hp_index = 0; drawn_hp_index < curr_hp; ++drawn_hp_index) {
		// full HP
		method_6_blit_img_to_scr(chtab_addrs[id_chtab_2_kid]->images[216], drawn_hp_index * 7, 194, blitters_0_no_transp);
	}
}

// seg000:1159
void __pascal far draw_guard_hp(short curr_hp,short max_hp) {
	short drawn_hp_index;
	short guard_charid;
	if (chtab_addrs[id_chtab_5_guard] == NULL) return;
	guard_charid = Guard.charid;
	if (guard_charid != charid_4_skeleton &&
		guard_charid != charid_24_mouse &&
		// shadow has HP only on level 12
		(guard_charid != charid_1_shadow || current_level == 12)
	) {
		for (drawn_hp_index = curr_hp; drawn_hp_index < max_hp; ++drawn_hp_index) {
			method_6_blit_img_to_scr(chtab_addrs[id_chtab_5_guard]->images[0], 314 - drawn_hp_index * 7, 194, blitters_9_black);
		}
		for (drawn_hp_index = 0; drawn_hp_index < curr_hp; ++drawn_hp_index) {
			method_6_blit_img_to_scr(chtab_addrs[id_chtab_5_guard]->images[0], 314 - drawn_hp_index * 7, 194, blitters_0_no_transp);
		}
	}
}

// seg000:11EC
void __pascal far add_life() {
	short hpmax = hitp_max;
	++hpmax;
	if (hpmax > 10) hpmax = 10;
	hitp_max = hpmax;
	set_health_life();
}

// seg000:1200
void __pascal far set_health_life() {
	hitp_delta = hitp_max - hitp_curr;
}

// seg000:120B
void __pascal far draw_hp() {
	if (hitp_delta) {
		draw_kid_hp(hitp_curr, hitp_max);
	}
	if (hitp_curr == 1 && current_level != 15) {
		// blinking hitpoint
		if (rem_tick & 1) {
			draw_kid_hp(1, 0);
		} else {
			draw_kid_hp(0, 1);
		}
	}
	if (guardhp_delta) {
		draw_guard_hp(guardhp_curr, guardhp_max);
	}
	if (guardhp_curr == 1) {
		if (rem_tick & 1) {
			draw_guard_hp(1, 0);
		} else {
			draw_guard_hp(0, 1);
		}
	}
}

// seg000:127B
void __pascal far do_delta_hp() {
	// level 12: if the shadow is hurt, Kid is also hurt
	if (Opp.charid == charid_1_shadow &&
		current_level == 12 &&
		guardhp_delta != 0
	) {
		hitp_delta = guardhp_delta;
	}
	hitp_curr = MIN(MAX(hitp_curr + hitp_delta, 0), hitp_max);
	guardhp_curr = MIN(MAX(guardhp_curr + guardhp_delta, 0), guardhp_max);
}

byte sound_prio_table[] = {
	0x14, 0x1E, 0x23, 0x66, 0x32, 0x37, 0x30, 0x30, 0x4B, 0x50, 0x0A,
	0x12, 0x0C, 0x0B, 0x69, 0x6E, 0x73, 0x78, 0x7D, 0x82, 0x91, 0x96,
	0x9B, 0xA0, 1, 1, 1, 1, 1, 0x13, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 0, 1, 1, 1, 1, 0x87, 0x8C, 0x0F, 0x10, 0x19, 0x16, 1,
	0, 1, 1, 1, 1, 1, 0
};
byte sound_pcspeaker_exists[] = {
	1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1,
	1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 0
};

// seg000:12C5
void __pascal far play_sound(int sound_id) {
	//printf("Would play sound %d\n", sound_id);
	if (next_sound < 0 || sound_prio_table[sound_id] <= sound_prio_table[next_sound]) {
		if (sound_pcspeaker_exists[sound_id] != 0 || sound_pointers[sound_id]->type != sound_speaker) {
			next_sound = sound_id;
		}
	}
}

// seg000:1304
void __pascal far play_next_sound() {
	if (next_sound >= 0) {
		if (!check_sound_playing() ||
			(sound_interruptible[current_sound] != 0 && sound_prio_table[next_sound] <= sound_prio_table[current_sound])
		) {
			current_sound = next_sound;
			play_sound_from_buffer(sound_pointers[current_sound]);
		}
	}
	next_sound = -1;
}

// seg000:1353
void __pascal far check_sword_vs_sword() {
	if (Kid.frame == 167 || Guard.frame == 167) {
		play_sound(sound_10_sword_vs_sword); // sword vs. sword
	}
}

// seg000:136A
void __pascal far load_chtab_from_file(int chtab_id,int resource,const char near *filename,int palette_bits) {
	//printf("Loading chtab %d, id %d from %s\n",chtab_id,resource,filename);
	dat_type* dathandle;
	if (chtab_addrs[chtab_id] != NULL) return;
	dathandle = open_dat(filename, 0);
	chtab_addrs[chtab_id] = load_sprites_from_file(resource, palette_bits, 1);
	close_dat(dathandle);
}

// seg000:13BA
void __pascal far free_all_chtabs_from(int first) {
	word chtab_id;
	free_peels();
	for (chtab_id = first; chtab_id < 10; ++chtab_id) {
		if (chtab_addrs[chtab_id]) {
			free_chtab(chtab_addrs[chtab_id]);
			chtab_addrs[chtab_id] = NULL;
		}
	}
}

// seg009:12EF
void __pascal far load_one_optgraf(chtab_type* chtab_ptr,dat_pal_type far *pal_ptr,int base_id,int min_index,int max_index) {
	short index;
	for (index = min_index; index <= max_index; ++index) {
		image_type* image = load_image(base_id + index + 1, pal_ptr);
		if (image != NULL) chtab_ptr->images[index] = image;
	}
}

byte optgraf_min[] = {0x01, 0x1E, 0x4B, 0x4E, 0x56, 0x65, 0x7F, 0x0A};
byte optgraf_max[] = {0x09, 0x1F, 0x4D, 0x53, 0x5B, 0x7B, 0x8F, 0x0D};
// seg000:13FC
void __pascal far load_more_opt_graf(const char *filename) {
	// stub
	dat_type* dathandle;
	dat_shpl_type area;
	short graf_index;
	dathandle = NULL;
	for (graf_index = 0; graf_index < 8; ++graf_index) {
		/*if (...) */ {
			if (dathandle == NULL) {
				dathandle = open_dat(filename, 0);
				load_from_opendats_to_area(200, &area, sizeof(area), "pal");
				area.palette.row_bits = 0x20;
			}
			load_one_optgraf(chtab_addrs[id_chtab_6_environment], &area.palette, 1200, optgraf_min[graf_index] - 1, optgraf_max[graf_index] - 1);
		}
	}
	if (dathandle != NULL) {
		close_dat(dathandle);
	}
}

// seg000:148D
int __pascal far do_paused() {
	word key;
	key = 0;
	next_room = 0;
	control_shift = 0;
	control_y = 0;
	control_x = 0;
	if (is_joyst_mode) {
		read_joyst_control();
	} else {
		read_keyb_control();
	}
	key = process_key();
	if (is_paused) {
		is_paused = 0;
		display_text_bottom("GAME PAUSED");
		// busy waiting?
		do {
			idle();
			//request_screen_update();
		} while (! process_key());
		erase_bottom_text(1);
	}
	return key || control_shift;
}

// seg000:1500
void __pascal far read_keyb_control() {

	//if (key_states[0x48] || key_states[0x47] || key_states[0x49]) {
	if (key_states[SDL_SCANCODE_UP] || key_states[SDL_SCANCODE_HOME] || key_states[SDL_SCANCODE_PAGEUP]
		|| key_states[SDL_SCANCODE_KP_8] || key_states[SDL_SCANCODE_KP_7] || key_states[SDL_SCANCODE_KP_9]
			) {
		control_y = -1;
		//} else if (key_states[0x4C] || key_states[0x50]) {
	} else if (key_states[SDL_SCANCODE_CLEAR] || key_states[SDL_SCANCODE_DOWN]
			   || key_states[SDL_SCANCODE_KP_5] || key_states[SDL_SCANCODE_KP_2]
			) {
		control_y = 1;
	}
	//if (key_states[0x4B] || key_states[0x47]) {
	if (key_states[SDL_SCANCODE_LEFT] || key_states[SDL_SCANCODE_HOME]
		|| key_states[SDL_SCANCODE_KP_4] || key_states[SDL_SCANCODE_KP_7]
			) {
		control_x = -1;
		//} else if (key_states[0x4D] || key_states[0x49]) {
	} else if (key_states[SDL_SCANCODE_RIGHT] || key_states[SDL_SCANCODE_PAGEUP]
			   || key_states[SDL_SCANCODE_KP_6] || key_states[SDL_SCANCODE_KP_9]
			) {
		control_x = 1;
	}
	control_shift = -(key_states[SDL_SCANCODE_LSHIFT] || key_states[SDL_SCANCODE_RSHIFT]);

	#ifdef USE_DEBUG_CHEATS
	if (cheats_enabled) {
		if (key_states[SDL_SCANCODE_RIGHTBRACKET]) ++Char.x;
		else if (key_states[SDL_SCANCODE_LEFTBRACKET]) --Char.x;
	}
	#endif
}

// seg000:156D
void __pascal far copy_screen_rect(const rect_type far *source_rect_ptr) {
	const rect_type* far target_rect_ptr;
	rect_type target_rect;
	if (upside_down) {
		target_rect_ptr = &target_rect;
		/**target_rect_ptr*/target_rect = *source_rect_ptr;
		/*target_rect_ptr->*/target_rect.top = 192 - source_rect_ptr->bottom;
		/*target_rect_ptr->*/target_rect.bottom = 192 - source_rect_ptr->top;
	} else {
		target_rect_ptr = source_rect_ptr;
	}
	method_1_blit_rect(onscreen_surface_, offscreen_surface, target_rect_ptr, target_rect_ptr, 0);
}

// seg000:15E9
void __pascal far toggle_upside() {
	upside_down = ~ upside_down;
	need_redraw_because_flipped = 1;
}

// seg000:15F8
void __pascal far feather_fall() {
	is_feather_fall = 1;
	flash_color = 2; // green
	flash_time = 3;
	stop_sounds();
	play_sound(sound_39_low_weight); // low weight
}

// seg000:1618
int __pascal far parse_grmode() {
	// stub
	set_gr_mode(gmMcgaVga);
	return gmMcgaVga;
}

// seg000:172C
void __pascal far gen_palace_wall_colors() {
	dword old_randseed;
	word prev_color;
	short row;
	short subrow;
	word color_base;
	short column;
	word color;

	old_randseed = random_seed;
	random_seed = drawn_room;
	prandom(1); // discard
	for (row = 0; row < 3; row++) {
		for (subrow = 0; subrow < 4; subrow++) {
			if (subrow % 2) {
				color_base = 0x61; // 0x61..0x64 in subrow 1 and 3
			} else {
				color_base = 0x66; // 0x66..0x69 in subrow 0 and 2
			}
			prev_color = -1;
			for (column = 0; column <= 10; ++column) {
				do {
					color = color_base + prandom(3);
				} while (color == prev_color);
				palace_wall_colors[44 * row + 11 * subrow + column] = color;
				//palace_wall_colors[row][subrow][column] = color;
				prev_color = color;
			}
		}
	}
	random_seed = old_randseed;
}

// data:042E
const rect_type rect_titles = {106,24,195,296};

// seg000:17E6
void __pascal far show_title() {
	word textcolor;
	load_opt_sounds(sound_50_story_2_princess, sound_55_story_1_absence); // main theme, story, princess door
	textcolor = get_text_color(15, 15, 0x800);
	dont_reset_time = 0;
	if(offscreen_surface) free_surface(offscreen_surface); // missing in original
	offscreen_surface = make_offscreen_buffer(&screen_rect);
	load_title_images(1);
	current_target_surface = offscreen_surface;
	do_wait(timer_0);

	draw_image_2(0 /*main title image*/, chtab_title50, 0, 0, blitters_0_no_transp);
	fade_in_2(offscreen_surface, 0x1000); //STUB
	method_1_blit_rect(onscreen_surface_, offscreen_surface, &screen_rect, &screen_rect, blitters_0_no_transp);
	play_sound_from_buffer(sound_pointers[54]); // main theme
	start_timer(timer_0, 0x82);
	draw_image_2(1 /*Broderbund Software presents*/, chtab_title50, 96, 106, blitters_0_no_transp);
	do_wait(0);

	start_timer(timer_0,0xCD);
	method_1_blit_rect(onscreen_surface_, offscreen_surface, &rect_titles, &rect_titles, blitters_0_no_transp);
	draw_image_2(0 /*main title image*/, chtab_title50, 0, 0, blitters_0_no_transp);
	do_wait(0);
	
	start_timer(timer_0,0x41);
	method_1_blit_rect(onscreen_surface_, offscreen_surface, &rect_titles, &rect_titles, blitters_0_no_transp);
	draw_image_2(0 /*main title image*/, chtab_title50, 0, 0, blitters_0_no_transp);
	draw_image_2(2 /*a game by Jordan Mechner*/, chtab_title50, 96, 122, blitters_0_no_transp);
	do_wait(0);
	
	start_timer(timer_0,0x10E);
	method_1_blit_rect(onscreen_surface_, offscreen_surface, &rect_titles, &rect_titles, blitters_0_no_transp);
	draw_image_2(0 /*main title image*/, chtab_title50, 0, 0, blitters_0_no_transp);
	do_wait(0);
	
	start_timer(timer_0,0xEB);
	method_1_blit_rect(onscreen_surface_, offscreen_surface, &rect_titles, &rect_titles, blitters_0_no_transp);
	draw_image_2(0 /*main title image*/, chtab_title50, 0, 0, blitters_0_no_transp);
	draw_image_2(3 /*Prince Of Persia*/, chtab_title50, 24, 107, blitters_10h_transp);
	draw_image_2(4 /*Copyright 1990 Jordan Mechner*/, chtab_title50, 48, 184, blitters_0_no_transp);
	do_wait(0);

	method_1_blit_rect(onscreen_surface_, offscreen_surface, &rect_titles, &rect_titles, blitters_0_no_transp);
	draw_image_2(0 /*story frame*/, chtab_title40, 0, 0, blitters_0_no_transp);
	draw_image_2(1 /*In the Sultan's absence*/, chtab_title40, 24, 25, textcolor);
	current_target_surface = onscreen_surface_;
	while (check_sound_playing()) {
		idle();
		do_paused();
	}
//	method_1_blit_rect(onscreen_surface_, offscreen_surface, &screen_rect, &screen_rect, blitters_0_no_transp);
	play_sound_from_buffer(sound_pointers[sound_55_story_1_absence]); // story 1: In the absence
	transition_ltr();
	pop_wait(timer_0, 0x258);
	fade_out_2(0x800);
	release_title_images();
	
	load_intro(0, &pv_scene, 0);
	
	load_title_images(1);
	current_target_surface = offscreen_surface;
	draw_image_2(0 /*story frame*/, chtab_title40, 0, 0, blitters_0_no_transp);
	draw_image_2(2 /*Marry Jaffar*/, chtab_title40, 24, 25, textcolor);
	fade_in_2(offscreen_surface, 0x800);
	draw_image_2(0 /*main title image*/, chtab_title50, 0, 0, blitters_0_no_transp);
	draw_image_2(3 /*Prince Of Persia*/, chtab_title50, 24, 107, blitters_10h_transp);
	draw_image_2(4 /*Copyright 1990 Jordan Mechner*/, chtab_title50, 48, 184, blitters_0_no_transp);
	while (check_sound_playing()) {
		idle();
		do_paused();
	}
	transition_ltr();
	pop_wait(0, 0x78);
	draw_image_2(0 /*story frame*/, chtab_title40, 0, 0, blitters_0_no_transp);
	draw_image_2(4 /*credits*/, chtab_title40, 24, 26, textcolor);
	transition_ltr();
	pop_wait(timer_0, 0x168);
	if (hof_count) {
		draw_image_2(0 /*story frame*/, chtab_title40, 0, 0, blitters_0_no_transp);
		draw_image_2(3 /*Prince Of Persia*/, chtab_title50, 24, 24, blitters_10h_transp);
		show_hof();
		transition_ltr();
		pop_wait(timer_0, 0xF0);
	}
	current_target_surface = onscreen_surface_;
	while (check_sound_playing()) {
		idle();
		do_paused();
	}
	fade_out_2(0x1800);
	free_surface(offscreen_surface);
	offscreen_surface = NULL; // added
	release_title_images();
	init_game(0);
}

// seg000:1BB3
void __pascal far transition_ltr() {
	short position;
	rect_type rect;
	rect.top = 0;
	rect.bottom = 200;
	rect.left = 0;
	rect.right = 2;
	for (position = 0; position < 320; position += 2) {
		method_1_blit_rect(onscreen_surface_, offscreen_surface, &rect, &rect, 0);
		rect.left += 2;
		rect.right += 2;
		pop_wait(timer_1, 0);
	}
}

// seg000:1C0F
void __pascal far release_title_images() {
	if (chtab_title50) {
		free_chtab(chtab_title50);
		chtab_title50 = NULL;
	}
	if (chtab_title40) {
		free_chtab(chtab_title40);
		chtab_title40 = NULL;
	}
}

// seg000:1C3A
void __pascal far draw_image_2(int id, chtab_type* chtab_ptr, int xpos, int ypos, int blit) {
	image_type* source;
	image_type* decoded_image;
	image_type* mask;
	mask = NULL;
	source = chtab_ptr->images[id];
	decoded_image = source;
	if (blit != blitters_0_no_transp && blit != blitters_10h_transp) {
		method_3_blit_mono(decoded_image, xpos, ypos, blitters_0_no_transp, blit);
	} else if (blit == blitters_10h_transp) {
		if (graphics_mode == gmCga || graphics_mode == gmHgaHerc) {
			//...
		} else {
			mask = decoded_image;
		}
		draw_image_transp(decoded_image, mask, xpos, ypos);
		if (graphics_mode == gmCga || graphics_mode == gmHgaHerc) {
			free_far(mask);
		}
	} else {
		method_6_blit_img_to_scr(decoded_image, xpos, ypos, blit);
	}
}

// seg000:1D2C
void __pascal far load_kid_sprite() {
	load_chtab_from_file(id_chtab_2_kid, 400, "KID.DAT", 1<<7);
}

// seg000:1D45
void __pascal far save_game() {
	word success;
	int handle;
	success = 0;
	// no O_TRUNC
	handle = open("PRINCE.SAV", O_WRONLY | O_CREAT | O_BINARY, 0600);
	if (handle == -1) goto loc_1DB8;
	if (write(handle, &rem_min, 2) == 2) goto loc_1DC9;
	loc_1D9B:
	close(handle);
	if (!success) {
		unlink("PRINCE.SAV");
	}
	loc_1DB8:
	if (!success) goto loc_1E18;
	display_text_bottom("GAME SAVED");
	goto loc_1E2E;
	loc_1DC9:
	if (write(handle, &rem_tick, 2) != 2) goto loc_1D9B;
	if (write(handle, &current_level, 2) != 2) goto loc_1D9B;
	if (write(handle, &hitp_beg_lev, 2) != 2) goto loc_1D9B;
	success = 1;
	goto loc_1D9B;
	loc_1E18:
	display_text_bottom("UNABLE TO SAVE GAME");
	//play_sound_from_buffer(&sound_cant_save);
	loc_1E2E:
	text_time_remaining = 24;
}

// seg000:1E38
short __pascal far load_game() {
	int handle;
	word success;
	success = 0;
	handle = open("PRINCE.SAV", O_RDONLY | O_BINARY);
	if (handle == -1) goto loc_1E99;
	if (read(handle, &rem_min, 2) == 2) goto loc_1E9E;
	loc_1E8E:
	close(handle);
	loc_1E99:
	return success;
	loc_1E9E:
	if (read(handle, &rem_tick, 2) != 2) goto loc_1E8E;
	if (read(handle, &start_level, 2) != 2) goto loc_1E8E;
	if (read(handle, &hitp_beg_lev, 2) != 2) goto loc_1E8E;
#ifdef USE_COPYPROT
	if (options.enable_copyprot && copyprot_level > 0) {
		copyprot_level = start_level;
	}
#endif
	success = 1;
	dont_reset_time = 1;
	goto loc_1E8E;
}

// seg000:1F02
void __pascal far clear_screen_and_sounds() {
	short index;
	stop_sounds();
	current_target_surface = rect_sthg(onscreen_surface_, &screen_rect);

	word_1EFAA = 0;
	peels_count = 0;
	// should these be freed?
	for (index = 2; index < 10; ++index) {
		if (chtab_addrs[index]) {
			// Original code does not free these?
			free_chtab(chtab_addrs[index]);
			chtab_addrs[index] = NULL;
		}
	}
	/* //Don't free sounds.
	for (index = 44; index < 57; ++index) {
		//continue; // don't release sounds? modern machines have enough memory
		free_sound(sound_pointers[index]); // added
		sound_pointers[index] = NULL;
	}
	*/
	current_level = -1;
}

// seg000:1F7B
void __pascal far parse_cmdline_sound() {
	// stub
	sound_flags |= sfDigi;
}

// seg000:226D
void __pascal far free_optional_sounds() {
	/* //Don't free sounds.
	int sound_id;
	for (sound_id = 44; sound_id < 57; ++sound_id) {
		free_sound(sound_pointers[sound_id]);
		sound_pointers[sound_id] = NULL;
	}
	*/
	// stub
}

// seg000:22BB
void __pascal far free_optsnd_chtab() {
	free_optional_sounds();
	free_all_chtabs_from(id_chtab_3_princessinstory);
}

// seg000:22C8
void __pascal far load_title_images(int bgcolor) {
	dat_type* dathandle;
	dathandle = open_dat("TITLE.DAT", 0);
	chtab_title40 = load_sprites_from_file(40, 1<<11, 1);
	chtab_title50 = load_sprites_from_file(50, 1<<12, 1);
	close_dat(dathandle);
	if (graphics_mode == gmMcgaVga) {
		// background of text frame
		SDL_Color color;
		if (bgcolor) {
			// RGB(4,0,18h) = #100060 = dark blue
			set_pal((find_first_pal_row(1<<11) << 4) + 14, 0x04, 0x00, 0x18, 1);
			color.r = 0x10;
			color.g = 0x00;
			color.b = 0x60;
			color.a = 0xFF;
		} else {
			// RGB(20h,0,0) = #800000 = dark red
			set_pal((find_first_pal_row(1<<11) << 4) + 14, 0x20, 0x00, 0x00, 1);
			color.r = 0x80;
			color.g = 0x00;
			color.b = 0x00;
			color.a = 0xFF;
		}
		SDL_SetPaletteColors(chtab_title40->images[0]->format->palette, &color, 14, 1);
	} else if (graphics_mode == gmEga || graphics_mode == gmTga) {
		// ...
	}
}

#ifdef USE_COPYPROT
// data:017A
const word copyprot_word[] = {9,  1,  6,  4,  5,  3,  6,  3,  4,  4,  3,  2, 12,  5, 13,  1,  9,  2,  2,  4,  9,  4, 11,  8,  5,  4,  1,  6,  2,  4,  6,  8,  4,  2,  7, 11,  5,  4,  1,  2};
// data:012A
const word copyprot_line[] = {2,  1,  5,  4,  3,  5,  1,  3,  7,  2,  2,  4,  6,  6,  2,  6,  3,  1,  2,  3,  2,  2,  3, 10,  5,  6,  5,  6,  3,  5,  7,  2,  2,  4,  5,  7,  2,  6,  5,  5};
// data:00DA
const word copyprot_page[] = {5,  3,  7,  3,  3,  4,  1,  5, 12,  5, 11, 10,  1,  2,  8,  8,  2,  4,  6,  1,  4,  7,  3,  2,  1,  7, 10,  1,  4,  3,  4,  1,  4,  1,  8,  1,  1, 10,  3,  3};
#endif

// seg000:23F4
void __pascal far show_copyprot(int where) {
#ifdef USE_COPYPROT
	char sprintf_temp[140];
	if (current_level != 15) return;
	if (where) {
		if (text_time_remaining || word_1EFAA) return;
		text_time_total = 1188;
		text_time_remaining = 1188;
		is_show_time = 0;
		snprintf(sprintf_temp, sizeof(sprintf_temp), "WORD %d LINE %d PAGE %d", copyprot_word[copyprot_idx], copyprot_line[copyprot_idx], copyprot_page[copyprot_idx]);
		display_text_bottom(sprintf_temp);
	} else {
		snprintf(sprintf_temp, sizeof(sprintf_temp), "Drink potion matching the first letter of Word %d on Line %d\nof Page %d of the manual.", copyprot_word[copyprot_idx], copyprot_line[copyprot_idx], copyprot_page[copyprot_idx]);
		show_dialog(sprintf_temp);
	}
#endif
}

// seg000:2489
void __pascal far show_loading() {
	show_text(&screen_rect, 0, 0, "Loading. . . .");
}

// data:42C4
word which_quote;

char const * const tbl_quotes[2] = {
"\"(****/****) Incredibly realistic. . . The adventurer character actually looks human as he runs, jumps, climbs, and hangs from ledges.\"\n\n                                  Computer Entertainer\n\n\n\n\n\"A tremendous achievement. . . Mechner has crafted the smoothest animation ever seen in a game of this type.\n\n\"PRINCE OF PERSIA is the STAR WARS of its field.\"\n\n                                  Computer Gaming World",
"\"An unmitigated delight. . . comes as close to (perfection) as any arcade game has come in a long, long time. . . what makes this game so wonderful (am I gushing?) is that the little onscreen character does not move like a little onscreen character -- he moves like a person.\"\n\n                                      Nibble"
};

// seg000:249D
void __pascal far show_quotes() {
	start_timer(timer_0,0);
	if (demo_mode && word_1F05E) {
		draw_rect(&screen_rect, 0);
		show_text(&screen_rect, -1, 0, tbl_quotes[which_quote]);
		which_quote = !which_quote;
		start_timer(timer_0,0x384);
	}
	word_1F05E = 0;
}
