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
#include <ctype.h>
#include <inttypes.h>


void turn_fixes_and_enhancements_on_off(byte new_state) {
	use_fixes_and_enhancements = new_state;
	fixes = (new_state) ? &fixes_saved : &fixes_disabled_state;
}

void turn_custom_options_on_off(byte new_state) {
	use_custom_options = new_state;
	custom = (new_state) ? &custom_saved : &custom_defaults;
}

// .ini file parser adapted from https://gist.github.com/OrangeTide/947070
/* Load an .ini format file
 * filename - path to a file
 * report - callback can return non-zero to stop, the callback error code is
 *     returned from this function.
 * return - return 0 on success
 */
int ini_load(const char *filename,
             int (*report)(const char *section, const char *name, const char *value))
{
	char name[64];
	char value[256];
	char section[128] = "";
	char *s;
	FILE *f;
	int cnt;

	f = fopen(filename, "r");
	if (!f) {
		return -1;
	}

	while (!feof(f)) {
		if (fscanf(f, "[%127[^];\n]]\n", section) == 1) {
		} else if ((cnt = fscanf(f, " %63[^=;\n] = %255[^;\n]", name, value))) {
			if (cnt == 1)
				*value = 0;
			for (s = name + strlen(name) - 1; s > name && isspace(*s); s--)
				*s = 0;
			for (s = value + strlen(value) - 1; s > value && isspace(*s); s--)
				*s = 0;
			report(section, name, value);
		}
		if (fscanf(f, " ;%*[^\n]") != 0 ||
		    fscanf(f, " \n") != 0) {
			fprintf(stderr, "short read from %s!?\n", filename);
			fclose(f);
			return -1;
		}
	}

	fclose(f);
	return 0;
}

NAMES_LIST(level_type_names, {"dungeon", "palace"});
//NAMES_LIST(guard_type_names, {"guard", "fat", "skel", "vizier", "shadow"});
// NAMES_LIST must start from 0, so I need KEY_VALUE_LIST if I want to assign a name to -1.
KEY_VALUE_LIST(guard_type_names, {{"none", -1}, {"guard", 0}, {"fat", 1}, {"skel", 2}, {"vizier", 3}, {"shadow", 4}});
NAMES_LIST(tile_type_names, {
				"empty", "floor", "spike", "pillar", "gate",                                        // 0..4
				"stuck", "closer", "doortop_with_floor", "bigpillar_bottom", "bigpillar_top",       // 5..9
				"potion", "loose", "doortop", "mirror", "debris",                                   // 10..14
				"opener", "level_door_left", "level_door_right", "chomper", "torch",                // 15..19
				"wall", "skeleton", "sword", "balcony_left", "balcony_right",                       // 20..24
				"lattice_pillar", "lattice_down", "lattice_small", "lattice_left", "lattice_right", // 25..29
				"torch_with_debris", // 30
});
NAMES_LIST(scaling_type_names, {"sharp", "fuzzy", "blurry"});
NAMES_LIST(row_names, {"top", "middle", "bottom"});
KEY_VALUE_LIST(direction_names, {{"left", dir_FF_left}, {"right", dir_0_right}});
NAMES_LIST(entry_pose_names, {"turning", "falling", "running"});
// 16 is higher than any level, so some options can be disabled by setting it to this value
KEY_VALUE_LIST(never_is_16, {{"Never", 16}});

#define INI_NO_VALID_NAME (-9999)

static int ini_get_named_value(const char* value, names_list_type* value_names) {
	if (value_names != NULL) {
		if (value_names->type == 0 /*names list*/) {
			char *base_ptr = (char *) value_names->names.data;
			for (int i = 0; i < value_names->names.count; ++i) {
				char *name = (base_ptr + i * MAX_OPTION_VALUE_NAME_LENGTH);
				if (strcasecmp(value, name) == 0) return i;
			}
		} else if (value_names->type == 1 /*key/value list*/) {
			for (int i = 0; i < value_names->kv_pairs.count; ++i) {
				key_value_type* kv_pair = value_names->kv_pairs.data + i;
				if (strcasecmp(value, kv_pair->key) == 0) return kv_pair->value;
			}
		}
	}
	return INI_NO_VALID_NAME; // failure
}

static int ini_process_boolean(const char* curr_name, const char* value, const char* option_name, byte* target) {
	if(strcasecmp(curr_name, option_name) == 0) {
		if (strcasecmp(value, "true") == 0) *target = 1;
		else if (strcasecmp(value, "false") == 0) *target = 0;
		return 1; // finished; don't look for more possible options that curr_name can be
	}
	return 0; // not the right option; should check another option_name
}

#define ini_process_numeric_func(data_type) \
static int ini_process_##data_type(const char* curr_name, const char* value, const char* option_name, data_type* target, names_list_type* value_names) { \
	if(strcasecmp(curr_name, option_name) == 0) { \
		if (strcasecmp(value, "default") != 0) { \
			int named_value = ini_get_named_value(value, value_names); \
			*target = (named_value == INI_NO_VALID_NAME) ? ((data_type) strtoimax(value, NULL, 0)) : ((data_type) named_value); \
		} \
		return 1; /* finished; don't look for more possible options that curr_name can be */ \
	} \
	return 0; /* not the right option; should check another option_name */ \
}
ini_process_numeric_func(word)
ini_process_numeric_func(short)
ini_process_numeric_func(byte)
ini_process_numeric_func(sbyte)
ini_process_numeric_func(int)

static int global_ini_callback(const char *section, const char *name, const char *value)
{
	//fprintf(stdout, "[%s] '%s'='%s'\n", section, name, value);

	#define check_ini_section(section_name)    (strcasecmp(section, section_name) == 0)

	// Make sure that we return successfully as soon as name matches the correct option_name
	#define process_word(option_name, target, value_names)                           \
	if (ini_process_word(name, value, option_name, target, value_names)) return 1;

	#define process_short(option_name, target, value_names)                           \
	if (ini_process_short(name, value, option_name, target, value_names)) return 1;

	#define process_byte(option_name, target, value_names)                           \
	if (ini_process_byte(name, value, option_name, target, value_names)) return 1;

	#define process_sbyte(option_name, target, value_names)                           \
	if (ini_process_sbyte(name, value, option_name, target, value_names)) return 1;

	#define process_int(option_name, target, value_names)                           \
	if (ini_process_int(name, value, option_name, target, value_names)) return 1;

	#define process_boolean(option_name, target)                        \
	if (ini_process_boolean(name, value, option_name, target)) return 1;

	if (check_ini_section("General")) {
#ifdef USE_MENU
		process_boolean("enable_pause_menu", &enable_pause_menu);
#endif
		if (strcasecmp(name, "mods_folder") == 0) {
			if (value[0] != '\0' && strcasecmp(value, "default") != 0) {
				strcpy(mods_folder, locate_file(value));
			}
			return 1;
		}
		process_boolean("enable_copyprot", &enable_copyprot);
		process_boolean("enable_music", &enable_music);
		process_boolean("enable_fade", &enable_fade);
		process_boolean("enable_flash", &enable_flash);
		process_boolean("enable_text", &enable_text);
		process_boolean("enable_info_screen", &enable_info_screen);
		process_boolean("start_fullscreen", &start_fullscreen);
		process_word("pop_window_width", &pop_window_width, NULL);
		process_word("pop_window_height", &pop_window_height, NULL);
		process_boolean("use_correct_aspect_ratio", &use_correct_aspect_ratio);
		process_boolean("use_integer_scaling", &use_integer_scaling);
		process_byte("scaling_type", &scaling_type, &scaling_type_names_list);
		process_boolean("enable_controller_rumble", &enable_controller_rumble);
		process_boolean("joystick_only_horizontal", &joystick_only_horizontal);
		process_int("joystick_threshold", &joystick_threshold, NULL);

		if (strcasecmp(name, "levelset") == 0) {
			if (value[0] == '\0' || strcasecmp(value, "original") == 0 || strcasecmp(value, "default") == 0) {
				use_custom_levelset = 0;
			} else {
				use_custom_levelset = 1;
				strcpy(levelset_name, value);
			}
			return 1;
		}

		if (strcasecmp(name, "gamecontrollerdb_file") == 0) {
			if (value[0] != '\0') {
				strcpy(gamecontrollerdb_file, locate_file(value));
			}
			return 1;
		}
	}

	if (check_ini_section("AdditionalFeatures")) {
		process_boolean("enable_quicksave", &enable_quicksave);
		process_boolean("enable_quicksave_penalty", &enable_quicksave_penalty);

#ifdef USE_REPLAY
		process_boolean("enable_replay", &enable_replay);

		if (strcasecmp(name, "replays_folder") == 0) {
			if (value[0] != '\0' && strcasecmp(value, "default") != 0) {
				strcpy(replays_folder, locate_file(value));
			}
			return 1;
		}
#endif
#ifdef USE_LIGHTING
		process_boolean("enable_lighting", &enable_lighting);
#endif
	}

	if (check_ini_section("Enhancements")) {
		if (strcasecmp(name, "use_fixes_and_enhancements") == 0) {
			if (strcasecmp(value, "true") == 0) use_fixes_and_enhancements = 1;
			else if (strcasecmp(value, "false") == 0) use_fixes_and_enhancements = 0;
			else if (strcasecmp(value, "prompt") == 0) use_fixes_and_enhancements = 2;
			return 1;
		}
		process_boolean("enable_crouch_after_climbing", &fixes_saved.enable_crouch_after_climbing);
		process_boolean("enable_freeze_time_during_end_music", &fixes_saved.enable_freeze_time_during_end_music);
		process_boolean("enable_remember_guard_hp", &fixes_saved.enable_remember_guard_hp);
		process_boolean("fix_gate_sounds", &fixes_saved.fix_gate_sounds);
		process_boolean("fix_two_coll_bug", &fixes_saved.fix_two_coll_bug);
		process_boolean("fix_infinite_down_bug", &fixes_saved.fix_infinite_down_bug);
		process_boolean("fix_gate_drawing_bug", &fixes_saved.fix_gate_drawing_bug);
		process_boolean("fix_bigpillar_climb", &fixes_saved.fix_bigpillar_climb);
		process_boolean("fix_jump_distance_at_edge", &fixes_saved.fix_jump_distance_at_edge);
		process_boolean("fix_edge_distance_check_when_climbing", &fixes_saved.fix_edge_distance_check_when_climbing);
		process_boolean("fix_painless_fall_on_guard", &fixes_saved.fix_painless_fall_on_guard);
		process_boolean("fix_wall_bump_triggers_tile_below", &fixes_saved.fix_wall_bump_triggers_tile_below);
		process_boolean("fix_stand_on_thin_air", &fixes_saved.fix_stand_on_thin_air);
		process_boolean("fix_press_through_closed_gates", &fixes_saved.fix_press_through_closed_gates);
		process_boolean("fix_grab_falling_speed", &fixes_saved.fix_grab_falling_speed);
		process_boolean("fix_skeleton_chomper_blood", &fixes_saved.fix_skeleton_chomper_blood);
		process_boolean("fix_move_after_drink", &fixes_saved.fix_move_after_drink);
		process_boolean("fix_loose_left_of_potion", &fixes_saved.fix_loose_left_of_potion);
		process_boolean("fix_guard_following_through_closed_gates", &fixes_saved.fix_guard_following_through_closed_gates);
		process_boolean("fix_safe_landing_on_spikes", &fixes_saved.fix_safe_landing_on_spikes);
		process_boolean("fix_glide_through_wall", &fixes_saved.fix_glide_through_wall);
		process_boolean("fix_drop_through_tapestry", &fixes_saved.fix_drop_through_tapestry);
		process_boolean("fix_land_against_gate_or_tapestry", &fixes_saved.fix_land_against_gate_or_tapestry);
		process_boolean("fix_unintended_sword_strike", &fixes_saved.fix_unintended_sword_strike);
		process_boolean("fix_retreat_without_leaving_room", &fixes_saved.fix_retreat_without_leaving_room);
		process_boolean("fix_running_jump_through_tapestry", &fixes_saved.fix_running_jump_through_tapestry);
		process_boolean("fix_push_guard_into_wall", &fixes_saved.fix_push_guard_into_wall);
		process_boolean("fix_jump_through_wall_above_gate", &fixes_saved.fix_jump_through_wall_above_gate);
		process_boolean("fix_chompers_not_starting", &fixes_saved.fix_chompers_not_starting);
		process_boolean("fix_feather_interrupted_by_leveldoor", &fixes_saved.fix_feather_interrupted_by_leveldoor);
		process_boolean("fix_offscreen_guards_disappearing", &fixes_saved.fix_offscreen_guards_disappearing);
		process_boolean("fix_move_after_sheathe", &fixes_saved.fix_move_after_sheathe);
		process_boolean("fix_hidden_floors_during_flashing", &fixes_saved.fix_hidden_floors_during_flashing);
		process_boolean("fix_hang_on_teleport", &fixes_saved.fix_hang_on_teleport);
		process_boolean("fix_exit_door", &fixes_saved.fix_exit_door);
		process_boolean("fix_quicksave_during_feather", &fixes_saved.fix_quicksave_during_feather);
		process_boolean("fix_caped_prince_sliding_through_gate", &fixes_saved.fix_caped_prince_sliding_through_gate);
		process_boolean("fix_doortop_disabling_guard", &fixes_saved.fix_doortop_disabling_guard);
		process_boolean("enable_super_high_jump", &fixes_saved.enable_super_high_jump);
	}

	if (check_ini_section("CustomGameplay")) {
		process_boolean("use_custom_options", &use_custom_options);
		process_word("start_minutes_left", &custom_saved.start_minutes_left, NULL);
		process_word("start_ticks_left", &custom_saved.start_ticks_left, NULL);
		process_word("start_hitp", &custom_saved.start_hitp, NULL);
		process_word("max_hitp_allowed", &custom_saved.max_hitp_allowed, NULL);
		process_word("saving_allowed_first_level", &custom_saved.saving_allowed_first_level, &never_is_16_list);
		process_word("saving_allowed_last_level", &custom_saved.saving_allowed_last_level, &never_is_16_list);
		process_boolean("start_upside_down", &custom_saved.start_upside_down);
		process_boolean("start_in_blind_mode", &custom_saved.start_in_blind_mode);
		process_word("copyprot_level", &custom_saved.copyprot_level, &never_is_16_list);
		process_byte("drawn_tile_top_level_edge", &custom_saved.drawn_tile_top_level_edge, &tile_type_names_list);
		process_byte("drawn_tile_left_level_edge", &custom_saved.drawn_tile_left_level_edge, &tile_type_names_list);
		process_byte("level_edge_hit_tile", &custom_saved.level_edge_hit_tile, &tile_type_names_list);
		process_boolean("allow_triggering_any_tile", &custom_saved.allow_triggering_any_tile);
		// TODO: Maybe allow automatically choosing the correct WDA, depending on the loaded VDUNGEON.DAT?
		process_boolean("enable_wda_in_palace", &custom_saved.enable_wda_in_palace);

		// Options that change the hard-coded color palette (options 'vga_color_0', 'vga_color_1', ...)
		static const char prefix[] = "vga_color_";
		static const size_t prefix_len = sizeof(prefix)-1;
		int ini_palette_color = -1;
		if (strncasecmp(name, prefix, prefix_len) == 0 && sscanf(name+prefix_len, "%d", &ini_palette_color) == 1) {
			if (!(ini_palette_color >= 0 && ini_palette_color <= 15)) return 0;

			byte rgb[3] = {0};
			if (strcasecmp(value, "default") != 0) {
				// We want to parse an rgb string with three entries like this: "255, 255, 255"
				char* start = (char*) value;
				char* end   = (char*) value;
				int i;
				for (i = 0; i < 3 && *end != '\0'; ++i) {
					rgb[i] = (byte) strtol(start, &end, 0); // convert this entry into a number 0..255

					while (*end == ',' || *end == ' ') {
						++end; // skip delimiter characters or whitespace
					}
					start = end; // start parsing the next entry here
				}
			}
			rgb_type* palette_color = &custom_saved.vga_palette[ini_palette_color];
			palette_color->r = rgb[0] / 4; // the palette uses values 0..63, not 0..255
			palette_color->g = rgb[1] / 4;
			palette_color->b = rgb[2] / 4;
			return 1;
		}
		process_word("first_level", &custom_saved.first_level, NULL);
		process_boolean("skip_title", &custom_saved.skip_title);
		process_word("shift_L_allowed_until_level", &custom_saved.shift_L_allowed_until_level, &never_is_16_list);
		process_word("shift_L_reduced_minutes", &custom_saved.shift_L_reduced_minutes, NULL);
		process_word("shift_L_reduced_ticks", &custom_saved.shift_L_reduced_ticks, NULL);
		process_word("demo_hitp", &custom_saved.demo_hitp, NULL);
		process_word("demo_end_room", &custom_saved.demo_end_room, NULL);
		process_word("intro_music_level", &custom_saved.intro_music_level, &never_is_16_list);
		process_word("have_sword_from_level", &custom_saved.have_sword_from_level, &never_is_16_list);
		process_word("checkpoint_level", &custom_saved.checkpoint_level, &never_is_16_list);
		process_sbyte("checkpoint_respawn_dir", &custom_saved.checkpoint_respawn_dir, &direction_names_list);
		process_byte("checkpoint_respawn_room", &custom_saved.checkpoint_respawn_room, NULL);
		process_byte("checkpoint_respawn_tilepos", &custom_saved.checkpoint_respawn_tilepos, NULL);
		process_byte("checkpoint_clear_tile_room", &custom_saved.checkpoint_clear_tile_room, NULL);
		process_byte("checkpoint_clear_tile_col", &custom_saved.checkpoint_clear_tile_col, NULL);
		process_byte("checkpoint_clear_tile_row", &custom_saved.checkpoint_clear_tile_row, &row_names_list);
		process_word("skeleton_level", &custom_saved.skeleton_level, &never_is_16_list);
		process_byte("skeleton_room", &custom_saved.skeleton_room, NULL);
		process_byte("skeleton_trigger_column_1", &custom_saved.skeleton_trigger_column_1, NULL);
		process_byte("skeleton_trigger_column_2", &custom_saved.skeleton_trigger_column_2, NULL);
		process_byte("skeleton_column", &custom_saved.skeleton_column, NULL);
		process_byte("skeleton_row", &custom_saved.skeleton_row, &row_names_list);
		process_boolean("skeleton_require_open_level_door", &custom_saved.skeleton_require_open_level_door);
		process_byte("skeleton_skill", &custom_saved.skeleton_skill, NULL);
		process_byte("skeleton_reappear_room", &custom_saved.skeleton_reappear_room, NULL);
		process_byte("skeleton_reappear_x", &custom_saved.skeleton_reappear_x, NULL);
		process_byte("skeleton_reappear_row", &custom_saved.skeleton_reappear_row, &row_names_list);
		process_byte("skeleton_reappear_dir", &custom_saved.skeleton_reappear_dir, &direction_names_list);
		process_word("mirror_level", &custom_saved.mirror_level, &never_is_16_list);
		process_byte("mirror_room", &custom_saved.mirror_room, NULL);
		process_byte("mirror_column", &custom_saved.mirror_column, NULL);
		process_byte("mirror_row", &custom_saved.mirror_row, &row_names_list);
		process_byte("mirror_tile", &custom_saved.mirror_tile, &tile_type_names_list);
		process_boolean("show_mirror_image", &custom_saved.show_mirror_image);
		process_word("falling_exit_level", &custom_saved.falling_exit_level, &never_is_16_list);
		process_byte("falling_exit_room", &custom_saved.falling_exit_room, NULL);
		process_word("falling_entry_level", &custom_saved.falling_entry_level, &never_is_16_list);
		process_byte("falling_entry_room", &custom_saved.falling_entry_room, NULL);
		process_word("mouse_level", &custom_saved.mouse_level, &never_is_16_list);
		process_byte("mouse_room", &custom_saved.mouse_room, NULL);
		process_word("mouse_delay", &custom_saved.mouse_delay, NULL);
		process_byte("mouse_object", &custom_saved.mouse_object, NULL);
		process_byte("mouse_start_x", &custom_saved.mouse_start_x, NULL);
		process_word("loose_tiles_level", &custom_saved.loose_tiles_level, &never_is_16_list);
		process_byte("loose_tiles_room_1", &custom_saved.loose_tiles_room_1, NULL);
		process_byte("loose_tiles_room_2", &custom_saved.loose_tiles_room_2, NULL);
		process_byte("loose_tiles_first_tile", &custom_saved.loose_tiles_first_tile, NULL);
		process_byte("loose_tiles_last_tile", &custom_saved.loose_tiles_last_tile, NULL);
		process_word("jaffar_victory_level", &custom_saved.jaffar_victory_level, &never_is_16_list);
		process_byte("jaffar_victory_flash_time", &custom_saved.jaffar_victory_flash_time, NULL);
		process_word("hide_level_number_from_level", &custom_saved.hide_level_number_from_level, &never_is_16_list);
		process_byte("level_13_level_number", &custom_saved.level_13_level_number, NULL);
		process_word("victory_stops_time_level", &custom_saved.victory_stops_time_level, &never_is_16_list);
		process_word("win_level", &custom_saved.win_level, &never_is_16_list);
		process_byte("win_room", &custom_saved.win_room, NULL);
		process_byte("loose_floor_delay", &custom_saved.loose_floor_delay, NULL);
		process_byte("base_speed", &custom_saved.base_speed, NULL);
		process_byte("fight_speed", &custom_saved.fight_speed, NULL);
		process_byte("chomper_speed", &custom_saved.chomper_speed, NULL);
	} // end of section [CustomGameplay]

	// [Level 1], etc.
	int ini_level = -1;
	if (strncasecmp(section, "Level ", 6) == 0 && sscanf(section+6, "%d", &ini_level) == 1) {
		if (ini_level >= 0 && ini_level <= 15) {
			// TODO: And maybe allow new types in addition to the existing ones.
			process_byte("level_type", &custom_saved.tbl_level_type[ini_level], &level_type_names_list);
			process_word("level_color", &custom_saved.tbl_level_color[ini_level], NULL);
			process_short("guard_type", &custom_saved.tbl_guard_type[ini_level], &guard_type_names_list);
			process_byte("guard_hp", &custom_saved.tbl_guard_hp[ini_level], NULL);

			byte cutscene_index = 0xFF;
			if (ini_process_byte(name, value, "cutscene", &cutscene_index, NULL) == 1) {
				if (cutscene_index < COUNT(custom_saved.tbl_cutscenes_by_index)) {
					custom_saved.tbl_cutscenes_by_index[ini_level] = cutscene_index;
				}
				return 1;
			}

			process_byte("entry_pose", &custom_saved.tbl_entry_pose[ini_level], &entry_pose_names_list);
			process_sbyte("seamless_exit", &custom_saved.tbl_seamless_exit[ini_level], NULL);
		} else {
			printf("Warning: Invalid section [Level %d] in the INI!\n", ini_level);
		}
	}

	// [Skill 0], etc.
	int ini_skill = -1;
	if (strncasecmp(section, "Skill ", 6) == 0 && sscanf(section+6, "%d", &ini_skill) == 1) {
		if (ini_skill >= 0 && ini_skill < NUM_GUARD_SKILLS) {
			process_word("strikeprob",    &custom_saved.strikeprob   [ini_skill], NULL);
			process_word("restrikeprob",  &custom_saved.restrikeprob [ini_skill], NULL);
			process_word("blockprob",     &custom_saved.blockprob    [ini_skill], NULL);
			process_word("impblockprob",  &custom_saved.impblockprob [ini_skill], NULL);
			process_word("advprob",       &custom_saved.advprob      [ini_skill], NULL);
			process_word("refractimer",   &custom_saved.refractimer  [ini_skill], NULL);
			process_word("extrastrength", &custom_saved.extrastrength[ini_skill], NULL);
		} else {
			printf("Warning: Invalid section [Skill %d] in the INI!\n", ini_skill);
		}
	}

	return 0;
}

// Callback for a mod-specific INI configuration (that may overrule SDLPoP.ini for SOME but not all options):
static int mod_ini_callback(const char *section, const char *name, const char *value) {
	if (check_ini_section("Enhancements") || check_ini_section("CustomGameplay") ||
		strncasecmp(section, "Level ", 6) == 0 ||
		strcasecmp(name, "enable_copyprot") == 0 ||
		strcasecmp(name, "enable_quicksave") == 0 ||
		strcasecmp(name, "enable_quicksave_penalty") == 0
	) {
		global_ini_callback(section, name, value);
	}
	return 0;
}

void set_options_to_default() {
#ifdef USE_MENU
	enable_pause_menu = 1;
#endif
	enable_copyprot = 0;
	enable_music = 1;
	enable_fade = 1;
	enable_flash = 1;
	enable_text = 1;
	enable_info_screen = 1;
	start_fullscreen = 0;
	use_correct_aspect_ratio = 0;
	use_integer_scaling = 0;
	scaling_type = 0;
	enable_controller_rumble = 1;
	joystick_only_horizontal = 1;
	joystick_threshold = 8000;
	enable_quicksave = 1;
	enable_quicksave_penalty = 1;
	enable_replay = 1;
#ifdef USE_LIGHTING
	enable_lighting = 0;
#endif
	// By default, all the fixes are used, unless otherwise specified.
	// So, if one of these options is omitted from the INI file, they default to true.
	memset(&fixes_saved, 1, sizeof(fixes_saved));
	custom_saved = custom_defaults;
	turn_fixes_and_enhancements_on_off(0);
	turn_custom_options_on_off(0);
}

void load_global_options() {
	set_options_to_default();
	ini_load(locate_file("SDLPoP.ini"), global_ini_callback); // global configuration
	load_dos_exe_modifications("."); // read PRINCE.EXE in the current working directory
}

void check_mod_param() {
	// The 'mod' command line argument can override the levelset choice in SDLPoP.ini
	// usage: prince mod "Mod Name"
	// TODO: maybe allow absolute paths, instead of only paths relative to the mods folder?
	const char* mod_param = check_param("mod");
	if (mod_param != NULL) {
		use_custom_levelset = true;
		memset(levelset_name, 0, sizeof(levelset_name));
		snprintf_check(levelset_name, sizeof(levelset_name), "%s", mod_param);
	}
}

enum dos_version {
	dos_10_packed = 0,
	dos_10_unpacked = 1,
	dos_13_packed = 2,
	dos_13_unpacked = 3,
	dos_14_packed = 4,
	dos_14_unpacked = 5,
};

bool read_exe_bytes(void* dest, size_t nbytes, byte* exe_memory, int exe_offset, int exe_size) {
	if (exe_offset < 0) return false; // Happens if a CusPop modification is not available for the mod's EXE version.
	if (exe_offset < exe_size) {
		memcpy(dest, exe_memory + exe_offset, nbytes);
	}
	return true;
}

int identify_dos_exe_version(int filesize) {
	int dos_version = -1;
	switch (filesize) {
		case 123335: dos_version = dos_10_packed; break;
		case 125115: dos_version = dos_13_packed; break;
		case 110855: dos_version = dos_14_packed; break;
		case 129504: dos_version = dos_10_unpacked; break;
		case 129472: dos_version = dos_13_unpacked; break;
		case 115008: dos_version = dos_14_unpacked; break;
		default: break;
	}
	return dos_version;
}

void load_dos_exe_modifications(const char* folder_name) {
	char filename[POP_MAX_PATH];
	snprintf(filename, sizeof(filename), "%s/%s", folder_name, "PRINCE.EXE");
	FILE* fp = fopen(filename, "rb");

	int dos_version = -1;
	struct stat info;
	if (fp != NULL && fstat(fileno(fp), &info) == 0 && info.st_size > 0) {
		dos_version = identify_dos_exe_version(info.st_size);
	} else {
		// PRINCE.EXE not found, try to search for other .EXE files in the same folder.
		directory_listing_type* directory_listing = create_directory_listing_and_find_first_file(folder_name, "exe");
		if (directory_listing != NULL) {
			do {
				char* current_filename = get_current_filename_from_directory_listing(directory_listing);
				snprintf(filename, sizeof(filename), "%s/%s", folder_name, current_filename);
				fp = fopen(filename, "rb");
				if (fp != NULL && fstat(fileno(fp), &info) == 0 && info.st_size > 0) {
					dos_version = identify_dos_exe_version(info.st_size);
					if (dos_version >= 0) {
						break; // We found a DOS executable with the right size!
					}
					fclose(fp);
					fp = NULL;
				}
				// Keep looking until we find an .EXE with the right size, or until there are no .EXE files left.
			} while (find_next_file(directory_listing));
			close_directory_listing(directory_listing);
		}
	}

	if (dos_version >= 0) {
		turn_custom_options_on_off(1);
		byte* exe_memory = malloc((size_t) info.st_size);
		if (fread(exe_memory, (size_t) info.st_size, 1, fp) != 1) {
			fprintf(stderr, "Could not read %s!?\n", filename);
			fclose(fp);
			return;
		}

		byte temp_bytes[64] = {0};
		word temp_word = 0;
		bool read_ok;

#define process(x, nbytes, ...) \
		do { \
			static const int offsets[6] = __VA_ARGS__; \
			int offset = offsets[dos_version]; \
			read_ok = read_exe_bytes(x, nbytes, exe_memory, offset, info.st_size); \
		} while(0)

		// Offsets and comparisons are derived from princehack.xml
		process(&custom_saved.start_minutes_left, 2, {0x04a23, 0x060d3, 0x04ea3, 0x055e3, 0x0495f, 0x05a8f});
		process(&custom_saved.start_ticks_left, 2, {0x04a29, 0x060d9, 0x04ea9, 0x055e9, 0x04965, 0x05a95});
		process(&custom_saved.start_hitp, 2, {0x04a2f, 0x060df, 0x04eaf, 0x055ef, 0x0496b, 0x05a9b});
		process(&custom_saved.first_level, 2, {0x00707, 0x01db7, 0x007db, 0x00f1b, 0x0079f, 0x018cf});
		process(&custom_saved.max_hitp_allowed, 2, {0x013f1, 0x02aa1, 0x015ac, 0x01cec, 0x014a3, 0x025d3});
		process(&custom_saved.saving_allowed_first_level, 1, {0x007c8, 0x01e78, 0x008b4, 0x00ff4, 0x00878, 0x019a8});
		if (read_ok) custom_saved.saving_allowed_first_level += 1;
		process(&custom_saved.saving_allowed_last_level, 1, {0x007cf, 0x01e7f, 0x008bb, 0x00ffb, 0x0087f, 0x019af});
		if (read_ok) custom_saved.saving_allowed_last_level -= 1;
		if (dos_version == dos_10_packed || dos_version == dos_10_unpacked) {
			static const byte comparison[] = {0xa3, 0x92, 0x4e, 0xa3, 0x5c, 0x40, 0xa3, 0x8e, 0x4e, 0xa2, 0x2a,
			                                  0x3d, 0xa2, 0x29, 0x3d, 0xa3, 0xee, 0x42, 0xa2, 0x2e, 0x3d, 0x98};
			process(temp_bytes, COUNT(comparison), {0x04c9b, 0x0634b, -1, -1, -1, -1});
			custom_saved.start_upside_down = (memcmp(temp_bytes, comparison, COUNT(comparison)) != 0);
		}
		process(&custom_saved.start_in_blind_mode, 1, {0x04e46, 0x064f6, 0x052ce, 0x05a0e, 0x04d8a, 0x05eba});
		process(&custom_saved.copyprot_level, 2, {0x1aaeb, 0x1c62e, 0x1b89b, 0x1c49e, 0x17c3d, 0x18e18});
		process(&custom_saved.drawn_tile_top_level_edge, 1, {0x0a1f0, 0x0b8a0, 0x0a69c, 0x0addc, 0x0a158, 0x0b288});
		process(&custom_saved.drawn_tile_left_level_edge, 1, {0x0a26b, 0x0b91b, -1, -1, -1, -1});
		process(&custom_saved.level_edge_hit_tile, 1, {0x06f02, 0x085b2, -1, -1, -1, -1});
		process(temp_bytes, 2, {0x9111, 0xA7C1, 0x95BE, 0x9CFE, 0x907A, 0xA1AA}); // allow triggering any tile
		if (read_ok) custom_saved.allow_triggering_any_tile = (temp_bytes[0] == 0x75 && temp_bytes[1] == 0x13);
		process(temp_bytes, 1, {0x0a7bb, 0x0be6b, 0x0ac67, 0x0b3a7, 0x0a723, 0x0b853}); // enable WDA in palace
		if (read_ok) custom_saved.enable_wda_in_palace = (temp_bytes[0] != 116);
		process(&custom_saved.tbl_level_type, 16, {0x1acea, 0x1c842, 0x1b9ae, 0x1c5c6, 0x17d4c, 0x18f3c});
		process(&custom_saved.tbl_guard_hp, 16, {0x1b8a8, 0x1d46a, 0x1c6c5, 0x1d35c, 0x18a97, 0x19d06});
		process(&custom_saved.tbl_guard_type, sizeof(short)*16, {-1, 0x1c964, -1, 0x1c702, -1, 0x1905e});
		process(&custom_saved.vga_palette, sizeof(rgb_type)*16,  {0x1d141, 0x1f136, 0x1df5e, 0x1f02a, 0x1a335, 0x1b9de});
		process(&temp_word, 2, {0x003e2, 0x01a92, 0x0046b, 0x00bab, 0x00455, 0x01585}); // titles skipping
		if (read_ok) custom_saved.skip_title = (temp_word != 63558);
		process(&custom_saved.shift_L_allowed_until_level, 1, {0x0085c, 0x01f0c, 0x00955, 0x01095, 0x00919, 0x01a49});
		if (read_ok) custom_saved.shift_L_allowed_until_level += 1;
		process(&custom_saved.shift_L_reduced_minutes, 2, {0x008ad, 0x01f5d, 0x00991, 0x010d1, 0x00955, 0x01a85});
		process(&custom_saved.shift_L_reduced_ticks, 2, {0x008b3, 0x01f63, 0x00997, 0x010d7, 0x0095b, 0x01a8b});
		// TODO: cutscenes
		// TODO: color variations
		process(&custom_saved.demo_hitp, 1, {0x04c28, 0x062d8, 0x050b0, 0x057f0, 0x04b6c, 0x05c9c});
		process(&custom_saved.demo_end_room, 1, {0x00b40, 0x021f0, 0x00c25, 0x01365, 0x00be9, 0x01d19});
		process(&custom_saved.intro_music_level, 1, {0x04c37, 0x062e7, 0x050bf, 0x057ff, 0x04b7b, 0x05cab});
		process(temp_bytes, 1, {0x04b29, 0x061d9, 0x04fa9, 0x056e9, 0x04a65, 0x05b95}); // where the kid will have the sword
		if (read_ok) custom_saved.have_sword_from_level = (temp_bytes[0] == 0xEB) ? 16 /*never*/ : 2;
		process(&custom_saved.checkpoint_level, 1, {0x04b9e, 0x0624e, 0x05026, 0x05766, 0x04ae2, 0x05c12});
		process(&custom_saved.checkpoint_respawn_dir, 1, {0x04bac, 0x0625c, 0x05034, 0x05774, 0x04af0, 0x05c20});
		process(&custom_saved.checkpoint_respawn_room, 1, {0x04bb1, 0x06261, 0x05039, 0x05779, 0x04af5, 0x05c25});
		process(&custom_saved.checkpoint_respawn_tilepos, 1, {0x04bb6, 0x06266, 0x0503e, 0x0577e, 0x04afa, 0x05c2a});
		process(&custom_saved.checkpoint_clear_tile_room, 1, {0x04bb8, 0x06268, 0x05040, 0x05780, 0x04afc, 0x05c2c});
		process(&custom_saved.checkpoint_clear_tile_col, 1, {0x04bbc, 0x0626c, 0x05044, 0x05784, 0x04b00, 0x05c30});
		process(&temp_word, 2, {0x04bbf, 0x0626f, 0x05047, 0x05787, 0x04b03, 0x05c33}); // row of the tile to clear
		if (read_ok) {
			if (temp_word == 49195) {
				custom_saved.checkpoint_clear_tile_row = 0;
			} else if (temp_word == 432) {
				custom_saved.checkpoint_clear_tile_row = 1;
			} else if (temp_word == 688) {
				custom_saved.checkpoint_clear_tile_row = 2;
			}
		}
		process(&custom_saved.skeleton_level, 1, {0x046a4, 0x05d54, -1, -1, -1, -1});
		process(&custom_saved.skeleton_room, 1, {0x046b8, 0x05d68, -1, -1, -1, -1});
		process(&custom_saved.skeleton_trigger_column_1, 1, {0x046cc, 0x05d7c, -1, -1, -1, -1});
		process(&custom_saved.skeleton_trigger_column_2, 1, {0x046d3, 0x05d83, -1, -1, -1, -1});
		process(&custom_saved.skeleton_column, 1, {0x046de, 0x05d8e, 0x04b5e, 0x0529e, 0x0461a, 0x0574a});
		process(&custom_saved.skeleton_row, 1, {0x046e2, 0x05d92, 0x04b62, 0x052a2, 0x0461e, 0x0574e});
		process(temp_bytes, 1, {0x046c3, 0x05d73, -1, -1, -1, -1});
		if (read_ok) custom_saved.skeleton_require_open_level_door = (temp_bytes[0] != 0xEB);
		process(&custom_saved.skeleton_skill, 1, {0x0478f, 0x05e3f, -1, -1, -1, -1});
		process(&custom_saved.skeleton_reappear_room, 1, {0x03b32, 0x051e2, 0x03fb2, 0x046f2, 0x03a6e, 0x04b9e});
		process(&custom_saved.skeleton_reappear_x, 1, {0x03b39, 0x051e9, -1, -1, -1, -1});
		process(&custom_saved.skeleton_reappear_row, 1, {0x03b3e, 0x051ee, -1, -1, -1, -1});
		process(&custom_saved.skeleton_reappear_dir, 1, {0x03b43, 0x051f3, -1, -1, -1, -1});
		process(&custom_saved.mirror_level, 1, {0x08dc7, 0x0a477, 0x09274, 0x099b4, 0x08d30, 0x09e60});
		process(&custom_saved.mirror_room, 1, {0x08dcb, 0x0a47b, 0x09278, 0x099b8, 0x08d34, 0x09e64});
		if (read_ok) custom_saved.mirror_column = custom_saved.mirror_room;
		process(&temp_word, 2, {0x08dcf, 0x0a47f, 0x0927c, 0x099bc, 0x08d38, 0x09e68}); // mirror row
		if (read_ok) {
			if (temp_word == 49195) {
				custom_saved.mirror_row = 0;
			} else if (temp_word == 432) {
				custom_saved.mirror_row = 1;
			} else if (temp_word == 688) {
				custom_saved.mirror_row = 2;
			}
		}
		process(&custom_saved.mirror_tile, 1, {0x08de3, 0x0a493, 0x09290, 0x099d0, 0x08d4c, 0x09e7c});
		process(temp_bytes, 1, {0x051a2, 0x06852, 0x05636, 0x05d76, 0x050f2, 0x06222});
		if (read_ok) custom_saved.show_mirror_image = (temp_bytes[0] != 0xEB);
		process(&custom_saved.falling_exit_level, 1, {0x03eb2, 0x05562, -1, -1, -1, -1});
		process(&custom_saved.falling_exit_room, 1, {0x03eb9, 0x05569, -1, -1, -1, -1});
		process(&custom_saved.falling_entry_level, 1, {0x04cbd, 0x0636d, -1, -1, -1, -1});
		process(&custom_saved.falling_entry_room, 1, {0x04cc4, 0x06374, -1, -1, -1, -1});
		process(&custom_saved.mouse_level, 1, {0x05166, 0x06816, 0x055fa, 0x05d3a, 0x050b6, 0x061e6});
		process(&custom_saved.mouse_room, 1, {0x0516d, 0x0681d, 0x05601, 0x05d41, 0x050bd, 0x061ed});
		process(&custom_saved.mouse_delay, 2, {0x0517f, 0x0682f, 0x05613, 0x05d53, 0x050cf, 0x061ff});
		process(&custom_saved.mouse_object, 1, {0x054b3, 0x06b63, 0x05947, 0x06087, 0x05403, 0x06533});
		process(&custom_saved.mouse_start_x, 1, {0x054b8, 0x06b68, 0x0594c, 0x0608c, 0x05408, 0x06538});
		{
			byte level = 0;
			byte room = 0;
			process(&level, 1, {0x00b84, 0x02234, 0x00c6d, 0x013ad, 0x00c31, 0x01d61}); // seamless exit
			if (read_ok) process(&room, 1, {0x00b8b, 0x0223b, 0x00c74, 0x013b4, 0x00c38, 0x01d68});
			if (read_ok && level < 16) {
				memset(custom_saved.tbl_seamless_exit, -1, sizeof(custom_saved.tbl_seamless_exit));
				custom_saved.tbl_seamless_exit[level] = room;
			}
		}
		process(&custom_saved.loose_tiles_level, 1, {0x0120d, 0x028bd, -1, -1, 0x01358, 0x02488});
		process(&custom_saved.loose_tiles_room_1, 1, {0x01214, 0x028c4, -1, -1, 0x0135f, 0x0248f});
		process(&custom_saved.loose_tiles_room_2, 1, {0x0121b, 0x028cb, -1, -1, 0x01366, 0x02496});
		process(&custom_saved.loose_tiles_first_tile, 1, {0x0122e, 0x028de, -1, -1, 0x01379, 0x024a9});
		process(&custom_saved.loose_tiles_last_tile, 1, {0x0124d, 0x028fd, -1, -1, 0x01398, 0x024c8});
		process(&custom_saved.jaffar_victory_level, 1, {0x084b3, 0x09b63, 0x08963, 0x090a3, 0x0841f, 0x0954f});
		process(&custom_saved.jaffar_victory_flash_time, 2, {0x084c0, 0x09b70, 0x08970, 0x090b0, 0x0842c, 0x0955c});
		process(&custom_saved.hide_level_number_from_level, 2, {0x0c3d9, 0x0da89, 0x0c8cd, 0x0d00d, 0x0c389, 0x0d4b9});
		process(&temp_bytes, 1, {0x0c3d9, 0x0da89, 0x0c8cd, 0x0d00d, 0x0c389, 0x0d4b9});
		if (read_ok) custom_saved.level_13_level_number = (temp_bytes[0] == 0xEB) ? 13 : 12;
		process(&custom_saved.victory_stops_time_level, 1, {0x0c2e0, 0x0d990, -1, -1, -1, -1});
		process(&custom_saved.win_level, 1, {0x011dc, 0x0288c, 0x01397, 0x01ad7, 0x01327, 0x02457});
		process(&custom_saved.win_room, 1, {0x011e3, 0x02893, 0x0139e, 0x01ade, 0x0132e, 0x0245e});
		process(&custom_saved.loose_floor_delay, 1, {0x9536, 0xABE6, -1, -1, -1, -1});

		// guard skills
		process(&custom_saved.strikeprob   , 2*NUM_GUARD_SKILLS, {-1, 0x1D3C2, -1, 0x1D2B4, -1, 0x19C5E});
		process(&custom_saved.restrikeprob , 2*NUM_GUARD_SKILLS, {-1, 0x1D3DA, -1, 0x1D2CC, -1, 0x19C76});
		process(&custom_saved.blockprob    , 2*NUM_GUARD_SKILLS, {-1, 0x1D3F2, -1, 0x1D2E4, -1, 0x19C8E});
		process(&custom_saved.impblockprob , 2*NUM_GUARD_SKILLS, {-1, 0x1D40A, -1, 0x1D2FC, -1, 0x19CA6});
		process(&custom_saved.advprob      , 2*NUM_GUARD_SKILLS, {-1, 0x1D422, -1, 0x1D314, -1, 0x19CBE});
		process(&custom_saved.refractimer  , 2*NUM_GUARD_SKILLS, {-1, 0x1D43A, -1, 0x1D32C, -1, 0x19CD6});
		process(&custom_saved.extrastrength, 2*NUM_GUARD_SKILLS, {-1, 0x1D452, -1, 0x1D344, -1, 0x19CEE});

		// shadow's starting positions
		process(&custom_saved.init_shad_6    , 8, {0x1B8B8, 0x1D47A, 0x1C6D5, 0x1D36C, 0x18AA7, 0x19D16});
		process(&custom_saved.init_shad_5    , 8, {0x1B8C0, 0x1D482, 0x1C6DD, 0x1D374, 0x18AAF, 0x19D1E});
		process(&custom_saved.init_shad_12   , 8, {     -1, 0x1D48A,      -1, 0x1D37C,      -1, 0x19D26}); // in the packed versions, the five zero bytes at the end are compressed
		// automatic moves
		process(&custom_saved.shad_drink_move,  8*4, {     -1, 0x1D492,      -1, 0x1D384,      -1, 0x19D2E}); // in the packed versions, the four zero bytes at the start are compressed
		process(&custom_saved.demo_moves     , 25*4, {0x1B8EE, 0x1D4B2, 0x1C70B, 0x1D3A4, 0x18ADD, 0x19D4E});

		// speeds
		process(&custom_saved.base_speed   , 1, { 0x4F01, 0x65B1, 0x5389, 0x5AC9, 0x4E45, 0x5F75 });
		process(&custom_saved.fight_speed  , 1, { 0x4EF9, 0x65A9, 0x5381, 0x5AC1, 0x4E3D, 0x5F6D });
		process(&custom_saved.chomper_speed, 1, { 0x8BBD, 0xA26D, 0x906D, 0x97AD, 0x8B29, 0x9C59 });

		// The order of offsets is: dos_10_packed, dos_10_unpacked, dos_13_packed, dos_13_unpacked, dos_14_packed, dos_14_unpacked

#undef process
		free(exe_memory);
	}

	if (fp != NULL) fclose(fp);
}


void load_mod_options() {
	// load mod-specific INI configuration
	if (use_custom_levelset) {
		// find the folder containing the mod's files
		char folder_name[POP_MAX_PATH];
		snprintf_check(folder_name, sizeof(folder_name), "%s/%s", mods_folder, levelset_name);
		const char* located_folder_name = locate_file(folder_name);
		//printf("located_folder_name = %s\n", located_folder_name);
		bool ok = false;
		struct stat info;
		if (stat(located_folder_name, &info) == 0) {
			if (S_ISDIR(info.st_mode)) {
				// It's a directory
				ok = true;
				snprintf_check(mod_data_path, sizeof(mod_data_path), "%s", located_folder_name);
				// Try to load PRINCE.EXE (DOS)
				load_dos_exe_modifications(located_folder_name);
				// Try to load mod.ini
				char mod_ini_filename[POP_MAX_PATH];
				snprintf_check(mod_ini_filename, sizeof(mod_ini_filename), "%s/%s", located_folder_name, "mod.ini");
				if (file_exists(mod_ini_filename)) {
					// Nearly all mods would want to use custom options, so always allow them.
					use_custom_options = 1;
					ini_load(mod_ini_filename, mod_ini_callback);
				}
			} else {
				printf("Could not load mod '%s' - not a directory\n", levelset_name);
			}
		} else {
			printf("Mod '%s' not found\n", levelset_name);
			char message[256];
			snprintf_check(message, sizeof(message), "Cannot find the mod '%s' in the mods folder.", levelset_name);
			show_dialog(message);
#ifdef USE_REPLAY
			if (replaying) show_dialog("If the replay file restarts the level or advances to the next level, a wrong level will be loaded.");
#endif
		}
		if (!ok) {
			use_custom_levelset = 0;
			levelset_name[0] = '\0';
		}
	}
	turn_fixes_and_enhancements_on_off(use_fixes_and_enhancements);
	turn_custom_options_on_off(use_custom_options);
}

int process_rw_write(SDL_RWops* rw, void* data, size_t data_size) {
	return SDL_RWwrite(rw, data, data_size, 1);
}

int process_rw_read(SDL_RWops* rw, void* data, size_t data_size) {
	return SDL_RWread(rw, data, data_size, 1);
	// if this returns 0, most likely the end of the stream has been reached
}


