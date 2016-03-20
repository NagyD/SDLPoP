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

#ifdef USE_REPLAY

#define MAX_REPLAY_DURATION 345600 // 8 hours: 720 * 60 * 8 ticks
byte moves[MAX_REPLAY_DURATION] = {0}; // static memory for now because it is easier (should this be dynamic?)
options_type replay_options; // Need to know what gameplay options are active during recording, for reproducability
options_type stored_options;

enum special_moves {
    MOVE_RESTART_LEVEL = 1,
};

// 1-byte structure representing which controls were active at a particular game tick
typedef union replay_move {
    struct {
        sbyte x : 2;
        sbyte y : 2;
        byte shift : 1;
        byte special : 3;
    };
    byte bits;
} replay_move;

dword curr_tick = 0;
dword saved_random_seed;
byte special_move = 0;

FILE* replay_fp;
#define REPLAY_DEFAULT_FILENAME "REPLAY_001.P1R"
const char replay_version[] = "V1.16b3 ";
char replay_control[] = "........";
byte replay_file_open = 0;
word current_replay_number = 0;

// Helper type, similar to the FILE struct, so that we can keep track of a 'current position' within a chunk of memory
typedef struct membuf_type {
    byte* curr_ptr;
    size_t content_size;
    byte* base_ptr;
    byte* end_ptr; // initialized to base_ptr + max buffer size
} membuf_type;

membuf_type savestate_membuf;
#define SAVESTATE_MEMBUF_SIZE 4096 // more than enough for a savestate

// These are defined in seg000.c:
extern const char quick_version[9];
extern char quick_control[9];

byte open_replay_file(const char *filename) {
    if (replay_file_open) fclose(replay_fp);
    replay_fp = fopen(filename, "rb");
    if (replay_fp != NULL) {
        replay_file_open = 1;
        return 1;
    }
    else {
        replay_file_open = 0;
        return 0;
    }
}

void init_membuf(membuf_type* buf, size_t buffer_size) {
    buf->curr_ptr = buf->base_ptr = malloc(buffer_size);
    buf->end_ptr = buf->base_ptr + buffer_size;
}

void reset_membuf(membuf_type* buf) {
    buf->content_size = 0;
    buf->curr_ptr = buf->base_ptr;
}

void init_record_replay() {
    if (!options.enable_replay) return;
    init_membuf(&savestate_membuf, SAVESTATE_MEMBUF_SIZE); // 4096 should be more than enough for a savestate
    if (g_argc > 1) {
        char *filename = g_argv[1]; // file dragged on top of executable or double clicked
        char *e = strrchr(filename, '.');
        if (e != NULL && strcasecmp(e, ".P1R") == 0) { // valid replay filename passed as first arg
            open_replay_file(filename);
            start_replay();
        }
    }
    if (check_param("record")) {
        start_recording();
    }
    else if (check_param("replay")) {
        start_replay();
    }
}

void replay_restore_level() {
    // Need to restore the savestate at the right time (just before the first room of the level is drawn).
    // Otherwise, for "on-the-fly" recordings, the screen will visibly "jump" to the replay savestate.
    // This only needs to happen at the very beginning of the replay (curr_tick == 0)
    if (curr_tick == 0) restore_savestate_from_buffer();
}

int process_to_membuf(void* data, size_t data_size, membuf_type* buf) {
    if (buf->curr_ptr + data_size > buf->end_ptr) {
        printf("Saving savestate to memory failed: buffer is overflowing!\n");
        return 0;
    }
    memcpy(buf->curr_ptr, data, data_size);
    buf->curr_ptr += data_size;
    buf->content_size += data_size;
    return 1;
}

int process_from_membuf(void* data, size_t data_size, membuf_type* buf) {
    if (buf->curr_ptr + data_size - buf->base_ptr > buf->content_size) {
        printf("Reading savestate from memory failed: reading past end of buffer!\n");
        return 0;
    }
    memcpy(data, buf->curr_ptr, data_size);
    buf->curr_ptr += data_size;
    buf->content_size += data_size;
    return 1;
}

int restore_savestate_from_buffer() {
    savestate_membuf.curr_ptr = savestate_membuf.base_ptr; // rewind the memory buffer for reading
    int ok = quick_process((process_func_type) process_from_membuf, &savestate_membuf);
    restore_room_after_quick_load();
    return ok;
}

void start_recording() {
    curr_tick = 0;
    recording = 1; // further set-up is done in add_replay_move, on the first gameplay tick
}

void add_replay_move() {
    if (curr_tick == 0) {
        prandom(1); // make sure random_seed is initialized
        saved_random_seed = random_seed;
        seed_was_init = 1;
        // create a savestate in memory
        reset_membuf(&savestate_membuf); // rewind and reset the buffer contents so it will be clean to use
        quick_process((process_func_type) process_to_membuf, &savestate_membuf);

        display_text_bottom("RECORDING");
        text_time_total = 24;
        text_time_remaining = 24;
    }

    replay_move curr_move = {{0}};
    curr_move.x = control_x;
    curr_move.y = control_y;
    if (control_shift) curr_move.shift = 1;

    if (special_move)  {
        curr_move.special = special_move;
        special_move = 0;
    }

    moves[curr_tick] = curr_move.bits;

    ++curr_tick;

    if (curr_tick >= MAX_REPLAY_DURATION) { // max replay length exceeded
        stop_recording();
    }
}

void stop_recording() {
    save_recorded_replay();
    recording = 0;
    display_text_bottom("REPLAY SAVED");
    text_time_total = 24;
    text_time_remaining = 24;
}

void apply_replay_options() {
    stored_options = options;
    options = replay_options;
    if (!options.use_fixes_and_enhancements) disable_fixes_and_enhancements();
    // but it does not make sense to apply these as well:
    options.enable_mixer = stored_options.enable_mixer;
    options.enable_fade = stored_options.enable_fade;
    options.enable_flash = stored_options.enable_flash;
    options.enable_text = stored_options.enable_text;
    options.enable_replay = 1; // just to be safe...
    options.enable_quicksave = 1;

}

void apply_stored_options() {
    options = stored_options;
    if (!options.use_fixes_and_enhancements) disable_fixes_and_enhancements();
}

void start_replay() {
    if (!options.enable_replay) return;
    replaying = 1;
    curr_tick = 0;
    load_replay();
    apply_replay_options();
}

void do_replay_move() {
    if (curr_tick == 0) {
        random_seed = saved_random_seed;
        seed_was_init = 1;
    }
    if (curr_tick == num_replay_ticks) { // replay is finished
        replaying = 0;
        start_level = 0;
        apply_stored_options();
        start_game();
    }

    replay_move curr_move;
    curr_move.bits = moves[curr_tick];

    control_x = curr_move.x;
    control_y = curr_move.y;
    control_shift = (curr_move.shift) ? -1 : 0;

    if (curr_move.special == MOVE_RESTART_LEVEL) { // restart level
        stop_sounds();
        is_restart_level = 1;
    }

//    if (curr_tick > 5 ) printf("rem_tick: %d\t curr_tick: %d\tlast 5 moves: %d, %d, %d, %d, %d\n", rem_tick, curr_tick,
//                               moves[curr_tick-4], moves[curr_tick-3], moves[curr_tick-2], moves[curr_tick-1], moves[curr_tick]);
    ++curr_tick;
}

void save_recorded_replay() {
    char filename[] = REPLAY_DEFAULT_FILENAME;
    word replay_number = 1;
    while (access(filename, F_OK) != -1) { // file already exists
        ++replay_number;
        sprintf(filename +7, "%03d.P1R", replay_number);
    }
    replay_fp = fopen(filename, "wb");
    if (replay_fp != NULL) {
        fwrite(replay_version, COUNT(replay_version), 1, replay_fp);
        fwrite(quick_version, COUNT(quick_version), 1, replay_fp);
        // embed a savestate into the replay
        size_t savestate_size = savestate_membuf.content_size;
        fwrite(&savestate_size, sizeof(savestate_size), 1, replay_fp);
        fwrite(savestate_membuf.base_ptr, savestate_size, 1, replay_fp);
        // save the rest of the replay data
        fwrite(&options, sizeof(options), 1, replay_fp);
        fwrite(&start_level, sizeof(start_level), 1, replay_fp);
        fwrite(&saved_random_seed, sizeof(saved_random_seed), 1, replay_fp);
        num_replay_ticks = curr_tick;
        fwrite(&num_replay_ticks, sizeof(num_replay_ticks), 1, replay_fp);
        fwrite(moves, num_replay_ticks, 1, replay_fp);
#ifdef USE_SCRIPT
        if (enable_scripts) script__write_savelist(replay_fp);
#endif
        fclose(replay_fp);
    }
}

byte open_next_replay_file() {
    char filename[] = REPLAY_DEFAULT_FILENAME;
    ++current_replay_number;
    int try;
#define MAX_REPLAY_NUMBER 999
    for (try = 0; try < current_replay_number + MAX_REPLAY_NUMBER; ++try) {
        sprintf(filename +7, "%03d.P1R", current_replay_number);
        if (open_replay_file(filename)) break;
        ++current_replay_number;
        if (current_replay_number > MAX_REPLAY_NUMBER) current_replay_number = 1; // cycle back to the first replay if necessary
    }
    if (!replay_file_open) return 0;
    else return 1;
}

void replay_cycle() {
    need_replay_cycle = 0;
    stop_sounds();
    if (!open_next_replay_file()) return; // failed: can't find replays
    load_replay();
    curr_tick = 0;
    apply_replay_options();
    restore_savestate_from_buffer();
    char message[] = "001";
    sprintf(message, "%03d", current_replay_number);
    display_text_bottom(message);
    text_time_remaining = 24;
    text_time_total = 24;
}

void load_replay() {
    if (!replay_file_open) {
        current_replay_number = 1;
        if (!open_replay_file(REPLAY_DEFAULT_FILENAME)) {
            open_next_replay_file();
        }
    }
    if (replay_fp != NULL) {
        fread(replay_control, COUNT(replay_control), 1, replay_fp);
        if (strcmp(replay_control, replay_version) != 0) {
            printf("Warning: unexpected replay format!\n");
        }
        fread(quick_control, COUNT(quick_control), 1, replay_fp);
        if (strcmp(quick_control, quick_version) != 0) {
            printf("Warning: unexpected savestate format!\n");
        }
        // copy the savestate into a memory buffer
        size_t savestate_size;
        fread(&savestate_size, sizeof(savestate_size), 1, replay_fp);
        fread(savestate_membuf.base_ptr, savestate_size, 1, replay_fp);
        savestate_membuf.content_size = savestate_size;

        // load the rest of the replay data
        fread(&replay_options, sizeof(replay_options), 1, replay_fp);
        fread(&start_level, sizeof(start_level), 1, replay_fp);
        fread(&saved_random_seed, sizeof(saved_random_seed), 1, replay_fp);
        fread(&num_replay_ticks, sizeof(num_replay_ticks), 1, replay_fp);
        fread(moves, num_replay_ticks, 1, replay_fp);
#ifdef USE_SCRIPT
        if (enable_scripts) script__read_savelist(replay_fp);
#endif
        fclose(replay_fp);
        replay_file_open = 0;
    }
}

void key_press_while_recording(int* key_ptr) {
    int key = *key_ptr;
    switch(key) {
        case SDL_SCANCODE_A | WITH_CTRL:
            special_move = MOVE_RESTART_LEVEL;
            break;
        case SDL_SCANCODE_R | WITH_CTRL:
            save_recorded_replay();
            recording = 0;
        default:
            break;
    }
}

void key_press_while_replaying(int* key_ptr) {
    int key = *key_ptr;
    switch(key) {
        default:
            // cannot manually do most stuff during a replay!
            *key_ptr = 0;
            break;
        // but these are allowable actions:
        case SDL_SCANCODE_ESCAPE:               // pause
        case SDL_SCANCODE_ESCAPE | WITH_SHIFT:
        case SDL_SCANCODE_SPACE:                // time
        case SDL_SCANCODE_S | WITH_CTRL:        // sound toggle
        case SDL_SCANCODE_V | WITH_CTRL:        // version
        case SDL_SCANCODE_C:                    // room numbers
        case SDL_SCANCODE_C | WITH_SHIFT:
        case SDL_SCANCODE_I | WITH_SHIFT:       // invert
        case SDL_SCANCODE_B | WITH_SHIFT:       // blind
        case SDL_SCANCODE_T:                    // debug time
            break;
        case SDL_SCANCODE_R | WITH_CTRL:        // restart game
            replaying = 0;
            apply_stored_options();
            break;
        case SDL_SCANCODE_TAB:
            need_replay_cycle = 1;
            break;
    }
}

#if 0
savelist_type replay_savelist;

void replay__register_savelist_var(char* name, size_t name_len, void* data, size_t data_size) {
    ++replay_savelist.num_vars;
    savelist_var_type* new_var = &replay_savelist.vars[replay_savelist.num_vars-1];
    new_var->name_len = (byte) name_len;
    memcpy(new_var->name, name, name_len);
    new_var->data = data;
    new_var->data_size = (word) data_size;
}

void replay__init_savelist() {
    #define register_var(var) replay__register_savelist_var(#var, sizeof(#var)-1, &var, sizeof(var))
    register_var(options.enable_copyprot);
    register_var(options.use_fixes_and_enhancements);
    register_var(options.enable_quicksave_penalty);
    register_var(options.enable_crouch_after_climbing);
    register_var(options.enable_freeze_time_during_end_music);
    register_var(options.fix_gate_sounds);
    register_var(options.fix_two_coll_bug);
    register_var(options.fix_infinite_down_bug);
    register_var(options.fix_gate_drawing_bug);
    register_var(options.fix_bigpillar_climb);
    register_var(options.fix_jump_distance_at_edge);
    register_var(options.fix_edge_distance_check_when_climbing);
    register_var(options.fix_painless_fall_on_guard);
    register_var(options.fix_wall_bump_triggers_tile_below);
    register_var(options.fix_stand_on_thin_air);
    register_var(options.fix_press_through_closed_gates);
    register_var(options.fix_grab_falling_speed);
    register_var(options.fix_skeleton_chomper_blood);
    register_var(options.fix_move_after_drink);
    register_var(options.fix_loose_left_of_potion);
    register_var(options.fix_guard_following_through_closed_gates);
    register_var(options.fix_safe_landing_on_spikes);
    register_var(options.enable_remember_guard_hp);
    register_var(options.fix_glide_through_wall);
    register_var(options.fix_drop_through_tapestry);
    register_var(options.fix_land_against_gate_or_tapestry);
}

void replay__read_savelist(FILE* stream) {
    // backup savelist to a temporary buffer (stored options)
        // for each var: process to buffer
}

void savelist_to_buffer(void* buffer) {

}
#endif

#endif // USE_REPLAY
