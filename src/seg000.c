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
#include <setjmp.h>
#include <math.h>

// data:461E
dat_type * dathandle;

// data:4C08
word need_redraw_because_flipped;

void fix_sound_priorities();

// seg000:0000
void far pop_main() {
	if (check_param("--version") || check_param("-v")) {
		printf ("SDLPoP v%s\n", SDLPOP_VERSION);
		exit(0);
	}

	if (check_param("--help") || check_param("-h") || check_param("-?")) {
		printf ("See doc/Readme.txt\n");
		exit(0);
	}

	const char* temp = check_param("seed=");
	if (temp != NULL) {
		random_seed = atoi(temp+5);
		seed_was_init = 1;
	}

	// debug only: check that the sequence table deobfuscation did not mess things up
	#ifdef CHECK_SEQTABLE_MATCHES_ORIGINAL
	check_seqtable_matches_original();
	#endif

#ifdef FIX_SOUND_PRIORITIES
	fix_sound_priorities();
#endif

	load_global_options();
	check_mod_param();
#ifdef USE_MENU
	load_ingame_settings();
#endif
	if (check_param("mute")) is_sound_on = 0;
	turn_sound_on_off((is_sound_on != 0) * 15); // Turn off sound/music if those options were set.

#ifdef USE_REPLAY
	if (g_argc > 1) {
		char *filename = g_argv[1]; // file dragged on top of executable or double clicked
		char *e = strrchr(filename, '.');
		if (e != NULL && strcasecmp(e, ".P1R") == 0) { // valid replay filename passed as first arg
			start_with_replay_file(filename);
		}
	}

	temp = check_param("validate");
	if (temp != NULL) {
		is_validate_mode = 1;
		start_with_replay_file(temp);
	}
#endif

	load_mod_options();

	// CusPop option
	is_blind_mode = custom->start_in_blind_mode;
	// Fix bug: with start_in_blind_mode enabled, moving objects are not displayed until blind mode is toggled off+on??
	need_drects = 1;

	apply_seqtbl_patches();

	char sprintf_temp[100];
	int i;

	dathandle = open_dat("PRINCE.DAT", 0);

	/*video_mode =*/ parse_grmode();

	init_timer(BASE_FPS);
	parse_cmdline_sound();

	set_hc_pal();

	current_target_surface = rect_sthg(onscreen_surface_, &screen_rect);
	show_loading();
	set_joy_mode();
	cheats_enabled = check_param("megahit") != NULL;
#ifdef USE_DEBUG_CHEATS
	debug_cheats_enabled = check_param("debug") != NULL;
	if (debug_cheats_enabled) cheats_enabled = 1; // param 'megahit' not necessary if 'debug' is used
#endif
	draw_mode = check_param("draw") != NULL && cheats_enabled;
	demo_mode = check_param("demo") != NULL;

	init_copyprot_dialog();
#ifdef USE_REPLAY
	init_record_replay();
#endif

	if (cheats_enabled
		#ifdef USE_REPLAY
		|| recording
		#endif
	) {
		for (i = 15; i >= 0; --i) {
			snprintf(sprintf_temp, sizeof(sprintf_temp), "%d", i);
			if (check_param(sprintf_temp)) {
				start_level = i;
				break;
			}
		}
	}

	play_demo_level = (check_param("playdemo") != NULL);

#ifdef USE_SCREENSHOT
	init_screenshot();
#endif

#ifdef USE_MENU
	init_menu();
#endif

	init_game_main();
}

byte* level_var_palettes;

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

		// Level color variations (1.3)
		level_var_palettes = load_from_opendats_alloc(20, "bin", NULL, NULL);
	}
	// PRINCE.DAT: sword
	chtab_addrs[id_chtab_0_sword] = load_sprites_from_file(700, 1<<2, 1);
	// PRINCE.DAT: flame, sword on floor, potion
	chtab_addrs[id_chtab_1_flameswordpotion] = load_sprites_from_file(150, 1<<3, 1);
	close_dat(dathandle);
#ifdef USE_LIGHTING
	init_lighting();
#endif
	load_all_sounds();

	hof_read();
	show_splash(); // added
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
	if (custom->skip_title) { // CusPop option: skip the title sequence (level loads instantly)
		int level_number = (start_level >= 0) ? start_level : custom->first_level;
		init_game(level_number);
		return;
	}

	if (start_level < 0) {
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
#ifdef USE_DEBUG_CHEATS
	// Don't load the level if the user holds either Shift key while pressing F9.
	if (debug_cheats_enabled && (key_states[SDL_SCANCODE_LSHIFT] || key_states[SDL_SCANCODE_RSHIFT])) {
		fseek(quick_fp, sizeof(level), SEEK_CUR);
	} else
#endif
	{
		process(level);
	}
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
	process(offguard);
	// guard
	process(Guard);
	process(Char);
	process(Opp);
	process(guardhp_curr);
	process(guardhp_max);
	process(demo_index);
	process(demo_time);
	process(curr_guard_color);
	process(guard_notice_timer);
	process(guard_skill);
	process(shadow_initialized);
	process(guard_refrac);
	process(justblocked);
	process(droppedout);
	// collision
	process(curr_row_coll_room);
	process(curr_row_coll_flags);
	process(below_row_coll_room);
	process(below_row_coll_flags);
	process(above_row_coll_room);
	process(above_row_coll_flags);
	process(prev_collision_row);
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
	// replay recording state
#ifdef USE_REPLAY
	process(curr_tick);
#endif
#ifdef USE_COLORED_TORCHES
	process(torch_colors);
#endif
#ifdef USE_SUPER_HIGH_JUMP
    process(super_jump_fall);
    process(super_jump_timer);
    process(super_jump_room);
    process(super_jump_col);
    process(super_jump_row);
#endif
#undef process
	return ok;
}

const char* quick_file = "QUICKSAVE.SAV";
const char quick_version[] = "V1.16b4 ";
char quick_control[] = "........";

const char* get_quick_path(char* custom_path_buffer, size_t max_len) {
	if (!use_custom_levelset) {
		return quick_file;
	}
	// if playing a custom levelset, try to use the mod folder
	snprintf_check(custom_path_buffer, max_len, "%s/%s", mod_data_path, quick_file /*QUICKSAVE.SAV*/ );
	return custom_path_buffer;
}

int quick_save() {
	int ok = 0;
	char custom_quick_path[POP_MAX_PATH];
	const char* path = get_quick_path(custom_quick_path, sizeof(custom_quick_path));
	quick_fp = fopen(path, "wb");
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
	reset_level_unused_fields(false);
	load_lev_spr(current_level);
	curr_guard_color = temp1;
	next_level = temp2;

	// feather fall can only get restored if the fix enabled
	if (!fixes->fix_quicksave_during_feather && is_feather_fall > 0) {
		is_feather_fall = 0;
		stop_sounds();
	}

	//need_full_redraw = 1;
	different_room = 1;
	// Show the room where the prince is, even if the player moved the view away from it (with the H,J,U,N keys).
	next_room = drawn_room = Kid.room;
	load_room_links();
	//draw_level_first();
	//gen_palace_wall_colors();
	is_guard_notice = 0; // prevent guard turning around immediately
	draw_game_frame(); // for falling
	//redraw_screen(1); // for room_L

	hitp_delta = guardhp_delta = 1; // force HP redraw
	// Don't draw guard HP if a previously viewed room (with the H,J,U,N keys) had a guard but the current room doesn't have one.
	if (Guard.room != drawn_room) {
		// Like in clear_char().
		Guard.direction = dir_56_none;
		guardhp_curr = 0;
	}

	draw_hp();
	loadkid_and_opp();
	// Get rid of "press button" message if kid was dead before quickload.
	text_time_total = text_time_remaining = 0;
	//next_sound = current_sound = -1;
	exit_room_timer = 0;
}

int quick_load() {
	int ok = 0;
	char custom_quick_path[POP_MAX_PATH];
	const char* path = get_quick_path(custom_quick_path, sizeof(custom_quick_path));
	quick_fp = fopen(path, "rb");
	if (quick_fp != NULL) {
		// check quicksave version is compatible
		process_load(quick_control, COUNT(quick_control));
		if (strcmp(quick_control, quick_version) != 0) {
			fclose(quick_fp);
			quick_fp = NULL;
			return 0;
		}

		stop_sounds();
		draw_rect(&screen_rect, 0);
		update_screen();
		delay_ticks(5); // briefly display a black screen as a visual cue

		short old_rem_min = rem_min;
		word old_rem_tick = rem_tick;

		ok = quick_process(process_load);
		fclose(quick_fp);
		quick_fp = NULL;

		restore_room_after_quick_load();
		update_screen();

		#ifdef USE_QUICKLOAD_PENALTY
		// Subtract one minute from the remaining time (if it is above 5 minutes)
		if (enable_quicksave_penalty &&
			// don't apply the penalty after time has already stopped!
			(current_level < /*13*/ custom->victory_stops_time_level || (current_level == /*13*/ custom->victory_stops_time_level && leveldoor_open < 2))
		) {
			int ticks_elapsed = 720 * (rem_min - old_rem_min) + (rem_tick - old_rem_tick);
			// don't restore time at all if the elapsed time is between 0 and 1 minutes
			if (ticks_elapsed > 0 && ticks_elapsed < 720) {
				rem_min = old_rem_min;
				rem_tick = old_rem_tick;
			}
			else {
				if (rem_min == 6) rem_tick = 719; // crop to "5 minutes" exactly, if hitting the threshold in <1 minute
				if (rem_min > 5 /*be lenient, not much time left*/ || rem_min < 0 /*time runs 'forward' if < 0*/) {
					--rem_min;
				}
			}

		}
		#endif
	}
	return ok;
}

void check_quick_op() {
	if (!enable_quicksave) return;
	if (need_quick_save) {
		if ((!is_feather_fall || fixes->fix_quicksave_during_feather) && quick_save()) {
			display_text_bottom("QUICKSAVE");
		} else {
			display_text_bottom("NO QUICKSAVE");
		}
		need_quick_save = 0;
		text_time_total = 24;
		text_time_remaining = 24;
	}
	if (need_quick_load) {
/*
#ifdef USE_REPLAY
		if (recording) {
			stop_recording(); // quickloading would mess up the replay!
		}
#endif
*/
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
	const char* answer_text = NULL;
	word need_show_text;
	need_show_text = 0;
	key = key_test_quit();

#ifdef USE_MENU
	if (is_paused && is_menu_shown) {
		key = key_test_paused_menu(key);
		if (key == 0) return 0;
	}
#endif

	if (start_level < 0) {
		if (key || control_shift) {
			#ifdef USE_QUICKSAVE
			if (key == SDL_SCANCODE_F9) need_quick_load = 1;
			#endif
			#ifdef USE_REPLAY
			if (key == SDL_SCANCODE_TAB || need_start_replay) {
				start_replay();
			}
			else if (key == (SDL_SCANCODE_TAB | WITH_CTRL)) {
				start_level = custom->first_level;
				start_recording();
			} else
			#endif
			if (key == (SDL_SCANCODE_L | WITH_CTRL)) { // Ctrl+L
				if (!load_game()) return 0;
			} else {
				start_level = custom->first_level; // 1
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
	// If the Kid died, Enter or Shift will restart the level.
	if (rem_min != 0 && Kid.alive > 6 && (control_shift || key == SDL_SCANCODE_RETURN)) {
		key = SDL_SCANCODE_A | WITH_CTRL; // Ctrl+A
	}
#ifdef USE_REPLAY
	if (recording) key_press_while_recording(&key);
	else if (replaying) key_press_while_replaying(&key);
#endif
	if (key == 0) return 0;
	if (is_keyboard_mode) clear_kbd_buf();

	switch(key) {
		case SDL_SCANCODE_ESCAPE: // Esc
		case SDL_SCANCODE_ESCAPE | WITH_SHIFT: // allow pause while grabbing
			is_paused = 1;
#ifdef USE_MENU
			if (enable_pause_menu && !is_cutscene && !is_ending_sequence) {
				is_menu_shown = 1;
			}
		break;
		case SDL_SCANCODE_BACKSPACE:
			if (!is_cutscene && !is_ending_sequence) {
				is_paused = 1;
				is_menu_shown = 1;
			}
#endif
		break;
		case SDL_SCANCODE_SPACE: // Space
			is_show_time = 1;
		break;
		case SDL_SCANCODE_A | WITH_CTRL: // Ctrl+A
			if (current_level != 15) {
				stop_sounds();
				is_restart_level = 1;
			}
		break;
		case SDL_SCANCODE_G | WITH_CTRL: // Ctrl+G
			// CusPoP: first and last level where saving is allowed
//			if (current_level > 2 && current_level < 14) { // original
			if (current_level >= custom->saving_allowed_first_level && current_level <= custom->saving_allowed_last_level) {
				save_game();
			}
		break;
		case SDL_SCANCODE_J | WITH_CTRL: // Ctrl+J
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
		case SDL_SCANCODE_K | WITH_CTRL: // Ctrl+K
			answer_text = "KEYBOARD MODE";
			is_joyst_mode = 0;
			is_keyboard_mode = 1;
			need_show_text = 1;
		break;
		case SDL_SCANCODE_R | WITH_CTRL: // Ctrl+R
			start_level = -1;
#ifdef USE_MENU
			if (is_menu_shown) menu_was_closed(); // Do necessary cleanup.
#endif
			start_game();
		break;
		case SDL_SCANCODE_S | WITH_CTRL: // Ctrl+S
			turn_sound_on_off((!is_sound_on) * 15);
			answer_text = "SOUND OFF";
			if (is_sound_on) {
				answer_text = "SOUND ON";
			}
			//
			need_show_text = 1;
		break;
		case SDL_SCANCODE_V | WITH_CTRL: // Ctrl+V
			//answer_text = "PRINCE OF PERSIA  V1.0";
			snprintf(sprintf_temp, sizeof(sprintf_temp), "SDLPoP v%s\n", SDLPOP_VERSION);
			answer_text = sprintf_temp;
			need_show_text = 1;
		break;
		case SDL_SCANCODE_C | WITH_CTRL: // Ctrl+C
		{
			SDL_version verc, verl;
			SDL_VERSION (&verc);
			SDL_GetVersion (&verl);
			snprintf (sprintf_temp, sizeof (sprintf_temp),
				"SDL COMP v%u.%u.%u LINK v%u.%u.%u",
				verc.major, verc.minor, verc.patch,
				verl.major, verl.minor, verl.patch);
			answer_text = sprintf_temp;
			need_show_text = 1;
		}
		break;
		case SDL_SCANCODE_L | WITH_SHIFT: // Shift+L
			if (current_level < custom->shift_L_allowed_until_level /* 4 */ || cheats_enabled) {
				// if Shift is not released within the delay, the cutscene is skipped
				Uint32 delay = 250;
				key_states[SDL_SCANCODE_LSHIFT] = 0;
				key_states[SDL_SCANCODE_RSHIFT] = 0;
				SDL_TimerID timer;
				timer = SDL_AddTimer(delay, temp_shift_release_callback, NULL);
				if (timer == 0) {
					sdlperror("process_key: SDL_AddTimer");
					quit(1);
				}
				if (current_level == 14) {
					next_level = 1;
				} else {
					if (current_level == 15 && cheats_enabled) {
#ifdef USE_COPYPROT
						if (enable_copyprot) {
							next_level = custom->copyprot_level;
							custom->copyprot_level = -1;
						}
#endif
					} else {
						next_level = current_level + 1;
						if (!cheats_enabled && rem_min > custom->shift_L_reduced_minutes /* 15 */) {
							rem_min = custom->shift_L_reduced_minutes; // 15
							rem_tick = custom->shift_L_reduced_ticks; // 719
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
#endif // USE_REPLAY
#endif // USE_QUICKSAVE
	}
	if (cheats_enabled) {
		switch (key) {
			case SDL_SCANCODE_C: // c
				snprintf(sprintf_temp, sizeof(sprintf_temp), "S%d L%d R%d A%d B%d", drawn_room, room_L, room_R, room_A, room_B);
				answer_text = /*&*/sprintf_temp;
				need_show_text = 1;
			break;
			case SDL_SCANCODE_C | WITH_SHIFT: // Shift+C
				snprintf(sprintf_temp, sizeof(sprintf_temp), "AL%d AR%d BL%d BR%d", room_AL, room_AR, room_BL, room_BR);
				answer_text = /*&*/sprintf_temp;
				need_show_text = 1;
			break;
			case SDL_SCANCODE_MINUS:
			case SDL_SCANCODE_KP_MINUS:		// '-' --> subtract time cheat
				if (rem_min > 1) --rem_min;

#ifdef ALLOW_INFINITE_TIME
				else if (rem_min < -1) ++rem_min; // if negative/infinite, time runs 'forward'
				else if (rem_min == -1) rem_tick = 720; // resets the timer to 00:00:00
#endif

				text_time_total = 0;
				text_time_remaining = 0;
				is_show_time = 1;
			break;
			case SDL_SCANCODE_EQUALS | WITH_SHIFT: // '+'
			case SDL_SCANCODE_KP_PLUS:	   // '+' --> add time cheat

#ifdef ALLOW_INFINITE_TIME
				if (rem_min < 0) { // if negative/infinite, time runs 'forward'
					if (rem_min > INT16_MIN) --rem_min;
				}
				else ++rem_min;
#else
				++rem_min;
#endif

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
				if (Guard.charid != charid_4_skeleton) {
					guardhp_delta = -guardhp_curr;
					Guard.alive = 0;
				}
			break;
			case SDL_SCANCODE_I | WITH_SHIFT: // Shift+I --> invert cheat
				toggle_upside();
			break;
			case SDL_SCANCODE_W | WITH_SHIFT: // Shift+W --> feather fall cheat
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
			case SDL_SCANCODE_B | WITH_CTRL: // Ctrl+B --> Go back to the room where the prince is. (Undo H,J,U,N.)
				draw_guard_hp(0, 10);
				next_room = Kid.room;
			break;
			case SDL_SCANCODE_B | WITH_SHIFT: // Shift+B
				is_blind_mode = !is_blind_mode;
				if (is_blind_mode) {
					draw_rect(&rect_top, 0);
				} else {
					need_full_redraw = 1;
				}
			break;
			case SDL_SCANCODE_S | WITH_SHIFT: // Shift+S
				if (hitp_curr != hitp_max) {
					play_sound(sound_33_small_potion); // small potion (cheat)
					hitp_delta = 1;
					flash_color = 4; // red
					flash_time = 2;
				}
			break;
			case SDL_SCANCODE_T | WITH_SHIFT: // Shift+T
				play_sound(sound_30_big_potion); // big potion (cheat)
				flash_color = 4; // red
				flash_time = 4;
				add_life();
			break;
			#ifdef USE_DEBUG_CHEATS
			case SDL_SCANCODE_T:
				is_timer_displayed = 1 - is_timer_displayed; // toggle
			break;
			case SDL_SCANCODE_F:
				if (fixes->fix_quicksave_during_feather) {
					is_feather_timer_displayed = 1 - is_feather_timer_displayed; // toggle
				} else {
					is_feather_timer_displayed = 0;
				}
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
	// play feather fall music if there is more than 1 second of feather fall left
	if (fixes->fix_quicksave_during_feather && is_feather_fall >= 10 && !check_sound_playing()) {
		play_sound(sound_39_low_weight);
	}
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
		if (Kid.room == /*24*/ custom->demo_end_room) {
			draw_rect(&screen_rect, 0);
			start_level = -1;
			need_quotes = 1;
			start_game();
		}
	} else if(current_level == /*6*/ custom->falling_exit_level) {
		// Special event: level 6 falling exit
		if (roomleave_result == -2) {
			Kid.y = -1;
			stop_sounds();
			++next_level;
		}
	} else if(/*current_level == 12*/ custom->tbl_seamless_exit[current_level] >= 0) {
		// Special event: level 12 running exit
		if (Kid.room == /*23*/ custom->tbl_seamless_exit[current_level]) {
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
			if (custom->tbl_level_type[current_level]) {
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
			start_level = -1;
			need_quotes = 1;

#ifdef USE_REPLAY
			if (recording) stop_recording();
			if (replaying) end_replay();
#endif

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

	// Animate torches in the rightmost column of the left-side room as well, because they are visible in the current room.
	for (int row = 0; row <= 2; row++) {
		switch (get_tile(room_L, 9, row)) {
			case tiles_19_torch:
			case tiles_30_torch_with_debris:
				start_anim_torch(room_L, row * 10 + 9);
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

	load_sound_names();

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
		tbl_envir_ki[custom->tbl_level_type[current_level]]
	);
	load_chtab_from_file(id_chtab_6_environment, 200, filename, 1<<5);
	load_more_opt_graf(filename);
	guardtype = custom->tbl_guard_type[current_level];
	if (guardtype != -1) {
		if (guardtype == 0) {
			dathandle = open_dat(custom->tbl_level_type[current_level] ? "GUARD1.DAT" : "GUARD2.DAT", 0);
		}
		load_chtab_from_file(id_chtab_5_guard, 750, tbl_guard_dat[guardtype], 1<<8);
		if (dathandle) {
			close_dat(dathandle);
		}
	}
	curr_guard_color = 0;
	load_chtab_from_file(id_chtab_7_environmentwall, 360, filename, 1<<6);

	// Level colors (1.3)
	if (graphics_mode == gmMcgaVga && level_var_palettes != NULL) {
		int level_color = custom->tbl_level_color[current_level];
		if (level_color != 0) {
			byte* env_pal = level_var_palettes + 0x30*(level_color-1);
			byte* wall_pal = env_pal + 0x30 * custom->tbl_level_type[current_level];
			set_pal_arr(0x50, 0x10, (rgb_type*)env_pal, 1);
			set_pal_arr(0x60, 0x10, (rgb_type*)wall_pal, 1);
			set_chtab_palette(chtab_addrs[id_chtab_6_environment], env_pal, 0x10);
			set_chtab_palette(chtab_addrs[id_chtab_7_environmentwall], wall_pal, 0x10);
		}
	}

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
	reset_level_unused_fields(true); // added
}

void reset_level_unused_fields(bool loading_clean_level) {
	// Entirely unused fields in the level format: reset to zero for now
	// They can be repurposed to add new stuff to the level format in the future
	memset(level.roomxs, 0, sizeof(level.roomxs));
	memset(level.roomys, 0, sizeof(level.roomys));
	memset(level.fill_1, 0, sizeof(level.fill_1));
	memset(level.fill_2, 0, sizeof(level.fill_2));
	memset(level.fill_3, 0, sizeof(level.fill_3));

	// level.used_rooms is 25 on some levels. Limit it to the actual number of rooms.
	if (level.used_rooms > 24) level.used_rooms = 24;

	// For these fields, only use the bits that are actually used, and set the rest to zero.
	// Good for repurposing the unused bits in the future.
	int i;
	for (i = 0; i < level.used_rooms; ++i) {
		//level.guards_dir[i]   &= 0x01; // 1 bit in use
		level.guards_skill[i] &= 0x0F; // 4 bits in use
	}

	// In savestates, additional information may be stored (e.g. remembered guard hp) - should not reset this then!
	if (loading_clean_level) {
		for (i = 0; i < level.used_rooms; ++i) {
			level.guards_color[i] &= 0x0F; // 4 bits in use (other 4 bits repurposed as remembered guard hp)
		}
	}

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
		if (current_level == /*14*/ custom->win_level && drawn_room == /*5*/ custom->win_room) {
#ifdef USE_REPLAY
			if (recording) stop_recording();
			if (replaying) end_replay();
#endif
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
	if (current_level == /*13*/ custom->loose_tiles_level &&
			(drawn_room == /*23*/ custom->loose_tiles_room_1 || drawn_room == /*16*/ custom->loose_tiles_room_2)
	) {
		get_room_address(curr_room = room_A);
		for (curr_tilepos = /*22*/ custom->loose_tiles_first_tile;
		     curr_tilepos <= /*27*/ custom->loose_tiles_last_tile; ++curr_tilepos) {
			make_loose_fall(-(prandom(0xFF) & 0x0F));
		}
	}
}

void get_joystick_state(int raw_x, int raw_y, int axis_state[2]) {

#define DEGREES_TO_RADIANS (M_PI/180.0)

	// check if the X/Y position is within the 'dead zone' of the joystick
	int dist_squared = raw_x*raw_x + raw_y*raw_y;
	// FIXED: Left jump (top-left) didn't work on some gamepads.
	// On some gamepads, raw_x = raw_y = -32768 in the top-left corner.
	// In this case, dist_squared is calculated as -32768 * -32768 + -32768 * -32768 = 2147483648.
	// But dist_squared is a 32-bit signed integer, which cannot store that number, so it overflows to -2147483648.
	// Therefore, dist_squared < joystick_threshold*joystick_threshold becomes true, and the game thinks the stick is centered.
	// To fix this, we cast both sides of the comparison to an unsigned 32-bit type.
	if ((dword)dist_squared < (dword)(joystick_threshold*joystick_threshold)) {
		axis_state[0] = 0;
		axis_state[1] = 0;
	} else {
		double angle = atan2(raw_y, raw_x); // angle of the joystick: 0 = right, >0 = downward, <0 = upward
		//printf("Joystick angle is %f degrees\n", angle/DEGREES_TO_RADIANS);

		if (fabs(angle) < (60*DEGREES_TO_RADIANS)) // 120 degree range facing right
			axis_state[0] = 1;

		else if (fabs(angle) > (120*DEGREES_TO_RADIANS)) // 120 degree range facing left
			axis_state[0] = -1;

		else {
			// joystick is neutral horizontally, so the control should be released
			// however: prevent stop running if the Kid was already running / trying to do a running-jump
			// (this tweak makes it a bit easier to do (multiple) running jumps)
			if (!(angle < 0 /*facing upward*/ && Kid.action == actions_1_run_jump)) {
				axis_state[0] = 0;
			}
		}

		if (angle < (-30*DEGREES_TO_RADIANS) && angle > (-150*DEGREES_TO_RADIANS)) // 120 degree range facing up
			axis_state[1] = -1;

		// down slightly less sensitive than up (prevent annoyance when your thumb slips down a bit by accident)
		// (not sure if this adjustment is really necessary)
		else if (angle > (35*DEGREES_TO_RADIANS) && angle < (145*DEGREES_TO_RADIANS)) // 110 degree range facing down
			axis_state[1] = 1;

		else {
			// joystick is neutral vertically, so the control should be released
			// however: should prevent unintended standing up when attempting to crouch-hop
			if (!((Kid.frame >= frame_108_fall_land_2 && Kid.frame <= frame_112_stand_up_from_crouch_3)
				  && angle > 0 /*facing downward*/))
			{
				axis_state[1] = 0;
			}
		}
	}
}

void get_joystick_state_hor_only(int raw_x, int axis_state[2]) {
	if (raw_x > joystick_threshold) {
		axis_state[0] = 1;
	} else if (raw_x < -joystick_threshold) {
		axis_state[0] = -1;
	} else axis_state[0] = 0;

	// disregard all vertical input from the joystick controls (only use Y and A buttons or D-pad for up/down)
	axis_state[1] = 0;
}

// seg000:1051
void __pascal far read_joyst_control() {

	if (joystick_only_horizontal) {
		get_joystick_state_hor_only(joy_axis[SDL_CONTROLLER_AXIS_LEFTX], joy_left_stick_states);
		get_joystick_state_hor_only(joy_axis[SDL_CONTROLLER_AXIS_RIGHTX], joy_right_stick_states);
	} else {
		get_joystick_state(joy_axis[SDL_CONTROLLER_AXIS_LEFTX], joy_axis[SDL_CONTROLLER_AXIS_LEFTY], joy_left_stick_states);
		get_joystick_state(joy_axis[SDL_CONTROLLER_AXIS_RIGHTX], joy_axis[SDL_CONTROLLER_AXIS_RIGHTY], joy_right_stick_states);
	}

	if (joy_left_stick_states[0] == -1 || joy_right_stick_states[0] == -1 || joy_hat_states[0] == -1)
		control_x = -1;

	if (joy_left_stick_states[0] == 1 || joy_right_stick_states[0] == 1 || joy_hat_states[0] == 1)
		control_x = 1;

	if (joy_left_stick_states[1] == -1 || joy_right_stick_states[1] == -1 || joy_hat_states[1] == -1 || joy_AY_buttons_state == -1)
		control_y = -1;

	if (joy_left_stick_states[1] == 1 || joy_right_stick_states[1] == 1 || joy_hat_states[1] == 1 || joy_AY_buttons_state == 1)
		control_y = 1;

	if (joy_X_button_state == 1 ||
			joy_axis[SDL_CONTROLLER_AXIS_TRIGGERLEFT] > 8000 ||
			joy_axis[SDL_CONTROLLER_AXIS_TRIGGERRIGHT] > 8000)
	{
		control_shift = -1;
	}

}

// seg000:10EA
void __pascal far draw_kid_hp(short curr_hp,short max_hp) {
	short drawn_hp_index;
	for (drawn_hp_index = curr_hp; drawn_hp_index < max_hp; ++drawn_hp_index) {
		// empty HP
		method_6_blit_img_to_scr(get_image(id_chtab_2_kid, 217), drawn_hp_index * 7, 194, blitters_0_no_transp);
	}
	for (drawn_hp_index = 0; drawn_hp_index < curr_hp; ++drawn_hp_index) {
		// full HP
		method_6_blit_img_to_scr(get_image(id_chtab_2_kid, 216), drawn_hp_index * 7, 194, blitters_0_no_transp);
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
	// CusPop: set maximum number of hitpoints (max_hitp_allowed, default = 10)
//	if (hpmax > 10) hpmax = 10; // original
	if (hpmax > custom->max_hitp_allowed) hpmax = custom->max_hitp_allowed;
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
	0x14, // sound_0_fell_to_death
	0x1E, // sound_1_falling
	0x23, // sound_2_tile_crashing
	0x66, // sound_3_button_pressed
	0x32, // sound_4_gate_closing
	0x37, // sound_5_gate_opening
	0x30, // sound_6_gate_closing_fast
	0x30, // sound_7_gate_stop
	0x4B, // sound_8_bumped
	0x50, // sound_9_grab
	0x0A, // sound_10_sword_vs_sword
	0x12, // sound_11_sword_moving
	0x0C, // sound_12_guard_hurt
	0x0B, // sound_13_kid_hurt
	0x69, // sound_14_leveldoor_closing
	0x6E, // sound_15_leveldoor_sliding
	0x73, // sound_16_medium_land
	0x78, // sound_17_soft_land
	0x7D, // sound_18_drink
	0x82, // sound_19_draw_sword
	0x91, // sound_20_loose_shake_1
	0x96, // sound_21_loose_shake_2
	0x9B, // sound_22_loose_shake_3
	0xA0, // sound_23_footstep
	0x01, // sound_24_death_regular
	0x01, // sound_25_presentation
	0x01, // sound_26_embrace
	0x01, // sound_27_cutscene_2_4_6_12
	0x01, // sound_28_death_in_fight
	0x13, // sound_29_meet_Jaffar
	0x01, // sound_30_big_potion
	0x01, // sound_31
	0x01, // sound_32_shadow_music
	0x01, // sound_33_small_potion
	0x01, // sound_34
	0x01, // sound_35_cutscene_8_9
	0x01, // sound_36_out_of_time
	0x01, // sound_37_victory
	0x01, // sound_38_blink
	0x00, // sound_39_low_weight
	0x01, // sound_40_cutscene_12_short_time
	0x01, // sound_41_end_level_music
	0x01, // sound_42
	0x01, // sound_43_victory_Jaffar
	0x87, // sound_44_skel_alive
	0x8C, // sound_45_jump_through_mirror
	0x0F, // sound_46_chomped
	0x10, // sound_47_chomper
	0x19, // sound_48_spiked
	0x16, // sound_49_spikes
	0x01, // sound_50_story_2_princess
	0x00, // sound_51_princess_door_opening
	0x01, // sound_52_story_4_Jaffar_leaves
	0x01, // sound_53_story_3_Jaffar_comes
	0x01, // sound_54_intro_music
	0x01, // sound_55_story_1_absence
	0x01, // sound_56_ending_music
	0x00
};
byte sound_pcspeaker_exists[] = {
	1, // sound_0_fell_to_death
	0, // sound_1_falling
	1, // sound_2_tile_crashing
	1, // sound_3_button_pressed
	1, // sound_4_gate_closing
	1, // sound_5_gate_opening
	1, // sound_6_gate_closing_fast
	1, // sound_7_gate_stop
	1, // sound_8_bumped
	1, // sound_9_grab
	1, // sound_10_sword_vs_sword
	0, // sound_11_sword_moving
	1, // sound_12_guard_hurt
	1, // sound_13_kid_hurt
	1, // sound_14_leveldoor_closing
	1, // sound_15_leveldoor_sliding
	1, // sound_16_medium_land
	1, // sound_17_soft_land
	1, // sound_18_drink
	0, // sound_19_draw_sword
	0, // sound_20_loose_shake_1
	0, // sound_21_loose_shake_2
	0, // sound_22_loose_shake_3
	1, // sound_23_footstep
	1, // sound_24_death_regular
	1, // sound_25_presentation
	1, // sound_26_embrace
	1, // sound_27_cutscene_2_4_6_12
	1, // sound_28_death_in_fight
	1, // sound_29_meet_Jaffar
	1, // sound_30_big_potion
	1, // sound_31
	1, // sound_32_shadow_music
	1, // sound_33_small_potion
	1, // sound_34
	1, // sound_35_cutscene_8_9
	1, // sound_36_out_of_time
	1, // sound_37_victory
	1, // sound_38_blink
	1, // sound_39_low_weight
	1, // sound_40_cutscene_12_short_time
	1, // sound_41_end_level_music
	1, // sound_42
	1, // sound_43_victory_Jaffar
	1, // sound_44_skel_alive
	1, // sound_45_jump_through_mirror
	1, // sound_46_chomped
	1, // sound_47_chomper
	1, // sound_48_spiked
	1, // sound_49_spikes
	1, // sound_50_story_2_princess
	1, // sound_51_princess_door_opening
	1, // sound_52_story_4_Jaffar_leaves
	1, // sound_53_story_3_Jaffar_comes
	1, // sound_54_intro_music
	1, // sound_55_story_1_absence
	1, // sound_56_ending_music
	0
};

void fix_sound_priorities() {
	// Change values to match those in PoP 1.3.

	// The "spiked" sound didn't interrupt the normal spikes sound when the prince ran into spikes.
	sound_interruptible[sound_49_spikes] = 1;
	sound_prio_table[sound_48_spiked] = 0x15; // moved above spikes

	// With PoP 1.3 sounds, the "guard hurt" sound didn't play when you hit a guard directly after parrying.
	sound_prio_table[sound_10_sword_vs_sword] = 0x0D; // moved below hit_user/hit_guard
}

// seg000:12C5
void __pascal far play_sound(int sound_id) {
	//printf("Would play sound %d\n", sound_id);
	if (next_sound < 0 || sound_prio_table[sound_id] <= sound_prio_table[next_sound]) {
		if (NULL == sound_pointers[sound_id]) return;
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
#ifdef USE_REPLAY
	if (replaying && skipping_replay) return 0;
#endif

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
	if (is_ending_sequence && is_paused) {
		is_paused = 0; // fix being able to pause the game during the ending sequence
	}
	if (is_paused) {
		// feather fall gets interrupted by pause
		if (fixes->fix_quicksave_during_feather &&
				is_feather_fall > 0 &&
				check_sound_playing()) {
			stop_sounds();
		}
		display_text_bottom("GAME PAUSED");
#ifdef USE_MENU
		if (enable_pause_menu || is_menu_shown) {
			draw_menu();
			menu_was_closed();
		} else
#endif
		{
			is_paused = 0;
			// busy waiting?
			do {
				idle();
				delay_ticks(1);
			} while (! process_key());
		}
		erase_bottom_text(1);
	}
	return key || control_shift;
}

// seg000:1500
void __pascal far read_keyb_control() {

	if (key_states[SDL_SCANCODE_UP] || key_states[SDL_SCANCODE_HOME] || key_states[SDL_SCANCODE_PAGEUP]
	    || key_states[SDL_SCANCODE_KP_8] || key_states[SDL_SCANCODE_KP_7] || key_states[SDL_SCANCODE_KP_9]
	) {
		control_y = -1;
	} else if (key_states[SDL_SCANCODE_CLEAR] || key_states[SDL_SCANCODE_DOWN]
	           || key_states[SDL_SCANCODE_KP_5] || key_states[SDL_SCANCODE_KP_2]
	) {
		control_y = 1;
	}
	if (key_states[SDL_SCANCODE_LEFT] || key_states[SDL_SCANCODE_HOME]
	    || key_states[SDL_SCANCODE_KP_4] || key_states[SDL_SCANCODE_KP_7]
	) {
		control_x = -1;
	} else if (key_states[SDL_SCANCODE_RIGHT] || key_states[SDL_SCANCODE_PAGEUP]
	           || key_states[SDL_SCANCODE_KP_6] || key_states[SDL_SCANCODE_KP_9]
	) {
		control_x = 1;
	}
	control_shift = -(key_states[SDL_SCANCODE_LSHIFT] || key_states[SDL_SCANCODE_RSHIFT]);

	#ifdef USE_DEBUG_CHEATS
	if (cheats_enabled && debug_cheats_enabled) {
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
#ifdef USE_LIGHTING
	update_lighting(target_rect_ptr);
#endif
}

// seg000:15E9
void __pascal far toggle_upside() {
	upside_down = ~ upside_down;
	need_redraw_because_flipped = 1;
}

// seg000:15F8
void __pascal far feather_fall() {
	//printf("slow fall started at: rem_min = %d, rem_tick = %d\n", rem_min, rem_tick);
	if (fixes->fix_quicksave_during_feather) {
		// feather fall is treated as a timer
		is_feather_fall = FEATHER_FALL_LENGTH * get_ticks_per_sec(timer_1);
	} else {
		is_feather_fall = 1;
	}
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
	load_opt_sounds(sound_50_story_2_princess, sound_55_story_1_absence); // main theme, story, princess door
	dont_reset_time = 0;
	if(offscreen_surface) free_surface(offscreen_surface); // missing in original
	offscreen_surface = make_offscreen_buffer(&screen_rect);
	load_title_images(1);
	current_target_surface = offscreen_surface;
	idle(); // modified
	do_paused();

	draw_full_image(TITLE_MAIN);
	fade_in_2(offscreen_surface, 0x1000); //STUB
	method_1_blit_rect(onscreen_surface_, offscreen_surface, &screen_rect, &screen_rect, blitters_0_no_transp);
	current_sound = sound_54_intro_music; // added
	play_sound_from_buffer(sound_pointers[sound_54_intro_music]); // main theme
	start_timer(timer_0, 0x82);
	draw_full_image(TITLE_PRESENTS);
	do_wait(timer_0);

	start_timer(timer_0, 0xCD);
	method_1_blit_rect(onscreen_surface_, offscreen_surface, &rect_titles, &rect_titles, blitters_0_no_transp);
	draw_full_image(TITLE_MAIN);
	do_wait(timer_0);

	start_timer(timer_0, 0x41);
	method_1_blit_rect(onscreen_surface_, offscreen_surface, &rect_titles, &rect_titles, blitters_0_no_transp);
	draw_full_image(TITLE_MAIN);
	draw_full_image(TITLE_GAME);
	do_wait(timer_0);

	start_timer(timer_0, 0x10E);
	method_1_blit_rect(onscreen_surface_, offscreen_surface, &rect_titles, &rect_titles, blitters_0_no_transp);
	draw_full_image(TITLE_MAIN);
	do_wait(timer_0);

	start_timer(timer_0, 0xEB);
	method_1_blit_rect(onscreen_surface_, offscreen_surface, &rect_titles, &rect_titles, blitters_0_no_transp);
	draw_full_image(TITLE_MAIN);
	draw_full_image(TITLE_POP);
	draw_full_image(TITLE_MECHNER);
	do_wait(timer_0);

	method_1_blit_rect(onscreen_surface_, offscreen_surface, &rect_titles, &rect_titles, blitters_0_no_transp);
	draw_full_image(STORY_FRAME);
	draw_full_image(STORY_ABSENCE);
	current_target_surface = onscreen_surface_;
	while (check_sound_playing()) {
		idle();
		do_paused();
		delay_ticks(1);
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
	draw_full_image(STORY_FRAME);
	draw_full_image(STORY_MARRY);
	fade_in_2(offscreen_surface, 0x800);
	draw_full_image(TITLE_MAIN);
	draw_full_image(TITLE_POP);
	draw_full_image(TITLE_MECHNER);
	while (check_sound_playing()) {
		idle();
		do_paused();
		delay_ticks(1);
	}
	transition_ltr();
	pop_wait(timer_0, 0x78);
	draw_full_image(STORY_FRAME);
	draw_full_image(STORY_CREDITS);
	transition_ltr();
	pop_wait(timer_0, 0x168);
	if (hof_count) {
		draw_full_image(STORY_FRAME);
		draw_full_image(HOF_POP);
		show_hof();
		transition_ltr();
		pop_wait(timer_0, 0xF0);
	}
	current_target_surface = onscreen_surface_;
	while (check_sound_playing()) {
		idle();
		do_paused();
		delay_ticks(1);
	}
	fade_out_2(0x1800);
	free_surface(offscreen_surface);
	offscreen_surface = NULL; // added
	release_title_images();
	init_game(0);
}

Uint64 last_transition_counter;

// seg000:1BB3
void __pascal far transition_ltr() {
	short position;
	rect_type rect;
	rect.top = 0;
	rect.bottom = 200;
	rect.left = 0;
	rect.right = 2;
	// Estimated transition fps based on the speed of the transition on an Apple IIe.
	// See: https://www.youtube.com/watch?v=7m7j2VuWhQ0
	int transition_fps = 120;
#ifdef USE_FAST_FORWARD
	extern int audio_speed;
	transition_fps *= audio_speed;
#endif
	Uint64 counters_per_frame = perf_frequency / transition_fps;
	last_transition_counter = SDL_GetPerformanceCounter();
	int overshoot = 0;
	for (position = 0; position < 320; position += 2) {
		method_1_blit_rect(onscreen_surface_, offscreen_surface, &rect, &rect, 0);
		rect.left += 2;
		rect.right += 2;
		if (overshoot > 0 && overshoot < 10) {
			--overshoot;
			continue; // On slow systems (e.g. Raspberry Pi), allow the animation to catch up, before refreshing the screen.
		}
		idle(); // modified
		do_paused();
		// Add an appropriate delay until the next frame, so that the animation isn't instantaneous on fast CPUs.
		for (;;) {
			Uint64 current_counter = SDL_GetPerformanceCounter();
			int frametimes_elapsed = (int)((current_counter / counters_per_frame) - (last_transition_counter / counters_per_frame));
			if (frametimes_elapsed > 0) {
				overshoot = frametimes_elapsed - 1;
				last_transition_counter = current_counter;
				break; // Proceed to the next frame.
			} else {
				SDL_Delay(1);
			}
		}

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
void __pascal far draw_full_image(enum full_image_id id) {
	image_type* decoded_image;
	image_type* mask = NULL;
	int xpos, ypos, blit;

	if (id >= MAX_FULL_IMAGES) return;
	if (NULL == *full_image[id].chtab) return;
	decoded_image = (*full_image[id].chtab)->images[full_image[id].id];
	blit = full_image[id].blitter;
	xpos = full_image[id].xpos;
	ypos = full_image[id].ypos;

	switch (blit) {
	case blitters_white:
		blit = get_text_color(15, color_15_brightwhite, 0x800);
		/* fall through */
	default:
		method_3_blit_mono(decoded_image, xpos, ypos, blitters_0_no_transp, blit);
		break;
	case blitters_10h_transp:
		if (graphics_mode == gmCga || graphics_mode == gmHgaHerc) {
			//...
		} else {
			mask = decoded_image;
		}
		draw_image_transp(decoded_image, mask, xpos, ypos);
		if (graphics_mode == gmCga || graphics_mode == gmHgaHerc) {
			free_far(mask);
		}
		break;
	case blitters_0_no_transp:
		method_6_blit_img_to_scr(decoded_image, xpos, ypos, blit);
		break;
	}
}

// seg000:1D2C
void __pascal far load_kid_sprite() {
	load_chtab_from_file(id_chtab_2_kid, 400, "KID.DAT", 1<<7);
}

const char* save_file = "PRINCE.SAV";

const char* get_save_path(char* custom_path_buffer, size_t max_len) {
	if (!use_custom_levelset) {
		return save_file;
	}
	// if playing a custom levelset, try to use the mod folder
	snprintf_check(custom_path_buffer, max_len, "%s/%s", mod_data_path, save_file /*PRINCE.SAV*/ );
	return custom_path_buffer;
}

// seg000:1D45
void __pascal far save_game() {
	word success;
	FILE* handle;
	success = 0;
	char custom_save_path[POP_MAX_PATH];
	const char* save_path = get_save_path(custom_save_path, sizeof(custom_save_path));
	handle = fopen(save_path, "wb");
	if (handle == NULL) goto loc_1DB8;
	if (fwrite(&rem_min, 1, 2, handle) == 2) goto loc_1DC9;
	loc_1D9B:
	fclose(handle);
	if (!success) {
		remove(save_path);
	}
	loc_1DB8:
	if (!success) goto loc_1E18;
	display_text_bottom("GAME SAVED");
	goto loc_1E2E;
	loc_1DC9:
	if (fwrite(&rem_tick, 1, 2, handle) != 2) goto loc_1D9B;
	if (fwrite(&current_level, 1, 2, handle) != 2) goto loc_1D9B;
	if (fwrite(&hitp_beg_lev, 1, 2, handle) != 2) goto loc_1D9B;
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
	word success;
	FILE* handle;
	success = 0;
	char custom_save_path[POP_MAX_PATH];
	const char* save_path = get_save_path(custom_save_path, sizeof(custom_save_path));
	handle = fopen(save_path, "rb");
	if (handle == NULL) goto loc_1E99;
	if (fread(&rem_min, 1, 2, handle) == 2) goto loc_1E9E;
	loc_1E8E:
	fclose(handle);
	loc_1E99:
	return success;
	loc_1E9E:
	if (fread(&rem_tick, 1, 2, handle) != 2) goto loc_1E8E;
	if (fread(&start_level, 1, 2, handle) != 2) goto loc_1E8E;
	if (fread(&hitp_beg_lev, 1, 2, handle) != 2) goto loc_1E8E;
#ifdef USE_COPYPROT
	if (enable_copyprot && custom->copyprot_level > 0) {
		custom->copyprot_level = start_level;
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

	is_cutscene = 0;
	is_ending_sequence = false; // added
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
	if (check_param("stdsnd")) {
		// Use PC Speaker sounds and music.
	} else {
		// Use digi (wave) sounds and MIDI music.
		sound_flags |= sfDigi;
		sound_flags |= sfMidi;
		sound_mode = smSblast;
	}
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

void free_all_sounds() {
	for (int i = 0; i < 58; ++i) {
		free_sound(sound_pointers[i]);
		sound_pointers[i] = NULL;
	}
}

void load_all_sounds() {
	if (!use_custom_levelset) {
		load_sounds(0, 43);
		load_opt_sounds(43, 56); //added
	} else {
		// First load any sounds included in the mod folder...
		skip_normal_data_files = true;
		load_sounds(0, 43);
		load_opt_sounds(43, 56);
		skip_normal_data_files = false;
		// ... then load any missing sounds from SDLPoP's own resources.
		skip_mod_data_files = true;
		load_sounds(0, 43);
		load_opt_sounds(43, 56);
		skip_mod_data_files = false;
	}
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
		if (NULL != chtab_title40) {
			SDL_SetPaletteColors(chtab_title40->images[0]->format->palette, &color, 14, 1);
		}
	} else if (graphics_mode == gmEga || graphics_mode == gmTga) {
		// ...
	}
}

#ifdef USE_COPYPROT
// data:017A
const word copyprot_word[] = {9, 1, 6, 4, 5, 3, 6, 3, 4, 4, 3, 2,12, 5,13, 1, 9, 2, 2, 4, 9, 4,11, 8, 5, 4, 1, 6, 2, 4, 6, 8, 4, 2, 7,11, 5, 4, 1, 2};
// data:012A
const word copyprot_line[] = {2, 1, 5, 4, 3, 5, 1, 3, 7, 2, 2, 4, 6, 6, 2, 6, 3, 1, 2, 3, 2, 2, 3,10, 5, 6, 5, 6, 3, 5, 7, 2, 2, 4, 5, 7, 2, 6, 5, 5};
// data:00DA
const word copyprot_page[] = {5, 3, 7, 3, 3, 4, 1, 5,12, 5,11,10, 1, 2, 8, 8, 2, 4, 6, 1, 4, 7, 3, 2, 1, 7,10, 1, 4, 3, 4, 1, 4, 1, 8, 1, 1,10, 3, 3};
#endif

// seg000:23F4
void __pascal far show_copyprot(int where) {
#ifdef USE_COPYPROT
	char sprintf_temp[140];
	if (current_level != 15) return;
	if (where) {
		if (text_time_remaining || is_cutscene) return;
		text_time_total = 1188;
		text_time_remaining = 1188;
		is_show_time = 0;
		snprintf(sprintf_temp, sizeof(sprintf_temp),
			"WORD %d LINE %d PAGE %d",
			copyprot_word[copyprot_idx], copyprot_line[copyprot_idx], copyprot_page[copyprot_idx]);
		display_text_bottom(sprintf_temp);
	} else {
		snprintf(sprintf_temp, sizeof(sprintf_temp),
			"Drink potion matching the first letter of Word %d on Line %d\n"
			"of Page %d of the manual.",
			copyprot_word[copyprot_idx], copyprot_line[copyprot_idx], copyprot_page[copyprot_idx]);
		show_dialog(sprintf_temp);
	}
#endif
}

// seg000:2489
void __pascal far show_loading() {
	show_text(&screen_rect, 0, 0, "Loading. . . .");
	update_screen();
}

// data:42C4
word which_quote;

char const * const tbl_quotes[2] = {
"\"(****/****) Incredibly realistic. . . The "
"adventurer character actually looks human as he "
"runs, jumps, climbs, and hangs from ledges.\"\n"
"\n"
"                                  Computer Entertainer\n"
"\n"
"\n"
"\n"
"\n"
"\"A tremendous achievement. . . Mechner has crafted "
"the smoothest animation ever seen in a game of this "
"type.\n"
"\n"
"\"PRINCE OF PERSIA is the STAR WARS of its field.\"\n"
"\n"
"                                  Computer Gaming World",
"\"An unmitigated delight. . . comes as close to "
"(perfection) as any arcade game has come in a long, "
"long time. . . what makes this game so wonderful (am "
"I gushing?) is that the little onscreen character "
"does not move like a little onscreen character -- he "
"moves like a person.\"\n"
"\n"
"                                      Nibble"
};

// seg000:249D
void __pascal far show_quotes() {
	//start_timer(timer_0,0);
	//remove_timer(timer_0);
	if (demo_mode && need_quotes) {
		draw_rect(&screen_rect, 0);
		show_text(&screen_rect, -1, 0, tbl_quotes[which_quote]);
		which_quote = !which_quote;
		start_timer(timer_0, 0x384);
	}
	need_quotes = 0;
}

const rect_type splash_text_1_rect = {0, 0, 50, 320};
const rect_type splash_text_2_rect = {50, 0, 200, 320};

const char* splash_text_1 = "SDLPoP " SDLPOP_VERSION;
const char* splash_text_2 =
#ifdef USE_QUICKSAVE
		"To quick save/load, press F6/F9 in-game.\n"
		"\n"
#endif
#ifdef USE_REPLAY
		"To record replays, press Ctrl+Tab in-game.\n"
		"To view replays, press Tab on the title screen.\n"
		"\n"
#endif
		"Edit SDLPoP.ini to customize SDLPoP.\n"
		"Mods also work with SDLPoP.\n"
		"\n"
		"For more information, read doc/Readme.txt.\n"
		"Questions? Visit https://forum.princed.org\n"
		"\n"
		"Press any key to continue...";

void show_splash() {
	if (!enable_info_screen || start_level >= 0) return;
	current_target_surface = onscreen_surface_;
	draw_rect(&screen_rect, 0);
	show_text_with_color(&splash_text_1_rect, 0, 0, splash_text_1, color_15_brightwhite);
	show_text_with_color(&splash_text_2_rect, 0, -1, splash_text_2, color_7_lightgray);

#ifdef USE_TEXT // Don't wait for a keypress if there is no text for the user to read.
	int key = 0;
	do {
		idle();
		key = key_test_quit();

		if (joy_hat_states[0] != 0 || joy_X_button_state != 0 || joy_AY_buttons_state != 0 || joy_B_button_state != 0) {
			joy_hat_states[0] = 0;
			joy_AY_buttons_state = 0;
			joy_X_button_state = 0;
			joy_B_button_state = 0;
			key_states[SDL_SCANCODE_LSHIFT] = 1; // close the splash screen using the gamepad
		}
		delay_ticks(1);

	} while(key == 0 && !(key_states[SDL_SCANCODE_LSHIFT] || key_states[SDL_SCANCODE_RSHIFT]));

	if ((key & WITH_CTRL) || (enable_quicksave && key == SDL_SCANCODE_F9) || (enable_replay && key == SDL_SCANCODE_TAB)) {
		extern int last_key_scancode; // defined in seg009.c
		last_key_scancode = key; // can immediately do Ctrl+L, etc from the splash screen
	}
	key_states[SDL_SCANCODE_LSHIFT] = 0; // don't immediately start the game if Shift was pressed!
	key_states[SDL_SCANCODE_RSHIFT] = 0;
#endif
}

