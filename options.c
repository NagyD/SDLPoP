/*
SDLPoP, a port/conversion of the DOS game Prince of Persia.
Copyright (C) 2013-2015  D�vid Nagy

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

void use_default_options() {
    options.use_fixes_and_enhancements = 0;
    options.enable_copyprot = 0;
    options.enable_mixer = 1;
    options.enable_fade = 1;
    options.enable_flash = 1;
    options.enable_text = 1;
    options.enable_quicksave = 1;
    options.enable_quicksave_penalty = 1;
    options.enable_replay = 1;
    options.enable_crouch_after_climbing = 1;
    options.enable_freeze_time_during_end_music = 1;
    options.fix_gate_sounds = 1;
    options.fix_two_coll_bug = 1;
    options.fix_infinite_down_bug = 1;
    options.fix_gate_drawing_bug = 0;
    options.fix_bigpillar_climb = 0;
    options.fix_jump_distance_at_edge = 1;
    options.fix_edge_distance_check_when_climbing = 1;
    options.fix_painless_fall_on_guard = 1;
    options.fix_wall_bump_triggers_tile_below = 1;
    options.fix_stand_on_thin_air = 1;
    options.fix_press_through_closed_gates = 1;
    options.fix_grab_falling_speed = 1;
    options.fix_skeleton_chomper_blood = 1;
    options.fix_move_after_drink = 1;
    options.fix_loose_left_of_potion = 1;
    options.fix_guard_following_through_closed_gates = 1;
    options.fix_safe_landing_on_spikes = 1;
}

void disable_fixes_and_enhancements() {
    options.enable_crouch_after_climbing = 0;
    options.enable_freeze_time_during_end_music = 0;
    options.fix_gate_sounds = 0;
    options.fix_two_coll_bug = 0;
    options.fix_infinite_down_bug = 0;
    options.fix_gate_drawing_bug = 0;
    options.fix_bigpillar_climb = 0;
    options.fix_jump_distance_at_edge = 0;
    options.fix_edge_distance_check_when_climbing = 0;
    options.fix_painless_fall_on_guard = 0;
    options.fix_wall_bump_triggers_tile_below = 0;
    options.fix_stand_on_thin_air = 0;
    options.fix_press_through_closed_gates = 0;
    options.fix_grab_falling_speed = 0;
    options.fix_skeleton_chomper_blood = 0;
    options.fix_move_after_drink = 0;
    options.fix_loose_left_of_potion = 0;
    options.fix_guard_following_through_closed_gates = 0;
    options.fix_safe_landing_on_spikes = 0;
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
        perror(filename);
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

static inline int ini_process_boolean(const char* curr_name, const char* value, const char* option_name, byte* target) {
    if(strcasecmp(curr_name, option_name) == 0) {
        if (strcasecmp(value, "true") == 0) *target = 1;
        else if (strcasecmp(value, "false") == 0) *target = 0;
        return 1; // finished; don't look for more possible options that curr_name can be
    }
    return 0; // not the right option; should check another option_name
}

static inline int ini_process_word(const char* curr_name, const char* value, const char* option_name, word* target) {
    if(strcasecmp(curr_name, option_name) == 0) {
        if (strcasecmp(value, "default") != 0) {
            word new_value = (word) strtoumax(value, NULL, 0);
            if (new_value != 0) *target = new_value;
        }
        return 1; // finished; don't look for more possible options that curr_name can be
    }
    return 0; // not the right option; should check another option_name
}

static int ini_callback(const char *section, const char *name, const char *value)
{
    //fprintf(stdout, "[%s] '%s'='%s'\n", section, name, value);

    #define check_ini_section(section_name)    (strcasecmp(section, section_name) == 0)

    // Make sure that we return successfully as soon as name matches the correct option_name
    #define process_word(option_name, target)                           \
    if (ini_process_word(name, value, option_name, target)) return 1;

    #define process_boolean(option_name, target)                        \
    if (ini_process_boolean(name, value, option_name, target)) return 1;

    if (check_ini_section("General")) {
        process_boolean("enable_copyprot", &options.enable_copyprot);
        process_boolean("enable_mixer", &options.enable_mixer);
        process_boolean("enable_fade", &options.enable_fade);
        process_boolean("enable_flash", &options.enable_flash);
        process_boolean("enable_text", &options.enable_text);
        process_boolean("start_fullscreen", &start_fullscreen);
        process_word("pop_window_width", &pop_window_width);
        process_word("pop_window_height", &pop_window_height);
    }

    if (check_ini_section("AdditionalFeatures")) {
        process_boolean("enable_quicksave", &options.enable_quicksave);
        process_boolean("enable_quicksave_penalty", &options.enable_quicksave_penalty);
        process_boolean("enable_replay", &options.enable_replay);
    }

    if (check_ini_section("Enhancements")) {
        if (strcasecmp(name, "use_fixes_and_enhancements") == 0) {
            if (strcasecmp(value, "true") == 0) options.use_fixes_and_enhancements = 1;
            else if (strcasecmp(value, "false") == 0) options.use_fixes_and_enhancements = 0;
            else if (strcasecmp(value, "prompt") == 0) options.use_fixes_and_enhancements = 2;
            return 1;
        }
        process_boolean("enable_crouch_after_climbing", &options.enable_crouch_after_climbing);
        process_boolean("enable_freeze_time_during_end_music", &options.enable_freeze_time_during_end_music);
        process_boolean("fix_gate_sounds", &options.fix_gate_sounds);
        process_boolean("fix_two_coll_bug", &options.fix_two_coll_bug);
        process_boolean("fix_infinite_down_bug", &options.fix_infinite_down_bug);
        process_boolean("fix_gate_drawing_bug", &options.fix_gate_drawing_bug);
        process_boolean("fix_bigpillar_climb", &options.fix_bigpillar_climb);
        process_boolean("fix_jump_distance_at_edge", &options.fix_jump_distance_at_edge);
        process_boolean("fix_edge_distance_check_when_climbing", &options.fix_edge_distance_check_when_climbing);
        process_boolean("fix_painless_fall_on_guard", &options.fix_painless_fall_on_guard);
        process_boolean("fix_wall_bump_triggers_tile_below", &options.fix_wall_bump_triggers_tile_below);
        process_boolean("fix_stand_on_thin_air", &options.fix_stand_on_thin_air);
        process_boolean("fix_press_through_closed_gates", &options.fix_press_through_closed_gates);
        process_boolean("fix_grab_falling_speed", &options.fix_grab_falling_speed);
        process_boolean("fix_skeleton_chomper_blood", &options.fix_skeleton_chomper_blood);
        process_boolean("fix_move_after_drink", &options.fix_move_after_drink);
        process_boolean("fix_loose_left_of_potion", &options.fix_loose_left_of_potion);
        process_boolean("fix_guard_following_through_closed_gates", &options.fix_guard_following_through_closed_gates);
        process_boolean("fix_safe_landing_on_spikes", &options.fix_safe_landing_on_spikes);
        process_boolean("fix_wall_bump_triggers_tile_below", &options.fix_wall_bump_triggers_tile_below);
        process_boolean("fix_wall_bump_triggers_tile_below", &options.fix_wall_bump_triggers_tile_below);
        process_boolean("fix_wall_bump_triggers_tile_below", &options.fix_wall_bump_triggers_tile_below);
    }

    if (check_ini_section("CustomGameplay")) {
        process_word("start_minutes_left", &start_minutes_left);
        process_word("start_ticks_left", &start_ticks_left);
        process_word("start_hitp", &start_hitp);
        process_word("max_hitp_allowed", &max_hitp_allowed);
        process_word("saving_allowed_first_level", &saving_allowed_first_level);
        process_word("saving_allowed_last_level", &saving_allowed_last_level);
        process_boolean("allow_triggering_any_tile", &allow_triggering_any_tile);
    }

    #undef process_word
    #undef process_boolean
    #undef check_ini_section
    return 0;
}

void load_options() {
    use_default_options();
    ini_load("SDLPoP.ini", ini_callback);
    if (!options.use_fixes_and_enhancements) disable_fixes_and_enhancements();
}

void show_use_fixes_and_enhancements_prompt() {
    if (options.use_fixes_and_enhancements != 2) return;
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
    while (options.use_fixes_and_enhancements == 2 ) {
		idle();
        switch (key_test_quit()) {
            case SDL_SCANCODE_Y:
                options.use_fixes_and_enhancements = 1;
                printf("Enabling game fixes and enhancements.\n");
                break;
            case SDL_SCANCODE_N:
                options.use_fixes_and_enhancements = 0;
                printf("Disabling game fixes and enhancements.\n");
                break;
		}
    }
    if (!options.use_fixes_and_enhancements) disable_fixes_and_enhancements();
}

