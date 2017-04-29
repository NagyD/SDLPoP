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
#include <ctype.h>
#include <inttypes.h>


void disable_fixes_and_enhancements() {
    enable_crouch_after_climbing = 0;
    enable_freeze_time_during_end_music = 0;
    enable_remember_guard_hp = 0;
    fix_gate_sounds = 0;
    fix_two_coll_bug = 0;
    fix_infinite_down_bug = 0;
    fix_gate_drawing_bug = 0;
    fix_bigpillar_climb = 0;
    fix_jump_distance_at_edge = 0;
    fix_edge_distance_check_when_climbing = 0;
    fix_painless_fall_on_guard = 0;
    fix_wall_bump_triggers_tile_below = 0;
    fix_stand_on_thin_air = 0;
    fix_press_through_closed_gates = 0;
    fix_grab_falling_speed = 0;
    fix_skeleton_chomper_blood = 0;
    fix_move_after_drink = 0;
    fix_loose_left_of_potion = 0;
    fix_guard_following_through_closed_gates = 0;
    fix_safe_landing_on_spikes = 0;
    fix_glide_through_wall = 0;
    fix_drop_through_tapestry = 0;
    fix_land_against_gate_or_tapestry = 0;
    fix_unintended_sword_strike = 0;
    fix_retreat_without_leaving_room = 0;
    fix_running_jump_through_tapestry= 0;
    fix_push_guard_into_wall = 0;
    fix_jump_through_wall_above_gate = 0;
    fix_chompers_not_starting = 0;
	fix_feather_interrupted_by_leveldoor = 0;
	fix_offscreen_guards_disappearing = 0;
	fix_move_after_sheathe = 0;
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

#define MAX_NAME_LENGTH 20
typedef struct ini_value_list_type {
    const char (* names)[][MAX_NAME_LENGTH];
    word num_names;
} ini_value_list_type;

const char level_type_names[][MAX_NAME_LENGTH] = {"dungeon", "palace"};
const char guard_type_names[][MAX_NAME_LENGTH] = {"guard", "fat", "skel", "vizier", "shadow"};
const char tile_type_names[][MAX_NAME_LENGTH] = {
				"empty", "floor", "spike", "pillar", "gate",                                        // 0..4
				"stuck", "closer", "doortop_with_floor", "bigpillar_bottom", "bigpillar_top",       // 5..9
				"potion", "loose", "doortop", "mirror", "debris",                                   // 10..14
				"opener", "level_door_left", "level_door_right", "chomper", "torch",                // 15..19
				"wall", "skeleton", "sword", "balcony_left", "balcony_right",                       // 20..24
				"lattice_pillar", "lattice_down", "lattice_small", "lattice_left", "lattice_right", // 25..29
				"torch_with_debris", // 30
};

ini_value_list_type level_type_names_list = {&level_type_names, COUNT(level_type_names)};
ini_value_list_type guard_type_names_list = {&guard_type_names, COUNT(guard_type_names)};
ini_value_list_type tile_type_names_list = {&tile_type_names, COUNT(tile_type_names)};

#define INI_NO_VALID_NAME -9999

static inline int ini_get_named_value(const char* value, ini_value_list_type* value_names) {
    if (value_names != NULL) {
        int i;
        char *base_ptr = (char *) value_names->names;
        for (i = 0; i < value_names->num_names; ++i) {
            char *name = (base_ptr + i * MAX_NAME_LENGTH);
            if (strcasecmp(value, name) == 0) return i;
        }
    }
    return INI_NO_VALID_NAME; // failure
}

static inline int ini_process_boolean(const char* curr_name, const char* value, const char* option_name, byte* target) {
    if(strcasecmp(curr_name, option_name) == 0) {
        if (strcasecmp(value, "true") == 0) *target = 1;
        else if (strcasecmp(value, "false") == 0) *target = 0;
        return 1; // finished; don't look for more possible options that curr_name can be
    }
    return 0; // not the right option; should check another option_name
}

static inline int ini_process_word(const char* curr_name, const char* value, const char* option_name, word* target, ini_value_list_type* value_names) {
    if(strcasecmp(curr_name, option_name) == 0) {
        if (strcasecmp(value, "default") != 0) {
            int named_value = ini_get_named_value(value, value_names);
            *target = (named_value == INI_NO_VALID_NAME) ? ((word) strtoumax(value, NULL, 0)) : ((word) named_value);
        }
        return 1; // finished; don't look for more possible options that curr_name can be
    }
    return 0; // not the right option; should check another option_name
}

static inline int ini_process_short(const char* curr_name, const char* value, const char* option_name, short* target, ini_value_list_type* value_names) {
    if(strcasecmp(curr_name, option_name) == 0) {
        if (strcasecmp(value, "default") != 0) {
            int named_value = ini_get_named_value(value, value_names);
            *target = (named_value == INI_NO_VALID_NAME) ? ((short) strtoimax(value, NULL, 0)) : ((short) named_value);
        }
        return 1; // finished; don't look for more possible options that curr_name can be
    }
    return 0; // not the right option; should check another option_name
}

static inline int ini_process_byte(const char* curr_name, const char* value, const char* option_name, byte* target, ini_value_list_type* value_names) {
    if(strcasecmp(curr_name, option_name) == 0) {
        if (strcasecmp(value, "default") != 0) {
            int named_value = ini_get_named_value(value, value_names);
            *target = (named_value == INI_NO_VALID_NAME) ? ((byte) strtoumax(value, NULL, 0)) : ((byte) named_value);
        }
        return 1; // finished; don't look for more possible options that curr_name can be
    }
    return 0; // not the right option; should check another option_name
}

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

    #define process_boolean(option_name, target)                        \
    if (ini_process_boolean(name, value, option_name, target)) return 1;

    if (check_ini_section("General")) {
        process_boolean("enable_copyprot", &enable_copyprot);
        process_boolean("enable_mixer", &enable_mixer);
        process_boolean("enable_fade", &enable_fade);
        process_boolean("enable_flash", &enable_flash);
        process_boolean("enable_text", &enable_text);
		process_boolean("enable_info_screen", &enable_info_screen);
		process_boolean("start_fullscreen", &start_fullscreen);
		process_word("pop_window_width", &pop_window_width, NULL);
		process_word("pop_window_height", &pop_window_height, NULL);
		process_boolean("use_correct_aspect_ratio", &use_correct_aspect_ratio);
        process_boolean("enable_controller_rumble", &enable_controller_rumble);
        process_boolean("joystick_only_horizontal", &joystick_only_horizontal);

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
                strcpy(replays_folder, value);
            }
            return 1;
        }
#endif
    }

    if (check_ini_section("Enhancements")) {
        if (strcasecmp(name, "use_fixes_and_enhancements") == 0) {
            if (strcasecmp(value, "true") == 0) use_fixes_and_enhancements = 1;
            else if (strcasecmp(value, "false") == 0) use_fixes_and_enhancements = 0;
            else if (strcasecmp(value, "prompt") == 0) use_fixes_and_enhancements = 2;
            return 1;
        }
        process_boolean("enable_crouch_after_climbing", &enable_crouch_after_climbing);
        process_boolean("enable_freeze_time_during_end_music", &enable_freeze_time_during_end_music);
        process_boolean("enable_remember_guard_hp", &enable_remember_guard_hp);
        process_boolean("fix_gate_sounds", &fix_gate_sounds);
        process_boolean("fix_two_coll_bug", &fix_two_coll_bug);
        process_boolean("fix_infinite_down_bug", &fix_infinite_down_bug);
        process_boolean("fix_gate_drawing_bug", &fix_gate_drawing_bug);
        process_boolean("fix_bigpillar_climb", &fix_bigpillar_climb);
        process_boolean("fix_jump_distance_at_edge", &fix_jump_distance_at_edge);
        process_boolean("fix_edge_distance_check_when_climbing", &fix_edge_distance_check_when_climbing);
        process_boolean("fix_painless_fall_on_guard", &fix_painless_fall_on_guard);
        process_boolean("fix_wall_bump_triggers_tile_below", &fix_wall_bump_triggers_tile_below);
        process_boolean("fix_stand_on_thin_air", &fix_stand_on_thin_air);
        process_boolean("fix_press_through_closed_gates", &fix_press_through_closed_gates);
        process_boolean("fix_grab_falling_speed", &fix_grab_falling_speed);
        process_boolean("fix_skeleton_chomper_blood", &fix_skeleton_chomper_blood);
        process_boolean("fix_move_after_drink", &fix_move_after_drink);
        process_boolean("fix_loose_left_of_potion", &fix_loose_left_of_potion);
        process_boolean("fix_guard_following_through_closed_gates", &fix_guard_following_through_closed_gates);
        process_boolean("fix_safe_landing_on_spikes", &fix_safe_landing_on_spikes);
        process_boolean("fix_glide_through_wall", &fix_glide_through_wall);
        process_boolean("fix_drop_through_tapestry", &fix_drop_through_tapestry);
        process_boolean("fix_land_against_gate_or_tapestry", &fix_land_against_gate_or_tapestry);
        process_boolean("fix_unintended_sword_strike", &fix_unintended_sword_strike);
        process_boolean("fix_retreat_without_leaving_room", &fix_retreat_without_leaving_room);
        process_boolean("fix_running_jump_through_tapestry", &fix_running_jump_through_tapestry);
        process_boolean("fix_push_guard_into_wall", &fix_push_guard_into_wall);
        process_boolean("fix_jump_through_wall_above_gate", &fix_jump_through_wall_above_gate);
        process_boolean("fix_chompers_not_starting", &fix_chompers_not_starting);
        process_boolean("fix_feather_interrupted_by_leveldoor", &fix_feather_interrupted_by_leveldoor);
        process_boolean("fix_offscreen_guards_disappearing", &fix_offscreen_guards_disappearing);
        process_boolean("fix_move_after_sheathe", &fix_move_after_sheathe);
    }

    if (check_ini_section("CustomGameplay")) {
        process_word("start_minutes_left", &start_minutes_left, NULL);
        process_word("start_ticks_left", &start_ticks_left, NULL);
        process_word("start_hitp", &start_hitp, NULL);
        process_word("max_hitp_allowed", &max_hitp_allowed, NULL);
        process_word("saving_allowed_first_level", &saving_allowed_first_level, NULL);
        process_word("saving_allowed_last_level", &saving_allowed_last_level, NULL);
        process_boolean("start_upside_down", &start_upside_down);
        process_boolean("start_in_blind_mode", &start_in_blind_mode);
        process_word("copyprot_level", &copyprot_level, NULL);
        process_byte("drawn_tile_top_level_edge", &drawn_tile_top_level_edge, &tile_type_names_list);
        process_byte("drawn_tile_left_level_edge", &drawn_tile_left_level_edge, &tile_type_names_list);
        process_byte("level_edge_hit_tile", &level_edge_hit_tile, &tile_type_names_list);
        process_boolean("allow_triggering_any_tile", &allow_triggering_any_tile);
        // TODO: Maybe allow automatically choosing the correct WDA, depending on the loaded VDUNGEON.DAT?
		process_boolean("enable_wda_in_palace", &enable_wda_in_palace);

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
			rgb_type* palette_color = &vga_palette[ini_palette_color];
			palette_color->r = rgb[0] / 4; // the palette uses values 0..63, not 0..255
			palette_color->g = rgb[1] / 4;
			palette_color->b = rgb[2] / 4;
			return 1;
		}
		process_word("first_level", &first_level, NULL);
		process_boolean("skip_title", &skip_title);
		process_word("shift_L_allowed_until_level", &shift_L_allowed_until_level, NULL);
		process_word("shift_L_reduced_minutes", &shift_L_reduced_minutes, NULL);
		process_word("shift_L_reduced_ticks", &shift_L_reduced_ticks, NULL);
	} // end of section [CustomGameplay]

    // [Level 1], etc.
    int ini_level = -1;
    if (strncasecmp(section, "Level ", 6) == 0 && sscanf(section+6, "%d", &ini_level) == 1) {
        if (ini_level >= 0 && ini_level <= 15) {
            // TODO: And maybe allow new types in addition to the existing ones.
            process_byte("level_type", &tbl_level_type[ini_level], &level_type_names_list);
            process_word("level_color", &tbl_level_color[ini_level], NULL);
            process_short("guard_type", &tbl_guard_type[ini_level], &guard_type_names_list);
            process_byte("guard_hp", &tbl_guard_hp[ini_level], NULL);

			byte cutscene_index = 0xFF;
			if (ini_process_byte(name, value, "cutscene", &cutscene_index, NULL) == 1) {
				if (cutscene_index < COUNT(tbl_cutscenes_lookup)) {
					tbl_cutscenes_by_index[ini_level] = cutscene_index;
					tbl_cutscenes[ini_level] = tbl_cutscenes_lookup[cutscene_index];
				}
				return 1;
			}
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

void load_global_options() {
    ini_load("SDLPoP.ini", global_ini_callback); // global configuration
}

void check_mod_param() {
	// The 'mod' command line argument can override the levelset choice in SDLPoP.ini
	// usage: prince mod "Mod Name"
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
        char filename[POP_MAX_PATH];
        snprintf(filename, sizeof(filename), "mods/%s/%s", levelset_name, "mod.ini");
        ini_load(filename, mod_ini_callback);
    }

	if (!use_fixes_and_enhancements) disable_fixes_and_enhancements();
}

void show_use_fixes_and_enhancements_prompt() {
    if (use_fixes_and_enhancements != 2) return;
    draw_rect(&screen_rect, 0);
    show_text(&screen_rect, 0, 0,
		"\n"
		"Enable bug fixes and\n"
		"gameplay enhancements?\n"
		"\n"
		"NOTE:\n"
		"This option disables some game quirks.\n"
		"Certain tricks will no longer work by default.\n"
		"\n"
		"\n"
		"Y:  enhanced behavior \n"
		"N:  original behavior    \n"
		"\n"
		"Y / N ?\n"
		"\n"
		"\n"
		"\n"
		"You can fine-tune your preferences\n"
		"and/or bypass this screen by editing the file\n"
		"'SDLPoP.ini'"
	);
    while (use_fixes_and_enhancements == 2 ) {
		idle();
        switch (key_test_quit()) {
            case SDL_SCANCODE_Y:
                use_fixes_and_enhancements = 1;
                printf("Enabling game fixes and enhancements.\n");
                break;
            case SDL_SCANCODE_N:
                use_fixes_and_enhancements = 0;
                printf("Disabling game fixes and enhancements.\n");
                break;
		}
    }
    if (!use_fixes_and_enhancements) disable_fixes_and_enhancements();
}


