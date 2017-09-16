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
#include <time.h>
#ifndef _MSC_VER // unistd.h does not exist in the Windows SDK.
#include <unistd.h>
#endif
#include <sys/stat.h>

// Directory listing using dirent.h is available using MinGW on Windows, but not using MSVC (need to use Win32 API).
// NOTE: If we are using MinGW, we'll opt to use the Win32 API as well: dirent.h would just wrap Win32 anyway!
#ifdef _WIN32
#define USE_WIN32_API_FOR_LISTING_REPLAY_FILES
#endif

#ifdef USE_WIN32_API_FOR_LISTING_REPLAY_FILES
#include <windows.h>
#include <wchar.h>
#else
#include <dirent.h>
#endif

#ifdef USE_REPLAY

const char replay_magic_number[3] = "P1R";
const word replay_format_class = 0;          // unique number associated with this SDLPoP implementation / fork
const char* implementation_name = "SDLPoP v" SDLPOP_VERSION;

#define REPLAY_FORMAT_CURR_VERSION       101 // current version number of the replay format
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

dword curr_tick = 0;

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

int read_replay_header(replay_header_type* header, FILE* fp, char* error_message) {
	// Explicitly go to the beginning, because the current filepos might be nonzero.
	fseek(fp, 0, SEEK_SET);
	// read the magic number
	char magic[3] = "";
	fread(magic, 3, 1, fp);
	if (strncmp(magic, replay_magic_number, 3) != 0) {
		if (error_message != NULL) {
			snprintf(error_message, REPLAY_HEADER_ERROR_MESSAGE_MAX, "not a valid replay file!");
		}
		return 0; // incompatible, magic number not correct!
	}
	// read the unique number associated with this SDLPoP implementation / fork (for normal SDLPoP: 0)
	word class;
	fread(&class, sizeof(class), 1, fp);
	// read the format version number
	byte version_number = (byte) fgetc(fp);
	// read the format deprecation number
	byte deprecation_number = (byte) fgetc(fp);

	// creation time (seconds since 1970) is embedded in the format, but not used in SDLPoP right now
	fseek(fp, sizeof(Sint64), SEEK_CUR);

	// read the levelset_name
	byte len_read = (byte) fgetc(fp);
	header->uses_custom_levelset = (len_read != 0);
	fread(header->levelset_name, sizeof(char), len_read, fp);
	header->levelset_name[len_read] = '\0';

	// read the implementation_name
	len_read = (byte) fgetc(fp);
	fread(header->implementation_name, sizeof(char), len_read, fp);
	header->implementation_name[len_read] = '\0';

	if (class != replay_format_class) {
		// incompatible, replay format is associated with a different implementation of SDLPoP
		if (error_message != NULL) {
			snprintf(error_message, REPLAY_HEADER_ERROR_MESSAGE_MAX,
			         "replay created with \"%s\"...\nIncompatible replay class identifier! (expected %d, found %d)",
			         header->implementation_name, replay_format_class, class);
		}
		return 0;
	}

	if (version_number < REPLAY_FORMAT_MIN_VERSION) {
		// incompatible, replay format is too old
		if (error_message != NULL) {
			snprintf(error_message, REPLAY_HEADER_ERROR_MESSAGE_MAX,
			         "replay created with \"%s\"...\nReplay format version too old! (minimum %d, found %d)",
			         header->implementation_name, REPLAY_FORMAT_MIN_VERSION, version_number);
		}
		return 0;
	}

	if (deprecation_number > REPLAY_FORMAT_DEPRECATION_NUMBER) {
		// incompatible, replay format is too new
		if (error_message != NULL) {
			snprintf(error_message, REPLAY_HEADER_ERROR_MESSAGE_MAX,
			         "replay created with \"%s\"...\nReplay deprecation number too new! (max %d, found %d)",
			         header->implementation_name, REPLAY_FORMAT_DEPRECATION_NUMBER, deprecation_number);
		}
		return 0;
	}

	if (is_validate_mode) {
		static byte is_replay_info_printed = 0;
		if (!is_replay_info_printed) {
			printf("\nReplay created with %s.\n", header->implementation_name);
			printf("Format: class identifier %d, version number %d, deprecation number %d.\n",
			       class, version_number, deprecation_number);
			if (header->levelset_name[0] == '\0') {
				printf("Levelset: original Prince of Persia.\n");
			} else {
				printf("Levelset: %s.\n", header->levelset_name);
			}
			putchar('\n');
			is_replay_info_printed = 1; // do this only once
		}
	}

	return 1;
}

int num_replay_files = 0; // number of listed replays
size_t max_replay_files = 128; // initially, may grow if there are > 128 replay files found
replay_info_type* replay_list = NULL;

// Compare function -- for qsort() in list_replay_files() below
// Compares creation dates of replays, so they can be loaded in reverse creation order (newest first)
static int compare_replay_creation_time(const void* a, const void* b)
{
	return (int) difftime( ((replay_info_type*)b)->creation_time, ((replay_info_type*)a)->creation_time );
}

// OS abstraction for listing directory contents (for list_replay_files() below)
// - Under GNU/Linux, etc (or if compiling with MinGW on Windows), we can use dirent.h
// - Under Windows, we'd like to directly call the Win32 API. (Note: MSVC does not include dirent.h)

#ifdef USE_WIN32_API_FOR_LISTING_REPLAY_FILES

// These macros are from the SDL2 source. (src/core/windows/SDL_windows.h)
// The pointers returned by these macros must be freed with SDL_free().
#define WIN_StringToUTF8(S) SDL_iconv_string("UTF-8", "UTF-16LE", (char *)(S), (SDL_wcslen(S)+1)*sizeof(WCHAR))
#define WIN_UTF8ToString(S) (WCHAR *)SDL_iconv_string("UTF-16LE", "UTF-8", (char *)(S), SDL_strlen(S)+1)

FILE* fopen_UTF8(const char* filename, const char* mode);
#define fopen fopen_UTF8
int chdir_UTF8(const char* path);
#define chdir chdir_UTF8

// This hack is needed because SDL uses UTF-8 everywhere (even in argv!), but fopen on Windows uses whatever code page is currently set.
FILE* fopen_UTF8(const char* filename_UTF8, const char* mode_UTF8) {
	WCHAR* filename_UTF16 = WIN_UTF8ToString(filename_UTF8);
	WCHAR* mode_UTF16 = WIN_UTF8ToString(mode_UTF8);
	FILE* result = _wfopen(filename_UTF16, mode_UTF16);
	SDL_free(mode_UTF16);
	SDL_free(filename_UTF16);
	return result;
}

int chdir_UTF8(const char* path_UTF8) {
	WCHAR* path_UTF16 = WIN_UTF8ToString(path_UTF8);
	int result = _wchdir(path_UTF16);
	SDL_free(path_UTF16);
	return result;
}

typedef struct directory_listing_data_type {
	char search_pattern[POP_MAX_PATH];
	WIN32_FIND_DATAW find_data;
	HANDLE search_handle;
	char* current_filename_UTF8;
} directory_listing_type;

static inline bool init_directory_listing_and_find_first_file(directory_listing_type *data) {
	data->current_filename_UTF8 = NULL;
	snprintf( data->search_pattern, POP_MAX_PATH, "%s\\*.p1r", replays_folder);
	WCHAR* search_pattern_UTF16 = WIN_UTF8ToString(data->search_pattern);
	data->search_handle = FindFirstFileW( search_pattern_UTF16, &data->find_data );
	SDL_free(search_pattern_UTF16);
	return (data->search_handle != INVALID_HANDLE_VALUE);
}

static inline char* get_current_filename_from_directory_listing(directory_listing_type* data) {
	SDL_free(data->current_filename_UTF8);
	data->current_filename_UTF8 = NULL;
	data->current_filename_UTF8 = WIN_StringToUTF8(data->find_data.cFileName);
	return data->current_filename_UTF8;
}

static inline bool find_next_file(directory_listing_type* data) {
	return (bool) FindNextFileW( data->search_handle, &data->find_data );
}

static inline void directory_listing_close(directory_listing_type *data) {
	FindClose(data->search_handle);
	SDL_free(data->current_filename_UTF8);
	data->current_filename_UTF8 = NULL;
}

#else // use dirent.h API for listing replay files

typedef struct directory_listing_data_type {
	DIR* dp;
	char* found_filename;

} directory_listing_type;

static inline bool init_directory_listing_and_find_first_file(directory_listing_type *data) {
	bool ok = false;
	data->dp = opendir(replays_folder);
	if (data->dp != NULL) {
		struct dirent* ep;
		while ((ep = readdir(data->dp))) {
			char *ext = strrchr(ep->d_name, '.');
			if (ext != NULL && strcasecmp(ext, ".p1r") == 0) {
				data->found_filename = ep->d_name;
				ok = true;
				break;
			}
		}
	}
	return ok;
}

static inline char* get_current_filename_from_directory_listing(directory_listing_type* data) {
	return data->found_filename;
}

static inline bool find_next_file(directory_listing_type* data) {
	bool ok = false;
	struct dirent* ep;
	while ((ep = readdir(data->dp))) {
		char *ext = strrchr(ep->d_name, '.');
		if (ext != NULL && strcasecmp(ext, ".p1r") == 0) {
			data->found_filename = ep->d_name;
			ok = true;
			break;
		}
	}
	return ok;
}

static inline void directory_listing_close(directory_listing_type *data) {
	closedir(data->dp);
}

#endif


void list_replay_files() {

	if (replay_list == NULL) {
		// need to allocate enough memory to store info about all replay files in the directory
		replay_list = malloc( max_replay_files * sizeof( replay_info_type ) ); // will realloc() later if > 256 files exist
	}

	num_replay_files = 0;

	directory_listing_type directory_listing = {0};
	if (!init_directory_listing_and_find_first_file(&directory_listing)) {
		return;
	}

	do {
		++num_replay_files;
		if (num_replay_files > max_replay_files) {
			// too many files, expand the memory available for replay_list
			max_replay_files += 128;
			replay_list = realloc( replay_list, max_replay_files * sizeof( replay_info_type ) );
		}
		replay_info_type* replay_info = &replay_list[num_replay_files - 1]; // current replay file
		memset( replay_info, 0, sizeof( replay_info_type ) );
		// store the filename of the replay
		snprintf( replay_info->filename, POP_MAX_PATH, "%s/%s", replays_folder,
					get_current_filename_from_directory_listing(&directory_listing) );

		// get the creation time
		struct stat st;
		if (stat( replay_info->filename, &st ) == 0) {
			replay_info->creation_time = st.st_ctime;
		}
		// read and store the levelset name associated with the replay
		FILE* fp = fopen( replay_info->filename, "rb" );
		int ok = 0;
		if (fp != NULL) {
			ok = read_replay_header( &replay_info->header, fp, NULL );
			fclose( fp );
		}
		if (!ok) --num_replay_files; // scrap the file if it is not compatible

	} while (find_next_file(&directory_listing));

	directory_listing_close(&directory_listing);

	if (num_replay_files > 1) {
		// sort listed replays by their creation date
		qsort( replay_list, (size_t) num_replay_files, sizeof( replay_info_type ), compare_replay_creation_time );
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
	int len;
	for (len = strlen(exe_path); len > 0; --len) {
		if (exe_path[len] == '\\' || exe_path[len] == '/') {
			break;
		}
	}
	if (len > 0) {
		char exe_dir[POP_MAX_PATH];
		strncpy(exe_dir, exe_path, len);
		exe_dir[len] = '\0';

		int result = chdir(exe_dir);
		if (result != 0) {
			perror("Can't change into SDLPoP directory");
		}
	}

};

// Called in pop_main(); check whether a replay file is being opened directly (double-clicked, dragged onto .exe, etc.)
void start_with_replay_file(const char *filename) {
	if (open_replay_file(filename)) {
		change_working_dir_to_sdlpop_root();
		current_replay_number = -1; // don't cycle when pressing Tab
		// We should read the header in advance so we know the levelset name
		// then the game can immediately load the correct resources
		replay_header_type header = {0};
		char header_error_message[REPLAY_HEADER_ERROR_MESSAGE_MAX];
		int ok = read_replay_header(&header, replay_fp, header_error_message);
		if (!ok) {
			char error_message[REPLAY_HEADER_ERROR_MESSAGE_MAX];
			snprintf(error_message, REPLAY_HEADER_ERROR_MESSAGE_MAX,
			         "Error opening replay file: %s\n",
			         header_error_message);
			fprintf(stderr, "%s", error_message);
			fclose(replay_fp);
			replay_fp = NULL;
			replay_file_open = 0;

			if (is_validate_mode) // Validating replays is cmd-line only, so, no sense continuing from here.
				exit(0);

			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SDLPoP", error_message, NULL);
			return;
		}
		if (header.uses_custom_levelset) {
			strncpy(replay_levelset_name, header.levelset_name, sizeof(replay_levelset_name)); // use the replays's levelset
		}
		rewind(replay_fp); // replay file is still open and will be read in load_replay() later
		need_start_replay = 1; // will later call start_replay(), from init_record_replay()
	}
}

int process_rw_write(SDL_RWops* rw, void* data, size_t data_size) {
	return SDL_RWwrite(rw, data, data_size, 1);
}

int process_rw_read(SDL_RWops* rw, void* data, size_t data_size) {
	return SDL_RWread(rw, data, data_size, 1);
	// if this returns 0, most likely the end of the stream has been reached
}

// The functions options_process_* below each process (read/write) a section of options variables (using SDL_RWops)
// This is I/O for the *binary* representation of the relevant options - this gets saved as part of a replay.

typedef int rw_process_func_type(SDL_RWops* rw, void* data, size_t data_size);
typedef void process_options_section_func_type(SDL_RWops* rw, rw_process_func_type process_func);

#define process(x) if (!process_func(rw, &(x), sizeof(x))) return

void options_process_features(SDL_RWops* rw, rw_process_func_type process_func) {
	process(enable_copyprot);
	process(enable_quicksave);
	process(enable_quicksave_penalty);
}

void options_process_enhancements(SDL_RWops* rw, rw_process_func_type process_func) {
	process(use_fixes_and_enhancements);
	process(enable_crouch_after_climbing);
	process(enable_freeze_time_during_end_music);
	process(enable_remember_guard_hp);
}

void options_process_fixes(SDL_RWops* rw, rw_process_func_type process_func) {
	process(fix_gate_sounds);
	process(fix_two_coll_bug);
	process(fix_infinite_down_bug);
	process(fix_gate_drawing_bug);
	process(fix_bigpillar_climb);
	process(fix_jump_distance_at_edge);
	process(fix_edge_distance_check_when_climbing);
	process(fix_painless_fall_on_guard);
	process(fix_wall_bump_triggers_tile_below);
	process(fix_stand_on_thin_air);
	process(fix_press_through_closed_gates);
	process(fix_grab_falling_speed);
	process(fix_skeleton_chomper_blood);
	process(fix_move_after_drink);
	process(fix_loose_left_of_potion);
	process(fix_guard_following_through_closed_gates);
	process(fix_safe_landing_on_spikes);
	process(fix_glide_through_wall);
	process(fix_drop_through_tapestry);
	process(fix_land_against_gate_or_tapestry);
	process(fix_unintended_sword_strike);
	process(fix_retreat_without_leaving_room);
	process(fix_running_jump_through_tapestry);
	process(fix_push_guard_into_wall);
	process(fix_jump_through_wall_above_gate);
	process(fix_chompers_not_starting);
	process(fix_feather_interrupted_by_leveldoor);
	process(fix_offscreen_guards_disappearing);
	process(fix_move_after_sheathe);
}

void options_process_custom_general(SDL_RWops* rw, rw_process_func_type process_func) {
	process(start_minutes_left);
	process(start_ticks_left);
	process(start_hitp);
	process(max_hitp_allowed);
	process(saving_allowed_first_level);
	process(saving_allowed_last_level);
	process(start_upside_down);
	process(start_in_blind_mode);
	process(copyprot_level);
	process(drawn_tile_top_level_edge);
	process(drawn_tile_left_level_edge);
	process(level_edge_hit_tile);
	process(allow_triggering_any_tile);
	process(enable_wda_in_palace);
	process(vga_palette);
	process(first_level);
	process(skip_title);
	process(shift_L_allowed_until_level);
	process(shift_L_reduced_minutes);
	process(shift_L_reduced_ticks);
}

void options_process_custom_per_level(SDL_RWops* rw, rw_process_func_type process_func) {
	process(tbl_level_type);
	process(tbl_level_color);
	process(tbl_guard_type);
	process(tbl_guard_hp);
	process(tbl_cutscenes_by_index);
}

#undef process

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

// output the current options to a memory buffer (e.g. to remember them before a replay is loaded)
size_t save_options_to_buffer(void* options_buffer, size_t max_size, process_options_section_func_type* process_section_func) {
	SDL_RWops* rw = SDL_RWFromMem(options_buffer, max_size);
	process_section_func(rw, process_rw_write);
	Sint64 section_size = SDL_RWtell(rw);
	if (section_size < 0) section_size = 0;
	SDL_RWclose(rw);
	return (size_t) section_size;
}

void apply_cutscene_pointers() {
	int i;
	for (i = 0; i < 16; ++i) {
		tbl_cutscenes[i] = tbl_cutscenes_lookup[tbl_cutscenes_by_index[i]];
	}
}

// restore the options from a memory buffer (e.g. reapply the original options after a replay is finished)
void load_options_from_buffer(void* options_buffer, size_t options_size, process_options_section_func_type* process_section_func) {
	SDL_RWops* rw = SDL_RWFromMem(options_buffer, options_size);
	process_section_func(rw, process_rw_read);
	apply_cutscene_pointers();
	SDL_RWclose(rw);
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

	replay_move_type curr_move = {{0}};
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
	recording = 0;
	if (save_recorded_replay()) {
		display_text_bottom("REPLAY SAVED");
	} else {
		display_text_bottom("REPLAY CANCELED");
	}
	text_time_total = 24;
	text_time_remaining = 24;
}

void apply_replay_options() {
	// store the current options, so they can be restored later
	for (int i = 0; i < COUNT(replay_options_sections); ++i) {
		save_options_to_buffer(replay_options_sections[i].stored_data, POP_MAX_OPTIONS_SIZE, replay_options_sections[i].section_func);
	}

	// apply the options from the memory buffer (max. replay_options_size bytes will be read)
	for (int i = 0; i < COUNT(replay_options_sections); ++i) {
		load_options_from_buffer(replay_options_sections[i].replay_data, replay_options_sections[i].data_size, replay_options_sections[i].section_func);
	}

	if (!use_fixes_and_enhancements) disable_fixes_and_enhancements();
	enable_replay = 1; // just to be safe...

	memcpy(stored_levelset_name, levelset_name, sizeof(levelset_name));
	memcpy(levelset_name, replay_levelset_name, sizeof(levelset_name));
	use_custom_levelset = (levelset_name[0] == '\0') ? 0 : 1;

	reload_resources();
}

void restore_normal_options() {
	// apply the stored options
	for (int i = 0; i < COUNT(replay_options_sections); ++i) {
		load_options_from_buffer(replay_options_sections[i].stored_data, POP_MAX_OPTIONS_SIZE, replay_options_sections[i].section_func);
	}

	start_level = -1; // may have been set to a different value by the replay

	memcpy(levelset_name, stored_levelset_name, sizeof(levelset_name));
	use_custom_levelset = (levelset_name[0] == '\0') ? 0 : 1;
}

static void print_remaining_time() {
	if (rem_min > 0) {
		printf("Remaining time: %d min, %d sec, %d ticks. ",
		       rem_min - 1, rem_tick / 12, rem_tick % 12);
	} else {
		printf("Elapsed time:   %d min, %d sec, %d ticks. ",
		       -(rem_min + 1), (719 - rem_tick) / 12, (719 - rem_tick) % 12);
	}
	printf("(rem_min=%d, rem_tick=%d)\n", rem_min, rem_tick);
}

void start_replay() {
	if (!enable_replay) return;
	need_start_replay = 0;
	if (!is_validate_mode) {
		list_replay_files();
		if (num_replay_files == 0) return;
	}
	if (!load_replay()) return;
	apply_replay_options();
	replaying = 1;
	curr_tick = 0;
}

void end_replay() {
	if (!is_validate_mode) {
		replaying = 0;
		skipping_replay = 0;
		restore_normal_options();
		start_game();
	} else {
		printf("\nReplay ended in level %d, room %d.\n", current_level, drawn_room);

		if (Kid.alive < 0)
			printf("Kid is alive.\n");
		else {
			if (text_time_total == 288 && text_time_remaining <= 1) {
				printf("Kid is dead. (Did not press button to continue.)\n");
			} else {
				printf("Kid is dead.\n");
			}
		}

		print_remaining_time();

		int minute_ticks = curr_tick % 720;
		printf("Play duration:  %d min, %d sec, %d ticks. (curr_tick=%d)\n\n",
		       curr_tick / 720, minute_ticks / 12, minute_ticks % 12, curr_tick);

		if (num_replay_ticks != curr_tick) {
			printf("WARNING: Play duration does not match replay length. (%d ticks)\n", num_replay_ticks);
		} else {
			printf("Play duration matches replay length. (%d ticks)\n", num_replay_ticks);
		}
		exit(0);
	}
}

void do_replay_move() {
	if (curr_tick == 0) {
		random_seed = saved_random_seed;
		seed_was_init = 1;

		if (is_validate_mode) {
			printf("Replay started in level %d, room %d.\n", current_level, drawn_room);
			print_remaining_time();
			skipping_replay = 1;
			replay_seek_target = replay_seek_2_end;
		}
	}
	if (curr_tick == num_replay_ticks) { // replay is finished
		end_replay();
		return;
	}
	if (current_level == next_level) {
		replay_move_type curr_move;
		curr_move.bits = moves[curr_tick];

		control_x = curr_move.x;
		control_y = curr_move.y;

		// Ignore shift if the kid is dead: restart moves are hard-coded as a 'special move'.
		if (rem_min != 0 && Kid.alive > 6)
			control_shift = 0;
		else
			control_shift = (curr_move.shift) ? -1 : 0;

		if (curr_move.special == MOVE_RESTART_LEVEL) { // restart level
			stop_sounds();
			is_restart_level = 1;
		} else if (curr_move.special == MOVE_EFFECT_END) {
			stop_sounds();
			need_level1_music = 0;
			is_feather_fall = 0;
		}

//    if (curr_tick > 5 ) printf("rem_tick: %d\t curr_tick: %d\tlast 5 moves: %d, %d, %d, %d, %d\n", rem_tick, curr_tick,
//                               moves[curr_tick-4], moves[curr_tick-3], moves[curr_tick-2], moves[curr_tick-1], moves[curr_tick]);
		++curr_tick;
	}
}

int save_recorded_replay() {
	// prompt for replay filename
	rect_type rect;
	short bgcolor = color_8_darkgray;
	short color = color_15_brightwhite;
	current_target_surface = onscreen_surface_;
	screen_updates_suspended = 1;
	method_1_blit_rect(offscreen_surface, onscreen_surface_, &copyprot_dialog->peel_rect, &copyprot_dialog->peel_rect, 0);
	draw_dialog_frame(copyprot_dialog);
	shrink2_rect(&rect, &copyprot_dialog->text_rect, 2, 1);
	show_text_with_color(&rect, 0, 0, "Save replay\nenter the filename...\n\n", color_15_brightwhite);
	clear_kbd_buf();

	rect_type text_rect;
	rect_type input_rect = {104,   64,  118,  256};
	offset4_rect_add(&text_rect, &input_rect, -2, 0, 2, 0);
	//peel_type* peel = read_peel_from_screen(&input_rect);
	draw_rect(&text_rect, bgcolor);
	current_target_surface = onscreen_surface_;
	screen_updates_suspended = 0;
	need_full_redraw = 1; // lazy: instead of neatly restoring the dialog peel, just redraw the whole screen

	char input_filename[POP_MAX_PATH] = "";
	int input_length;
	do {
		input_length = input_str(&input_rect, input_filename, 64, "", 0, 0, color, bgcolor);
	} while (input_length == 0); // filename must be at least 1 character

	if (input_length < 0) {
		return 0;  // Escape was pressed -> discard the replay
	}

	char full_filename[POP_MAX_PATH] = "";
	snprintf(full_filename, sizeof(full_filename), "%s/%s.p1r", replays_folder, input_filename);

	// create the "replays" folder if it does not exist already
#if defined WIN32 || _WIN32 || WIN64 || _WIN64
	mkdir (replays_folder);
#else
	mkdir (replays_folder, 0700);
#endif

	// NOTE: We currently overwrite the replay file if it exists already. Maybe warn / ask for confirmation??

	replay_fp = fopen(full_filename, "wb");
	if (replay_fp != NULL) {
		fwrite(replay_magic_number, COUNT(replay_magic_number), 1, replay_fp); // magic number "P1R"
		fwrite(&replay_format_class, sizeof(replay_format_class), 1, replay_fp);
		putc(REPLAY_FORMAT_CURR_VERSION, replay_fp);
		putc(REPLAY_FORMAT_DEPRECATION_NUMBER, replay_fp);
		Sint64 seconds = time(NULL);
		fwrite(&seconds, sizeof(seconds), 1, replay_fp);
		// levelset_name
		putc(strnlen(levelset_name, UINT8_MAX), replay_fp); // length of the levelset name (is zero for original levels)
		fputs(levelset_name, replay_fp);
		// implementation name
		putc(strnlen(implementation_name, UINT8_MAX), replay_fp);
		fputs(implementation_name, replay_fp);
		// embed a savestate into the replay
		fwrite(&savestate_size, sizeof(savestate_size), 1, replay_fp);
		fwrite(savestate_buffer, savestate_size, 1, replay_fp);

		// save the options, organized per section
		byte temp_options[POP_MAX_OPTIONS_SIZE];
		for (int i = 0; i < COUNT(replay_options_sections); ++i) {
			dword section_size = save_options_to_buffer(temp_options, sizeof(temp_options), replay_options_sections[i].section_func);
			fwrite(&section_size, sizeof(section_size), 1, replay_fp);
			fwrite(temp_options, section_size, 1, replay_fp);
		}

		// save the rest of the replay data
		fwrite(&start_level, sizeof(start_level), 1, replay_fp);
		fwrite(&saved_random_seed, sizeof(saved_random_seed), 1, replay_fp);
		num_replay_ticks = curr_tick;
		fwrite(&num_replay_ticks, sizeof(num_replay_ticks), 1, replay_fp);
		fwrite(moves, num_replay_ticks, 1, replay_fp);
		fclose(replay_fp);
		replay_fp = NULL;
	}
	return 1;
}

byte open_next_replay_file() {
	if (next_replay_number > num_replay_files-1) {
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
	skipping_replay = 0;
	stop_sounds();
	if (current_replay_number == -1 /* opened .P1R file directly, so cycling is disabled */ ||
		!open_next_replay_file() ||
		!load_replay()
	) {
		// there is no replay to be cycled to after the current one --> restart the game
		replaying = 0;
		restore_normal_options();
		start_game();
		return;
	}
	curr_tick = 0;
	apply_replay_options();
	restore_savestate_from_buffer();
}

int load_replay() {
	if (!replay_file_open) {
		next_replay_number = 0;
		if (!open_next_replay_file()) {
			return 0;
		}
	}
	if (savestate_buffer == NULL)
		savestate_buffer = malloc(MAX_SAVESTATE_SIZE);
	if (replay_fp != NULL && savestate_buffer != NULL) {
		replay_header_type header = {0};
		char error_message[REPLAY_HEADER_ERROR_MESSAGE_MAX];
		int ok = read_replay_header(&header, replay_fp, error_message);
		if (!ok) {
			printf("Error loading replay: %s!\n", error_message);
			fclose(replay_fp);
			replay_fp = NULL;
			replay_file_open = 0;
			return 0;
		}

		memcpy(replay_levelset_name, header.levelset_name, sizeof(header.levelset_name));

		// load the savestate
		fread(&savestate_size, sizeof(savestate_size), 1, replay_fp);
		fread(savestate_buffer, savestate_size, 1, replay_fp);

		// load the replay options, organized per section
		for (int i = 0; i < COUNT(replay_options_sections); ++i) {
			dword section_size = 0;
			fread(&section_size, sizeof(section_size), 1, replay_fp);
			fread(replay_options_sections[i].replay_data, section_size, 1, replay_fp);
			replay_options_sections[i].data_size = section_size;
		}

		// load the rest of the replay data
		fread(&start_level, sizeof(start_level), 1, replay_fp);
		fread(&saved_random_seed, sizeof(saved_random_seed), 1, replay_fp);
		fread(&num_replay_ticks, sizeof(num_replay_ticks), 1, replay_fp);
		fread(moves, num_replay_ticks, 1, replay_fp);
		fclose(replay_fp);
		replay_fp = NULL;
		replay_file_open = 0;
		return 1; // success
	}
	return 0;
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
		case 0:                                 // 'no key pressed'
			break;
		default:
			// cannot manually do most stuff during a replay, so cancel the pressed key...
			*key_ptr = 1; // don't set to zero (we would be unable to unpause a replay because all keys are ignored)
			              // (1 is not in use as a scancode, see https://wiki.libsdl.org/SDLScancodeLookup)
			break;
		// ...but these are allowable actions:
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
		case SDL_SCANCODE_F:                    // skip forward to next room
			skipping_replay = 1;
			replay_seek_target = replay_seek_0_next_room;
			break;
		case SDL_SCANCODE_F | WITH_SHIFT:       // skip forward to start of next level
			skipping_replay = 1;
			replay_seek_target = replay_seek_1_next_level;
			break;
	}
}

#endif // USE_REPLAY
