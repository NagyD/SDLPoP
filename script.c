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
        printf("Aah, compile string failed!");
        return 1;
    }

    // Data symbols accessible in the script:
    tcc_add_symbol(s, "ptr_level", &level);

    // Function symbols accessible in the script:
    tcc_add_symbol(s, "play_sound", play_sound);

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
    tcc_add_symbol(s, "set_tile_and_modifier", script__set_modifier);

    /* relocate the code */
    if (tcc_relocate(s, TCC_RELOCATE_AUTO) < 0){
        return 1;
    }

    on_load_room = tcc_get_symbol(s, "on_load_room");
    on_init_game = tcc_get_symbol(s, "on_init_game");
    on_load_level = tcc_get_symbol(s, "on_load_level");

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

