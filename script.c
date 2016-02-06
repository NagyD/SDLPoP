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
#include <libtcc.h>

// The compiler state
TCCState *s = NULL;

// Script functions called from the main program:
void (*on_load_room)(int) = NULL;
void (*on_init_game)(void) = NULL;
void (*on_load_level)(int) = NULL;
void (*on_end_level)(void) = NULL;
void (*on_drink_potion)(int) = NULL;
void (*custom_potion_anim)(int) = NULL;
void (*custom_timers)(void) = NULL;

// Used by custom_potion_anim
word* ptr_potion_color = NULL;
word* ptr_potion_pot_size = NULL;

word override_start_sequence = 0;


#define SAVELIST_MAX_VARS 256
#define SAVELIST_MAX_VAR_SIZE 65536
#define SAVELIST_MAX_VAR_NAME_LEN 32

typedef struct savelist_var_type {
    void* var;
    int num_bytes;
    char name[SAVELIST_MAX_VAR_NAME_LEN];
} savelist_var_type;

savelist_var_type savelist[SAVELIST_MAX_VARS] = {{0}};
int savelist_num_vars = 0;
int savelist_size = 0;

// Functions callable by the script start below:

// Registered variables will be saved in and loaded from savestates (replays, quicksave)
void script__register_savestate_variable(void* source, int var_num_bytes, char* variable_name) {
    if (var_num_bytes <= 0) {
        fprintf(stderr, "Script: Error in register_savestate_variable \"%s\": invalid number of bytes given (%d)\n",
                variable_name, var_num_bytes);
        return;
    }
    if (var_num_bytes > SAVELIST_MAX_VAR_SIZE) {
        fprintf(stderr, "Script: Error in register_savestate_variable \"%s\": variable size is too large (%d)\n",
                variable_name, var_num_bytes);
        return;
    }
    if (savelist_num_vars >= SAVELIST_MAX_VARS) {
        fprintf(stderr, "Script: Error in register_savestate_variable \"%s\": limit of %d savestate variables reached\n",
                variable_name, SAVELIST_MAX_VARS);
        return;
    }
    ++savelist_num_vars;
    savelist_size += var_num_bytes;
    savelist[savelist_num_vars-1] = (savelist_var_type) {source, var_num_bytes, {0}};
    strncpy(savelist[savelist_num_vars-1].name, variable_name, SAVELIST_MAX_VAR_NAME_LEN);

    //printf("Registering savestate variable %s. Value = %d\n", variable_name, *((int*) source));
    // TODO: Make this work with the savestate code. (Savestates should get carefully serialized and deserialized)
}

word script__get_minutes_remaining(void) { return rem_min; }
word script__get_ticks_remaining(void) { return rem_tick; }

void script__set_time_remaining(word minutes, word ticks) {
    rem_min = minutes;
    rem_tick = ticks;
}

void script__select_tile(int room, int column, int row) {
    get_tile(room, column, row);
}

void script__select_tile_at_tilepos(word room, word tilepos) {
    get_tile(room, tilepos % 10, tilepos / 10);
}

byte script__get_curr_tile(void) {
    return curr_tile2;
}

byte script__get_curr_modifier(void) {
    if (curr_room > 0) {
        return curr_room_modif[curr_tilepos];
    }
    else return 0;
}

void script__set_curr_tile(byte new_tile) {
    if (curr_room > 0) {
        curr_room_tiles[curr_tilepos] = new_tile;
    }
}

void script__set_curr_modifier(byte new_modifier) {
    if (curr_room > 0) {
        curr_room_modif[curr_tilepos] = new_modifier;
    }
}

void script__set_curr_tile_and_modifier(byte new_tile, byte new_modifier) {
    if (curr_room > 0) {
        curr_room_tiles[curr_tilepos] = new_tile;
        curr_room_modif[curr_tilepos] = new_modifier;
    }
}

void script__set_tile(word room, word tilepos, byte new_tile) {
    if (room > 0 && room <= level.used_rooms && tilepos < 30) {
        get_room_address(room);
        curr_room_tiles[tilepos] = new_tile;
    }
}

void script__set_modifier(word room, word tilepos, byte new_modifier) {
    if (room > 0 && room <= level.used_rooms && tilepos < 30) {
        get_room_address(room);
        curr_room_modif[tilepos] = new_modifier;
    }
}

void script__set_tile_and_modifier(word room, word tilepos, byte new_tile, byte new_modifier) {
    if (room > 0 && room <= level.used_rooms && tilepos < 30) {
        get_room_address(room);
        curr_room_tiles[tilepos] = new_tile;
        curr_room_modif[tilepos] = new_modifier;
    }
}

word script__get_hp(void) {return hitp_curr; }

void script__set_hp(word new_hp) {
    hitp_delta = new_hp - hitp_curr;
}
word script__get_max_hp(void) {return hitp_max; }

void script__set_max_hp(word new_max_hp) {
    word old_hitp_max = hitp_max;
    hitp_max = new_max_hp;
    if (hitp_curr > hitp_max) hitp_curr = hitp_max; // remove excess health if necessary
    draw_kid_hp(hitp_curr, MAX(old_hitp_max, new_max_hp));
}

void script__set_flash(word color, word duration) {
    flash_color = color;
    flash_time = duration;
}

// Call this function only from within custom_potion_anim()
void script__set_potion_color(word color) {
    if (ptr_potion_color != NULL) {
        *ptr_potion_color = color;
    }
}

// Call this function only from within custom_potion_anim()
void script__set_potion_pot_size(word pot_size) {
    if (ptr_potion_pot_size != NULL) {
        *ptr_potion_pot_size = pot_size;
    }
}

void script__show_time(void) {
    text_time_remaining = 0;
    text_time_total = 0;
    is_show_time = 1;
}

short script__have_sword(void) { return have_sword; }
void script__set_have_sword(short kid_has_sword) { have_sword = kid_has_sword; }

word script__get_curr_level(void) { return current_level; }

void script__override_level_start_sequence(word sequence_index) {
    override_start_sequence = sequence_index;
}

// Not callable directly! This simply applies the 'reservation' made by override_level_start_sequence
// (this is automatically called shortly after)
void script__apply_override_level_start_sequence() {
    if (override_start_sequence != 0) {
        seqtbl_offset_char(override_start_sequence);
        override_start_sequence = 0;
    }
}

void script__disable_level1_music(void) {
    need_level1_music = 0;
}

// End of functions that can be called by scripts.


// Loads an ANSI text file into a newly allocated buffer and returns the buffer.
char* load_script(char* filename) {
    char* buffer = NULL;
    FILE* script_file = fopen(filename, "rb");
    fseek(script_file, 0, SEEK_END);
    off_t script_file_size = ftell(script_file);
    if (script_file_size > 0) {
        buffer = malloc(script_file_size + 1);
        if (buffer != NULL) {
            rewind(script_file);
            fread(buffer, 1, script_file_size, script_file);
            buffer[script_file_size] = '\0';
        }
    }
    fclose(script_file);
    return buffer;
}

int init_script() {
    if (!(enable_scripts && use_custom_levelset)) return 1; // only load scripts as part of mods

    char filename[256];
    snprintf(filename, sizeof(filename), "mods/%s/%s", levelset_name, "mod.p1s");
    char* script_program = load_script(filename); // script must be an ANSI-encoded text file
    if (script_program == NULL) return 1;

    s = tcc_new();
    if (s == NULL) {
        fprintf(stderr, "Could not create tcc state\n");
        return 1;
    }

    tcc_add_include_path(s, "data/script/"); // location of pop_script.h
    tcc_set_lib_path(s, "data/script/"); // location of lib/libtcc1.a
    tcc_set_output_type(s, TCC_OUTPUT_MEMORY);

    if (tcc_compile_string(s, script_program) == -1) {
        return 1;
    }
    free(script_program);

    // Data symbols accessible in the script.
    // They are linked as pointers. However, the script header file (pop_script.h) provides macros
    // that dereference those pointers, so you can still access the symbols as if they were "plain data" fields.
    tcc_add_symbol(s, "ptr_level", &level);
    tcc_add_symbol(s, "ptr_kid", &Kid);
    tcc_add_symbol(s, "ptr_guard", &Guard);

    // Function symbols accessible in the script:

    // Functions for proper communication between the main program and the script
    tcc_add_symbol(s, "register_savestate_variable_explicitly", script__register_savestate_variable);

    // PoP functions that can be called directly
    tcc_add_symbol(s, "play_sound", play_sound);
    tcc_add_symbol(s, "stop_sounds", stop_sounds);
    tcc_add_symbol(s, "draw_kid_hp", draw_kid_hp);
    tcc_add_symbol(s, "take_hp", take_hp);
    tcc_add_symbol(s, "set_hp_full", set_health_life);
    tcc_add_symbol(s, "set_char_sequence", seqtbl_offset_char);

    // Script functions
    tcc_add_symbol(s, "get_minutes_remaining", script__get_minutes_remaining);
    tcc_add_symbol(s, "get_ticks_remaining", script__get_ticks_remaining);
    tcc_add_symbol(s, "set_time_remaining", script__set_time_remaining);
    tcc_add_symbol(s, "select_tile_at_col_row", script__select_tile);
    tcc_add_symbol(s, "select_tile_at_tilepos", script__select_tile_at_tilepos);
    tcc_add_symbol(s, "get_curr_tile", script__get_curr_tile);
    tcc_add_symbol(s, "get_curr_modifier", script__get_curr_modifier);
    tcc_add_symbol(s, "set_curr_tile", script__set_curr_tile);
    tcc_add_symbol(s, "set_curr_modifier", script__set_curr_modifier);
    tcc_add_symbol(s, "set_curr_tile_and_modifier", script__set_curr_tile_and_modifier);
    tcc_add_symbol(s, "set_tile", script__set_tile);
    tcc_add_symbol(s, "set_modifier", script__set_modifier);
    tcc_add_symbol(s, "set_tile_and_modifier", script__set_tile_and_modifier);
    tcc_add_symbol(s, "get_hp", script__get_hp);
    tcc_add_symbol(s, "set_hp", script__set_hp);
    tcc_add_symbol(s, "get_max_hp", script__get_max_hp);
    tcc_add_symbol(s, "set_max_hp", script__set_max_hp);
    tcc_add_symbol(s, "set_flash", script__set_flash);
    tcc_add_symbol(s, "set_potion_color", script__set_potion_color);
    tcc_add_symbol(s, "set_potion_pot_size", script__set_potion_pot_size);
    tcc_add_symbol(s, "show_time", script__show_time);
    tcc_add_symbol(s, "have_sword", script__have_sword);
    tcc_add_symbol(s, "set_have_sword", script__set_have_sword);
    tcc_add_symbol(s, "get_curr_level", script__get_curr_level);
    tcc_add_symbol(s, "override_level_start_sequence", script__override_level_start_sequence);
    tcc_add_symbol(s, "disable_level1_music", script__disable_level1_music);

    /* relocate the code */
    if (tcc_relocate(s, TCC_RELOCATE_AUTO) < 0){
        return 1;
    }

    // Look for script entry points
    on_load_room = tcc_get_symbol(s, "on_load_room");
    on_init_game = tcc_get_symbol(s, "on_init_game");
    on_load_level = tcc_get_symbol(s, "on_load_level");
    on_end_level = tcc_get_symbol(s, "on_end_level");
    on_drink_potion = tcc_get_symbol(s, "on_drink_potion");
    custom_potion_anim = tcc_get_symbol(s, "custom_potion_anim");
    custom_timers = tcc_get_symbol(s, "custom_timers");

    /* delete the state */
    //tcc_delete(s);

    return 0;
}


// Functions that invoke the script:

void script__on_load_room(int room) {
    if (on_load_room != NULL) on_load_room(room);
    get_room_address(drawn_room); // careful, scripted on_load_room() might change curr_room_tiles[]/modif[]!
}

void script__on_init_game(void) {
    if (on_init_game != NULL) on_init_game();
}

void script__on_load_level(int level_number) {
    if (on_load_level != NULL) on_load_level(level_number);
}

void script__on_end_level(int level_number) {
    if (on_end_level != NULL) on_end_level();
}

void script__on_drink_potion(int potion_id) {
    if (on_drink_potion != NULL) on_drink_potion(potion_id);
}

void script__custom_potion_anim(int potion_id, word *color, word *pot_size) {
    // do not expose raw pointers in the script!
    // instead, we can call set_potion_color() and set_potion_pot_size() while we are in custom_potion_anim()
    ptr_potion_color = color;
    ptr_potion_pot_size = pot_size;
    if (custom_potion_anim != NULL) custom_potion_anim(potion_id);

    // safety: set_potion_color() and set_potion_pot_size() will not do anything when the pointers are NULL
    ptr_potion_color = NULL;
    ptr_potion_pot_size = NULL;
}

void script__custom_timers() {
    if (custom_timers != NULL) custom_timers();
}

// TODO: add script events: on_quicksave(), on_quickload()
// TODO: allow scripts to store their local data in savestates (perhaps a small stack using unused level fields?)
// TODO: allow scripts to modify the next level


