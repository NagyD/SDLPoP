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

#ifdef USE_SCRIPT

#include <libtcc.h>

typedef struct script_type {
    void (*on_init)(void);
    void (*on_load_room)(int);
    void (*on_start_game)(void);
    void (*on_load_level)(int);
    void (*on_end_level)(int);
    void (*on_drink_potion)(int);
    void (*custom_potion_anim)(int);
    void (*custom_timers)(void);
} script_type;

#define MAX_SCRIPTS 10
script_type scripts[MAX_SCRIPTS];
word num_active_scripts = 0;

// Used by custom_potion_anim
word* ptr_potion_color = NULL;
word* ptr_potion_pot_size = NULL;

// Used by on_level_end
word* ptr_next_level = NULL;

// Variable used in on_load_level, acts as a temporary 'reservation'
// for overriding the kid's level entry sequence (running, turning, falling, etc.)
word override_start_sequence = 0;

#define SAVELIST_MAX_VARS 256
#define SAVELIST_MAX_VAR_SIZE 65536
#define SAVELIST_MAX_VAR_NAME_LEN 64

#define SAVESTATE_OPTIONVARS_HEADER_BYTE 'O'
#define SCRIPT_SAVELIST_HEADER_BYTE 'S'

typedef struct savelist_var_type {
    byte name_len;
    char name[SAVELIST_MAX_VAR_NAME_LEN];
    word data_size;
    void* data;
} savelist_var_type;

typedef struct savelist_type {
    int num_vars;
    savelist_var_type vars[SAVELIST_MAX_VARS];
} savelist_type;

// List of script-defined variable names (and associated data pointers), for which the script has requested
// storage in savestates (i.e. these vars also get saved in quicksaves and replays)
savelist_type script_savelist;

int load_script(char* filename); // forward declaration

// Writing and reading registered script variables to and from savestates:

void savelist_save(savelist_type* savelist, process_func_type save_func, void* stream) {
    int num_vars = savelist->num_vars;
    save_func(&num_vars, sizeof(num_vars), stream);
    int i;
    for (i = 0; i < num_vars; ++i) {
        savelist_var_type* var = &savelist->vars[i];
        byte var_name_len = var->name_len;
        word var_data_size = var->data_size;
        save_func(&var_name_len, sizeof(var_name_len), stream);
        save_func(var->name, var_name_len, stream);
        save_func(&var_data_size, sizeof(var_data_size), stream);
        save_func(var->data, var_data_size, stream);
    }
}

void savelistvar_deserialize(savelist_var_type* var_dest, void* data_buffer, process_func_type load_func, void* stream) {
    byte name_len = 0;
    word data_size = 0;

    load_func(&name_len, sizeof(name_len), stream);
    name_len = (byte) MIN(name_len, SAVELIST_MAX_VAR_NAME_LEN);
    var_dest->name_len = name_len;
    load_func(var_dest->name, name_len, stream);

    load_func(&data_size, sizeof(data_size), stream);
    data_size = (word) MIN(data_size, SAVELIST_MAX_VAR_SIZE);
    var_dest->data_size = data_size;

    load_func(data_buffer, data_size, stream); // this retrieves the actual data of the variable
}

void savelist_load(savelist_type *savelist, int num_vars_read, process_func_type load_func, void *stream) {
    // Reserve enough memory as a buffer for the largest possible savelist variable
    byte* var_buffer = malloc(SAVELIST_MAX_VAR_SIZE);

    // Read savestate's variables
    int i;
    for (i = 0; i < num_vars_read; ++i) {
        savelist_var_type the_var = {0};
        savelistvar_deserialize(&the_var, var_buffer, load_func, stream);

        // Match with the script's registered variables
        int curr_var_id;
        for (curr_var_id = 0; curr_var_id < savelist->num_vars; ++curr_var_id) {
            if (strncmp(the_var.name, savelist->vars[curr_var_id].name, SAVELIST_MAX_VAR_NAME_LEN) == 0) {
                goto found;
            }
        }
        fprintf(stderr, "Warning: Savestate contains unregistered variable \"%s\".\n", the_var.name);
        continue; // Matching script var not found, discard and read the next var in the savestate

        found:
        {
            // Matching script var found, try to replace that var's data with the data from the savestate
            savelist_var_type* found_var = &savelist->vars[curr_var_id];
            word savelist_var_data_size = found_var->data_size;

            if (savelist_var_data_size != the_var.data_size) {
                fprintf(stderr, "Warning: Restored savestate variable \"%s\" has an unexpected size "
                                "(%d bytes, expected %d bytes).\n",
                        found_var->name, the_var.data_size, savelist_var_data_size);
            }
            memset(found_var->data, 0, savelist_var_data_size);
            memcpy(found_var->data, var_buffer, MIN(the_var.data_size, savelist_var_data_size));
        }
    }
    free(var_buffer);
}

void script__write_savelist(FILE*fp) {
    if (fp != NULL) {
        fputc(SCRIPT_SAVELIST_HEADER_BYTE, fp);
        fputc(strnlen(levelset_name, 255), fp);
        fputs(levelset_name, fp);
        savelist_save(&script_savelist, (process_func_type) process_save_to_file, fp);
    }
}

void script__read_savelist(FILE*fp) {
    if (fp != NULL) {
        // Confirm that script variables are actually included in the savestate
        byte header_byte = (byte) fgetc(fp);
        if (feof(fp) || ferror(fp)) {
            if (script_savelist.num_vars > 0) {
                fprintf(stderr, "Warning: Script variables cannot be restored: not found in savestate (expected %d).\n",
                        script_savelist.num_vars);
            }
            return;
        }
        if (header_byte != SCRIPT_SAVELIST_HEADER_BYTE) {
            fseek(fp, -1, SEEK_CUR); // not a savelist
            return;
        }

        // Check that the correct script for the savestate is also active (levelset name)
        char levelset_name_read[256];
        byte levelset_name_len_read = (byte) fgetc(fp);
        fread(levelset_name_read, sizeof(char), levelset_name_len_read, fp);
        levelset_name_read[levelset_name_len_read] = '\0';
        if (strcmp(levelset_name_read, levelset_name) != 0) {
            fprintf(stderr, "Warning: Loading savestate created by \"%s\", but the active levelset is \"%s\"\n",
                    levelset_name_read, levelset_name);
        }

        int savelist_num_vars_read = 0;
        fread(&savelist_num_vars_read, sizeof(savelist_num_vars_read), 1, fp);
        if (script_savelist.num_vars != savelist_num_vars_read) {
            fprintf(stderr, "Warning: Found %d script variables in savestate; does not match "
                    "number expected by the active script (%d).\n", savelist_num_vars_read, script_savelist.num_vars);
        }

        savelist_load(&script_savelist, savelist_num_vars_read,
                      (process_func_type) process_load_from_file, fp);
    }
}


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
    if (script_savelist.num_vars >= SAVELIST_MAX_VARS) {
        fprintf(stderr, "Script: Error in register_savestate_variable \"%s\": limit of %d savestate variables reached\n",
                variable_name, SAVELIST_MAX_VARS);
        return;
    }
    ++script_savelist.num_vars;
    savelist_var_type* new_var = &script_savelist.vars[script_savelist.num_vars-1];
    *new_var = (savelist_var_type) {(byte) strnlen(variable_name, SAVELIST_MAX_VAR_NAME_LEN),
                                 {0}, (word) var_num_bytes, source};
    strncpy(new_var->name, variable_name, SAVELIST_MAX_VAR_NAME_LEN);

    //printf("Registering savestate variable %s. Value = %d\n", variable_name, *((int*) source));
}

void script__load_additional_script(char* filename) {
    if (num_active_scripts < MAX_SCRIPTS) {
        char filename2[256];
        snprintf(filename2, sizeof(filename2), "mods/%s/%s", levelset_name, filename);
        //printf("Loading additional script: %s\n", filename2);
        load_script(filename2);
    }
    else {
        printf("Cannot load script %s, maximum active scripts reaches (%d)\n", filename, MAX_SCRIPTS);
    }
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
word script__is_leveldoor_open(void) { return leveldoor_open; }

// Call this only from on_level_load
void script__set_level_start_sequence(word sequence_index) {
    override_start_sequence = sequence_index;
}

// Call this only from within on_level_end
void script__set_next_level(word level_number) {
    if (ptr_next_level != NULL) {
        *ptr_next_level = level_number;
    }
}

// Not callable directly! This simply applies the 'reservation' made by override_level_start_sequence
// (this is automatically called shortly after)
void script__apply_set_level_start_sequence() {
    if (override_start_sequence != 0) {
        seqtbl_offset_char(override_start_sequence);
        override_start_sequence = 0;
    }
}

void script__disable_level1_music(void) {
    need_level1_music = 0;
}

void script__show_dialog(char* text) {
    word key;
    rect_type rect;
    screen_updates_suspended = 1;
    method_1_blit_rect(offscreen_surface, onscreen_surface_, &copyprot_dialog->peel_rect, &copyprot_dialog->peel_rect, 0);
    draw_dialog_frame(copyprot_dialog);
    shrink2_rect(&rect, &copyprot_dialog->text_rect, 2, 1);
    show_text_with_color(&rect, 0, 0, text, color_15_brightwhite);
    screen_updates_suspended = 0;
    request_screen_update();
    clear_kbd_buf();
    bool controls_not_yet_released = true;
    do {
        idle();
        key = key_test_quit(); // Press any key to continue...

        // We want to check that the arrow keys have been completely released, before the dialog box can close.
        // Otherwise, the dialog could just blink once and immediately disappear!
        if (controls_not_yet_released) {
            control_y = 0;
            control_x = 0;
            if (is_joyst_mode) {
                read_joyst_control();
            } else {
                read_keyb_control();
            }
            controls_not_yet_released = (control_x || control_y);
        }
    } while(controls_not_yet_released || key == 0);
    savekid();
    redraw_screen(0);
    loadkid();
    return;
}


// End of functions that can be called by scripts.


// Loads an ANSI text file into a newly allocated buffer and returns the buffer.
char* load_script_text(char* filename) {
    char* buffer = NULL;
    FILE* script_file = fopen(filename, "rb");
    if (script_file == NULL) {
        return NULL;
    }
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

int load_script(char* filename) {
    char* script_program = load_script_text(filename); // script must be an ANSI-encoded text file
    if (script_program == NULL) return 1;

    // Create the compiler state
    TCCState *s = tcc_new();
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

    // Callable functions defined in script.c (this file) with a "meta" purpose
    tcc_add_symbol(s, "load_script", script__load_additional_script);
    tcc_add_symbol(s, "register_savestate_variable_explicitly", script__register_savestate_variable);

    // PoP functions that can be called directly
    tcc_add_symbol(s, "play_sound", play_sound);
    tcc_add_symbol(s, "stop_sounds", stop_sounds);
    tcc_add_symbol(s, "draw_kid_hp", draw_kid_hp);
    tcc_add_symbol(s, "take_hp", take_hp);
    tcc_add_symbol(s, "set_hp_full", set_health_life);
    tcc_add_symbol(s, "set_char_sequence", seqtbl_offset_char);

    // Extra callable functions defined in script.c (this file)
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
    tcc_add_symbol(s, "is_leveldoor_open", script__is_leveldoor_open);
    tcc_add_symbol(s, "set_next_level", script__set_next_level);
    tcc_add_symbol(s, "set_level_start_sequence", script__set_level_start_sequence);
    tcc_add_symbol(s, "disable_level1_music", script__disable_level1_music);
    tcc_add_symbol(s, "show_dialog", script__show_dialog);

    /* relocate the code */
    if (tcc_relocate(s, TCC_RELOCATE_AUTO) < 0){
        return 1;
    }

    // Look for script entry points and store them for this script
    script_type* script = &scripts[num_active_scripts];
    ++num_active_scripts;
    script->on_init = tcc_get_symbol(s, "on_init");
    script->on_load_room = tcc_get_symbol(s, "on_load_room");
    script->on_start_game = tcc_get_symbol(s, "on_start_game");
    script->on_load_level = tcc_get_symbol(s, "on_load_level");
    script->on_end_level = tcc_get_symbol(s, "on_end_level");
    script->on_drink_potion = tcc_get_symbol(s, "on_drink_potion");
    script->custom_potion_anim = tcc_get_symbol(s, "custom_potion_anim");
    script->custom_timers = tcc_get_symbol(s, "custom_timers");

    if (script->on_init != NULL) {
        script->on_init(); // on_init called in the script itself
    }

    // Intentional leak: don't free the compilation context (that would also delete the compiled code)
    return 0;
}

int init_script() {
    if (!(enable_scripts && use_custom_levelset)) return 1; // only load scripts as part of mods
    char filename[256];
    snprintf(filename, sizeof(filename), "mods/%s/%s", levelset_name, "mod.scr");
    return load_script(filename);
}




// Functions that invoke the script:

void script__on_load_room(int room) {
    for (int i = num_active_scripts-1; i >= 0; --i ) {
        if (scripts[i].on_load_room != NULL) scripts[i].on_load_room(room);
    }

    get_room_address(drawn_room); // careful, scripted on_load_room() might change curr_room_tiles[]/modif[]!
}

void script__on_start_game(void) {
    #ifdef USE_REPLAY
    if (replaying) return;
    #endif
    for (int i = num_active_scripts-1; i >= 0; --i ) {
        if (scripts[i].on_start_game != NULL) {
            scripts[i].on_start_game();
        }
    }
}

void script__on_load_level(int level_number) {
    override_start_sequence = 0;
    for (int i = num_active_scripts-1; i >= 0; --i ) {
        if (scripts[i].on_load_level != NULL) {
            scripts[i].on_load_level(level_number);
        }
    }
}

void script__on_end_level(int level_number, word* next_level_number) {
    // ptr_next_level is used by set_next_level
    ptr_next_level = next_level_number; // do not expose raw pointers in the script
    for (int i = num_active_scripts-1; i >= 0; --i ) {
        if (scripts[i].on_end_level != NULL) {
            scripts[i].on_end_level(level_number);
        }
    }
    ptr_next_level = NULL; // safety
}

void script__on_drink_potion(int potion_id) {
//    if (on_drink_potion != NULL) on_drink_potion(potion_id);
    for (int i = num_active_scripts-1; i >= 0; --i ) {
        if (scripts[i].on_drink_potion != NULL) {
            scripts[i].on_drink_potion(potion_id);
        }
    }
}

void script__custom_potion_anim(int potion_id, word *color, word *pot_size) {
    // do not expose raw pointers in the script!
    // instead, we can call set_potion_color() and set_potion_pot_size() while we are in custom_potion_anim()
    ptr_potion_color = color;
    ptr_potion_pot_size = pot_size;
    for (int i = num_active_scripts-1; i >= 0; --i ) {
        if (scripts[i].custom_potion_anim != NULL) {
            scripts[i].custom_potion_anim(potion_id);
        }
    }

    // safety: set_potion_color() and set_potion_pot_size() will not do anything when the pointers are NULL
    ptr_potion_color = NULL;
    ptr_potion_pot_size = NULL;
}

void script__custom_timers() {
    for (int i = num_active_scripts-1; i >= 0; --i ) {
        if (scripts[i].custom_timers != NULL) {
            scripts[i].custom_timers();
        }
    }
}

#endif
