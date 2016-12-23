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
#include <dirent.h>
#include <time.h>
#include <sys/stat.h>

#ifdef USE_REPLAY

#define MAX_REPLAY_DURATION 345600 // 8 hours: 720 * 60 * 8 ticks
byte moves[MAX_REPLAY_DURATION] = {0}; // static memory for now because it is easier (should this be dynamic?)
byte replay_options[POP_MAX_OPTIONS_SIZE]; // Need to know what gameplay options are active during recording, for reproducability
size_t replay_options_size;
byte stored_options[POP_MAX_OPTIONS_SIZE];

char replay_levelset_name[POP_MAX_PATH];
char stored_levelset_name[POP_MAX_PATH];

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
const char replay_version[9] = "V1.17";
byte replay_file_open = 0;
int current_replay_number = 0;
int next_replay_number = 0;

byte* savestate_buffer = NULL;
size_t savestate_offset = 0;
size_t savestate_size = 0;
#define MAX_SAVESTATE_SIZE 4096

// These are defined in seg000.c:
typedef int process_func_type(void* data, size_t data_size);
extern int quick_process(process_func_type process_func);
extern const char quick_version[9];

// header information read from the first part of a replay file
typedef struct replay_header_type {
	char replay_version[9];
	char quick_version[9];
	byte uses_custom_levelset;
	char levelset_name[POP_MAX_PATH];
} replay_header_type;

// information needed to keep track of all listed replay files, and to sort them by their creation date
typedef struct replay_info_type {
    char filename[POP_MAX_PATH];
	time_t creation_time;
	replay_header_type header;
} replay_info_type;

void read_replay_header(replay_header_type* header, FILE* fp) {
	// read the version strings
	fread(header->replay_version, sizeof(header->replay_version), 1, fp);
	fread(header->quick_version, sizeof(header->quick_version), 1, fp);
	// some backwards compatibility with older replay format: if old format, don't read the levelset name
	if (strncmp(header->replay_version, "V1.16", 5) == 0) {
		return;
	}
	// read the levelset_name
	byte levelset_name_len_read = (byte) fgetc(fp);
	header->uses_custom_levelset = (levelset_name_len_read != 0);
	fread(header->levelset_name, sizeof(char), levelset_name_len_read, fp);
	header->levelset_name[levelset_name_len_read] = '\0';
}

size_t num_replay_files = 0; // number of listed replays
size_t max_replay_files = 128; // initially, may grow if there are > 128 replay files found
replay_info_type* replay_list = NULL;

// Compare function -- for qsort() in list_replay_files() below
// Compares creation dates of replays, so they can be loaded in reverse creation order (newest first)
static int compare_replay_creation_time(const void* a, const void* b)
{
	return (int) difftime( ((replay_info_type*)b)->creation_time, ((replay_info_type*)a)->creation_time );
}

void list_replay_files() {
	DIR* dp;
	struct dirent* ep;

    if (replay_list == NULL) {
        // need to allocate enough memory to store info about all replay files in the directory
        replay_list = malloc(max_replay_files * sizeof(replay_info_type)); // will realloc() later if > 256 files exist
    }

    num_replay_files = 0;
	dp = opendir(replays_folder);
	if (dp != NULL) {
		while ((ep = readdir(dp))) {
			char* ext = strrchr(ep->d_name, '.');
			if (ext != NULL && strcasecmp(ext, ".p1r") == 0) {
				++num_replay_files;
                if (num_replay_files > max_replay_files) {
                    // too many files, expand the memory available for replay_list
                    max_replay_files += 128;
                    replay_list = realloc(replay_list, max_replay_files * sizeof(replay_info_type));
                }
                replay_info_type* replay_info = &replay_list[num_replay_files-1]; // current replay file
				memset(replay_info, 0, sizeof(replay_info_type));
                // store the filename of the replay
				snprintf(replay_info->filename, POP_MAX_PATH, "%s/%s", replays_folder, ep->d_name);
				// get the creation time
				struct stat st;
				if (stat(replay_info->filename, &st) == 0) {
					replay_info->creation_time = st.st_ctime;
				}
				// read and store the levelset name associated with the replay
				FILE* fp = fopen(replay_info->filename, "rb");
				if (fp != NULL) {
					read_replay_header(&replay_info->header, fp);
					fclose(fp);
				}
			}
		}
	}

	if (num_replay_files > 1) {
		// sort listed replays by their creation date
		qsort(replay_list, num_replay_files, sizeof(replay_info_type), compare_replay_creation_time);
	}
};

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

void change_working_dir_to_sdlpop_root() {
	char* exe_path = g_argv[0];
	// strip away everything after the last slash or backslash in the path
	size_t len;
	for (len = strlen(exe_path); len >= 0; --len) {
		if (exe_path[len] == '\\' || exe_path[len] == '/') {
			break;
		}
	}
	char exe_dir[POP_MAX_PATH];
	strncpy(exe_dir, exe_path, len);
	exe_dir[len] = '\0';
	chdir(exe_dir);
};

// Called in pop_main(); check whether a replay file is being opened directly (double-clicked, dragged onto .exe, etc.)
void check_if_opening_replay_file() {
	if (!enable_replay) return;
	if (g_argc > 1) {
		char *filename = g_argv[1]; // file dragged on top of executable or double clicked
		char *e = strrchr(filename, '.');
		if (e != NULL && strcasecmp(e, ".P1R") == 0) { // valid replay filename passed as first arg
			if (open_replay_file(filename)) {
				change_working_dir_to_sdlpop_root();
				current_replay_number = -1; // don't cycle when pressing Tab
				// We should read the header in advance so we know the levelset name
				// then the game can immediately load the correct resources
				replay_header_type header = {{0}};
				read_replay_header(&header, replay_fp);
				if (header.uses_custom_levelset) {
					strncpy(replay_levelset_name, header.levelset_name, sizeof(replay_levelset_name)); // use the replays's levelset
				}
				rewind(replay_fp); // replay file is still open and will be read in load_replay() later
				need_start_replay = 1; // will later call start_replay(), from init_record_replay()
			};
		}
	}
}

void init_record_replay() {
    if (!enable_replay) return;
    if (check_param("record")) {
        start_recording();
    }
    else if (need_start_replay || check_param("replay")) {
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

void reload_resources() {
	// the replay's levelset might use different sounds, so we need to free and reload sounds
	// (except the music (OGG) files, which take too long to reload and cannot (yet) be easily replaced by a mod)
	reload_non_music_sounds();
	free_all_chtabs_from(id_chtab_0_sword);
	// chtabs 3 and higher will be freed/reloaded in load_lev_spr() (called by restore_room_after_quick_load())
	// However, chtabs 0-2 are usually not freed at all (they are loaded only once, in init_game_main())
	// So we should reload them manually (PRINCE.DAT and KID.DAT may still have been modified after all!)
	dat_type* dat = open_dat("PRINCE.DAT", 0);
	// PRINCE.DAT: sword
	chtab_addrs[id_chtab_0_sword] = load_sprites_from_file(700, 1<<2, 1);
	// PRINCE.DAT: flame, sword on floor, potion
	chtab_addrs[id_chtab_1_flameswordpotion] = load_sprites_from_file(150, 1<<3, 1);
	close_dat(dat);
	load_kid_sprite();  // reloads chtab 2
}

int restore_savestate_from_buffer() {
    int ok = 0;
    savestate_offset = 0;
    while (savestate_offset < savestate_size) {
        ok = quick_process(process_load_from_buffer);
    }
	reload_resources();
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
	// store the current options, so they can be restored later
	save_options_to_buffer(stored_options, sizeof(stored_options));

	// apply the options from the memory buffer (max. replay_options_size bytes will be read)
	load_options_from_buffer(replay_options, replay_options_size);

    if (!use_fixes_and_enhancements) disable_fixes_and_enhancements();
    enable_replay = 1; // just to be safe...

	memcpy(stored_levelset_name, levelset_name, sizeof(levelset_name));
	memcpy(levelset_name, replay_levelset_name, sizeof(levelset_name));
	use_custom_levelset = (levelset_name[0] == '\0') ? 0 : 1;

	reload_resources();
}

void restore_normal_options() {
	load_options_from_buffer(stored_options, sizeof(stored_options));

	start_level = 0; // may have been set to a different value by the replay

	memcpy(levelset_name, stored_levelset_name, sizeof(levelset_name));
	use_custom_levelset = (levelset_name[0] == '\0') ? 0 : 1;
}

void start_replay() {
	if (!enable_replay) return;
	need_start_replay = 0;
	list_replay_files();
	if (num_replay_files == 0) return;
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
		restore_normal_options();
        start_game();
		return;
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

const char* original_levels_name = "Prince of Persia";

void save_recorded_replay() {

	const char* name = (use_custom_levelset) ? levelset_name : original_levels_name;

	time_t now;
	time(&now);
	struct tm *tm_now = localtime(&now);

	char timestamp[32];
	strftime(timestamp, sizeof(timestamp), "%Y-%m-%d", tm_now);

	char filename[POP_MAX_PATH];
    word replay_number = 1;
    do {
		snprintf(filename, sizeof(filename), "%s/%s %s L%d - %d.p1r",
				 replays_folder, timestamp, name, current_level, replay_number);
		++replay_number;
	} while (access(filename, F_OK) != -1); // check if file already exists

	// create the "replays" folder if it does not exist already
#if defined WIN32 || _WIN32 || WIN64 || _WIN64
	mkdir (replays_folder);
#else
	mkdir (replays_folder, 0700);
#endif

    replay_fp = fopen(filename, "wb");
    if (replay_fp != NULL) {
        fwrite(replay_version, COUNT(replay_version), 1, replay_fp);
        fwrite(quick_version, COUNT(quick_version), 1, replay_fp);
		// include the name of the levelset (is empty for original levels)
		fputc(strnlen(levelset_name, UINT8_MAX), replay_fp);
		fputs(levelset_name, replay_fp);
        // embed a savestate into the replay
        fwrite(&savestate_size, sizeof(savestate_size), 1, replay_fp);
        fwrite(savestate_buffer, savestate_size, 1, replay_fp);

		// save the options
		byte temp_options[POP_MAX_OPTIONS_SIZE];
		size_t options_size = save_options_to_buffer(temp_options, sizeof(temp_options));
		fwrite(&options_size, sizeof(size_t), 1, replay_fp);
		fwrite(temp_options, options_size, 1, replay_fp);

        // save the rest of the replay data
        fwrite(&start_level, sizeof(start_level), 1, replay_fp);
        fwrite(&saved_random_seed, sizeof(saved_random_seed), 1, replay_fp);
        num_replay_ticks = curr_tick;
        fwrite(&num_replay_ticks, sizeof(num_replay_ticks), 1, replay_fp);
        fwrite(moves, num_replay_ticks, 1, replay_fp);
        fclose(replay_fp);
    }
}

byte open_next_replay_file() {
	if (next_replay_number > num_replay_files) {
		return 0; // reached the last replay file, return to title screen
	}
	current_replay_number = next_replay_number;
	++next_replay_number; // cycle
	open_replay_file(replay_list[current_replay_number].filename);
	if (replay_file_open) {
		return 1;
	}
    return 0;
}

void replay_cycle() {
    need_replay_cycle = 0;
    stop_sounds();
    if (current_replay_number == -1 /* opened .P1R file directly */ || !open_next_replay_file()) {
		// there is no replay to be cycled to after the current one --> restart the game
		replaying = 0;
		restore_normal_options();
		start_game();
		return;
	}
    load_replay();
    curr_tick = 0;
    apply_replay_options();
    restore_savestate_from_buffer();
}

void load_replay() {
    if (!replay_file_open) {
        next_replay_number = 0;
		open_next_replay_file();
    }
    if (savestate_buffer == NULL)
        savestate_buffer = malloc(MAX_SAVESTATE_SIZE);
    if (replay_fp != NULL && savestate_buffer != NULL) {
        replay_header_type header = {{0}};
		read_replay_header(&header, replay_fp);
		/* // Todo: decide if a warning or error is needed here (advantage: safe; disadvantage: annoying)
        if (strcmp(header.replay_version, replay_version) != 0) {
            printf("Warning: unexpected replay format!\n");
        }
        if (strcmp(header.quick_version, quick_version) != 0) {
            printf("Warning: unexpected savestate format!\n");
        }*/
		memcpy(replay_levelset_name, header.levelset_name, sizeof(header.levelset_name));

		// load the savestate
        fread(&savestate_size, sizeof(savestate_size), 1, replay_fp);
        fread(savestate_buffer, savestate_size, 1, replay_fp);

		// load the replay options
		if (strncmp(header.replay_version, "V1.16", 5) == 0) {
			// backward compatibility: only the first 38 (of 64) written bytes are equal to the newer format
			fread(replay_options, 64, 1, replay_fp);
			replay_options_size = 38;
		} else {
			fread(&replay_options_size, sizeof(replay_options_size), 1, replay_fp);
			fread(replay_options, replay_options_size, 1, replay_fp);
		}

		// load the rest of the replay data
        fread(&start_level, sizeof(start_level), 1, replay_fp);
        fread(&saved_random_seed, sizeof(saved_random_seed), 1, replay_fp);
        fread(&num_replay_ticks, sizeof(num_replay_ticks), 1, replay_fp);
        fread(moves, num_replay_ticks, 1, replay_fp);
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
			restore_normal_options();
            break;
        case SDL_SCANCODE_TAB:
            need_replay_cycle = 1;
			restore_normal_options();
            break;
    }
}

#endif // USE_REPLAY
