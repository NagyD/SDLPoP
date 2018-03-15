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
		fscanf(f, " ;%*[^\n]");
		fscanf(f, " \n");
	}

	fclose(f);
	return 0;
}

NAMES_LIST(level_type_names, {"dungeon", "palace"});
NAMES_LIST(guard_type_names, {"guard", "fat", "skel", "vizier", "shadow"});
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
		if (strcasecmp(name, "mods_folder") == 0) {
			if (value[0] != '\0' && strcasecmp(value, "default") != 0) {
				strcpy(mods_folder, locate_file(value));
			}
			return 1;
		}
#endif
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
		process_word("hide_level_number_first_level", &custom_saved.hide_level_number_first_level, &never_is_16_list);
		process_byte("level_13_level_number", &custom_saved.level_13_level_number, NULL);
		process_word("victory_stops_time_level", &custom_saved.victory_stops_time_level, &never_is_16_list);
		process_word("win_level", &custom_saved.win_level, &never_is_16_list);
		process_byte("win_room", &custom_saved.win_room, NULL);
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
			// TODO: warning?
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
	enable_lighting = 0;
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
}

void check_mod_param() {
	// The 'mod' command line argument can override the levelset choice in SDLPoP.ini
	// usage: prince mod "Mod Name"
	// TODO: maybe allow absolute paths, instead of only paths relative to the mods folder?
	const char* mod_param = check_param("mod");
	if (mod_param != NULL) {
		use_custom_levelset = true;
		memset(levelset_name, 0, sizeof(levelset_name));
		strncpy(levelset_name, mod_param, sizeof(levelset_name));
	}
}

void load_mod_options() {
	// load mod-specific INI configuration
	if (use_custom_levelset) {
		// find the folder containing the mod's files
		char folder_name[POP_MAX_PATH];
		snprintf(folder_name, sizeof(folder_name), "%s/%s", mods_folder, levelset_name);
		const char* located_folder_name = locate_file(folder_name);
		bool ok = false;
		struct stat info;
		if (stat(located_folder_name, &info) == 0) {
			if (S_ISDIR(info.st_mode)) {
				// It's a directory
				ok = true;
				strncpy(mod_data_path, located_folder_name, sizeof(mod_data_path));
				char mod_ini_filename[POP_MAX_PATH];
				snprintf(mod_ini_filename, sizeof(mod_ini_filename), "%s/%s", located_folder_name, "mod.ini");
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
		}
		if (!ok) {
			use_custom_levelset = 0;
			levelset_name[0] = '\0';
		}
	}
	turn_fixes_and_enhancements_on_off(use_fixes_and_enhancements);
	turn_custom_options_on_off(use_custom_options);
}



