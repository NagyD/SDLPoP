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

TCCState *s = NULL;

// Script functions called from the main program:

void (*on_load_room)(int) = NULL;
void (*on_init_game)(void) = NULL;
void (*on_load_level)(int) = NULL;
void (*on_drink_potion)(int) = NULL;
void (*custom_potion_anim)(int) = NULL;

word* ptr_potion_color = NULL;
word* ptr_potion_pot_size = NULL;

// Functions callable by the script:

// Data encapsulation, because libtcc can only link with 'extern' variables through a pointer
// and doing all interesting data access through pointers may not be easy or fun...
// (Or perhaps there is another way to get libtcc to properly link with external data symbols?)

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
    s = tcc_new();
    if (!s) {
        fprintf(stderr, "Could not create tcc state\n");
        return 1;
    }

    tcc_add_include_path(s, "data/script/");
    tcc_set_lib_path(s, "data/script/"); // location of lib/libtcc1.a

    /* MUST BE CALLED before any compilation */
    tcc_set_output_type(s, TCC_OUTPUT_MEMORY);

    char* script_program = load_script("script.p1s"); // script must be an ANSI-encoded text file
    if (script_program == NULL) return 1;

    if (tcc_compile_string(s, script_program) == -1) {
        return 1;
    }

    // Data symbols accessible in the script:
    tcc_add_symbol(s, "ptr_level", &level);

    // Function symbols accessible in the script:
    tcc_add_symbol(s, "play_sound", play_sound);
    tcc_add_symbol(s, "stop_sounds", stop_sounds);
    tcc_add_symbol(s, "draw_kid_hp", draw_kid_hp);
    tcc_add_symbol(s, "take_hp", take_hp);
    tcc_add_symbol(s, "set_hp_full", set_health_life);

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

    /* relocate the code */
    if (tcc_relocate(s, TCC_RELOCATE_AUTO) < 0){
        return 1;
    }

    on_load_room = tcc_get_symbol(s, "on_load_room");
    on_init_game = tcc_get_symbol(s, "on_init_game");
    on_load_level = tcc_get_symbol(s, "on_load_level");
    on_drink_potion = tcc_get_symbol(s, "on_drink_potion");
    custom_potion_anim = tcc_get_symbol(s, "custom_potion_anim");

    /* delete the state */
    //tcc_delete(s);

    return 0;
}



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