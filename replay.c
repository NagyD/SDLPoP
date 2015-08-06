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

#ifdef USE_REPLAY

#define MAX_REPLAY_DURATION 345600 // 8 hours: 720 * 60 * 8 ticks
byte moves[MAX_REPLAY_DURATION] = {0}; // static memory for now because it is easier (should this be dynamic?)

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
const char* const replay_file = "REPLAY.SAV";

byte* savestate_buffer = NULL;
size_t savestate_offset = 0;
size_t savestate_size = 0;
#define MAX_SAVESTATE_SIZE 4096

// These are defined in seg000.c:
typedef int process_func_type(void* data, size_t data_size);
extern int quick_process(process_func_type process_func);
extern const char const quick_version[];
extern char quick_control[];

void init_record_replay() {
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

int process_to_buffer(void* data, size_t data_size) {
    if (savestate_offset + data_size > MAX_SAVESTATE_SIZE) {
        printf("Saving savestate to memory failed: buffer is overflowing!\n");
        return 0;
    }
    memcpy(savestate_buffer + savestate_offset, data, data_size);
    savestate_offset += data_size;
    return 1;
}

int process_load_from_buffer(void* data, size_t data_size) {
    memcpy(data, savestate_buffer + savestate_offset, data_size);
    savestate_offset += data_size;
    return 1;
}

int savestate_to_buffer() {
    int ok = 0;
    if (savestate_buffer == NULL)
        savestate_buffer = malloc(MAX_SAVESTATE_SIZE);
    if (savestate_buffer != NULL) {
        savestate_offset = 0;
        savestate_size = 0;
        ok = quick_process(process_to_buffer);
        savestate_size = savestate_offset;
    }
    return ok;
}


int restore_savestate_from_buffer() {
    int ok = 0;
    savestate_offset = 0;
    while (savestate_offset < savestate_size) {
        ok = quick_process(process_load_from_buffer);
    }
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
        savestate_to_buffer(); // create a savestate in memory
        display_text_bottom("RECORDING");
        text_time_total = 24;
        text_time_remaining = 24;
    }

    replay_move curr_move = {0};
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

void start_replay() {
    replaying = 1;
    curr_tick = 0;
    load_recorded_replay();
}

void do_replay_move() {
    if (curr_tick == 0) {
        random_seed = saved_random_seed;
        seed_was_init = 1;
    }
    if (curr_tick == num_replay_ticks) { // recording is finished
        replaying = 0;
        start_level = 0;
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
    replay_fp = fopen(replay_file, "wb");
    if (replay_fp != NULL) {
        // embed a savestate into the replay
        fwrite(&savestate_size, sizeof(savestate_size), 1, replay_fp);
        fwrite(savestate_buffer, savestate_size, 1, replay_fp);
        // save the rest of the replay data
        num_replay_ticks = curr_tick;
        fwrite(&start_level, sizeof(start_level), 1, replay_fp);
        fwrite(&saved_random_seed, sizeof(saved_random_seed), 1, replay_fp);
        fwrite(&num_replay_ticks, sizeof(num_replay_ticks), 1, replay_fp);
        fwrite(moves, num_replay_ticks, 1, replay_fp);
        fclose(replay_fp);
    }
}

void load_recorded_replay() {
    replay_fp = fopen(replay_file, "rb");
    if (savestate_buffer == NULL)
        savestate_buffer = malloc(MAX_SAVESTATE_SIZE);
    if (replay_fp != NULL && savestate_buffer != NULL) {
        // load the savestate
        fread(&savestate_size, sizeof(savestate_size), 1, replay_fp);
        fread(savestate_buffer, savestate_size, 1, replay_fp);
        // load the rest of the replay data
        fread(&start_level, sizeof(start_level), 1, replay_fp);
        fread(&saved_random_seed, sizeof(saved_random_seed), 1, replay_fp);
        fread(&num_replay_ticks, sizeof(num_replay_ticks), 1, replay_fp);
        fread(moves, num_replay_ticks, 1, replay_fp);
        fclose(replay_fp);
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
            break;
    }
}

#endif // USE_REPLAY
