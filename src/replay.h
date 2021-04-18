/*
SDLPoP, a port/conversion of the DOS game Prince of Persia.
Copyright (C) 2013-2020  Dávid Nagy

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
#include <time.h>

#ifdef USE_REPLAY

const char replay_magic_number[3] = "P1R";
const word replay_format_class = 0;          // unique number associated with this SDLPoP implementation / fork
const char* implementation_name = "SDLPoP v" SDLPOP_VERSION;

#define REPLAY_FORMAT_CURR_VERSION       102 // current version number of the replay format
#define REPLAY_FORMAT_MIN_VERSION        101 // SDLPoP will open replays with this version number and higher
#define REPLAY_FORMAT_DEPRECATION_NUMBER 1   // SDLPoP won't open replays with a higher deprecation number

#define MAX_REPLAY_DURATION 345600 // 8 hours: 720 * 60 * 8 ticks
byte moves[MAX_REPLAY_DURATION] = {0}; // static memory for now because it is easier (should this be dynamic?)

char replay_levelset_name[POP_MAX_PATH];
char stored_levelset_name[POP_MAX_PATH];

// 1-byte structure representing which controls were active at a particular game tick
typedef union replay_move_type {
 struct {
  sbyte x : 2;
  sbyte y : 2;
  byte shift : 1;
  byte special : 3; // enum replay_special_moves, see types.h
 };
 byte bits;
} replay_move_type;

//dword curr_tick = 0;

FILE* replay_fp = NULL;
byte replay_file_open = 0;
int current_replay_number = 0;
int next_replay_number = 0;

byte* savestate_buffer = NULL;
dword savestate_offset = 0;
dword savestate_size = 0;
#define MAX_SAVESTATE_SIZE 4096

// These are defined in seg000.c:
typedef int process_func_type(void* data, size_t data_size);
extern int quick_process(process_func_type process_func);
extern const char quick_version[9];

// header information read from the first part of a replay file
typedef struct replay_header_type {
 byte uses_custom_levelset;
 char levelset_name[POP_MAX_PATH];
 char implementation_name[POP_MAX_PATH];
} replay_header_type;

// information needed to keep track of all listed replay files, and to sort them by their creation date
typedef struct replay_info_type {
 char filename[POP_MAX_PATH];
 time_t creation_time;
 replay_header_type header;
} replay_info_type;

#define REPLAY_HEADER_ERROR_MESSAGE_MAX 512

#define fread_check(dst, size, elements, fp) do {  \
  size_t __count;     \
  __count = fread(dst, size, elements, fp); \
  if (__count != (elements)) {   \
   if (error_message != NULL) {  \
    snprintf_check(error_message, REPLAY_HEADER_ERROR_MESSAGE_MAX,\
            #dst " missing -- not a valid replay file!");\
   }     \
                return 0; /* incompatible file */  \
                }      \
 } while (0)

// The functions options_process_* below each process (read/write) a section of options variables (using SDL_RWops)
// This is I/O for the *binary* representation of the relevant options - this gets saved as part of a replay.

typedef int rw_process_func_type(SDL_RWops* rw, void* data, size_t data_size);
typedef void process_options_section_func_type(SDL_RWops* rw, rw_process_func_type process_func);

#define process(x) if (!process_func(rw, &(x), sizeof(x))) return
fixes_options_type fixes_options_replay;

// struct for keeping track of both the normal and the replay options (which we want to easily switch between)
// (separately for each 'section', so adding future options becomes easy without messing up the format!)
typedef struct replay_options_section_type {
 dword data_size;
 byte replay_data[POP_MAX_OPTIONS_SIZE]; // binary representation of the options that are active during the replay
 byte stored_data[POP_MAX_OPTIONS_SIZE]; // normal options are restored from this, after the replay is finished
 process_options_section_func_type* section_func;
} replay_options_section_type;

replay_options_section_type replay_options_sections[] = {
 {.section_func = options_process_features},
 {.section_func = options_process_enhancements},
 {.section_func = options_process_fixes},
 {.section_func = options_process_custom_general},
 {.section_func = options_process_custom_per_level},
};


#endif
