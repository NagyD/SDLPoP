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

#ifdef USE_SCREENSHOT

// TODO: Use incrementing numbers and a separate folder, like DOSBox? Or allow custom filenames.
const char screenshot_filename[] = "screenshot.png";

// Save a screenshot.
void save_screenshot() {
	IMG_SavePNG(onscreen_surface_, screenshot_filename);
	printf("Saved screenshot to \"%s\".\n", screenshot_filename);
}

// Switch to the given room and draw it.
void switch_to_room(int room) {
	drawn_room = room;
	load_room_links();
	
	// for guards
	Guard.direction = dir_56_none;
	guardhp_curr = 0; // otherwise guard HPs stay on screen
	draw_guard_hp(0, 10); // otherwise guard HPs still stay on screen if some guards have extra HP
	enter_guard(); // otherwise the guard won't show up
	check_shadow(); // otherwise the shadow won't appear on level 6
	
	// for potion bubbles
	anim_tile_modif();
	process_trobs();
	
	redraw_screen(1);
}

// Save a "screenshot" of the whole level.
void save_level_screenshot() {
	// TODO: Disable in the intro or if a cutscene is active?
	// Restrict this to cheat mode. After all, it's like using H/J/U/N or opening the level in an editor.
	if (!cheats_enabled) return;

	// First, figure out where to put each room.
	// TODO: Check for broken room links?

	#define NUMBER_OF_ROOMS 24
	int xpos[NUMBER_OF_ROOMS+1] = {0};
	int ypos[NUMBER_OF_ROOMS+1] = {0};
	bool processed[NUMBER_OF_ROOMS+1] = {false};
	xpos[drawn_room] = 0;
	ypos[drawn_room] = 0;
	int queue[NUMBER_OF_ROOMS] = {drawn_room}; // We start mapping from the current room.
	int queue_start = 0;
	int queue_end = 1;
	
	const int dx[4] = {-1, +1,  0,  0};
	const int dy[4] = { 0,  0, -1, +1};
	
	while (queue_start < queue_end) {
		int room = queue[queue_start++];
		byte* roomlinks = (byte*)(&level.roomlinks[room-1]);
		for (int direction = 0; direction < 4; direction++) {
			int other_room = roomlinks[direction];
			if (other_room >= 1 && other_room <= NUMBER_OF_ROOMS && !processed[other_room]) {
				int other_x = xpos[room] + dx[direction];
				int other_y = ypos[room] + dy[direction];
				xpos[other_room] = other_x;
				ypos[other_room] = other_y;
				processed[other_room] = true;
				queue[queue_end++] = other_room;
			}
		}
	}
	
	int min_x=0, max_x=0, min_y=0, max_y=0;
	for (int room=1;room<=NUMBER_OF_ROOMS;room++) {
		if (xpos[room] < min_x) min_x = xpos[room];
		if (xpos[room] > max_x) max_x = xpos[room];
		if (ypos[room] < min_y) min_y = ypos[room];
		if (ypos[room] > max_y) max_y = ypos[room];
	}
	
	int map_width = max_x-min_x+1;
	int map_height = max_y-min_y+1;

	#define MAX_MAP_SIZE NUMBER_OF_ROOMS
	int map[MAX_MAP_SIZE][MAX_MAP_SIZE] = {{0}};
	for (int room=1;room<=NUMBER_OF_ROOMS;room++) {
		if (processed[room]) {
			int y = ypos[room] - min_y;
			int x = xpos[room] - min_x;
			if (x>=0 && y>=0 && x<MAX_MAP_SIZE && y<MAX_MAP_SIZE) map[y][x] = room;
		}
	}
	
	// Debug printout of arrangement.
	printf("LEVEL %d\n", current_level);
	for (int y=0;y<map_height;y++) {
		for (int x=0;x<map_width;x++) {
			int room = map[y][x];
			if (room) {
				printf(" %2d", room);
			} else {
				printf("   ");
			}
		}
		printf("\n");
	}
	printf("\n");
	
	// Now we have the arrangement, let's make the picture!
	
	int image_width = map_width*320;
	int image_height = map_height*189+3+8;
	
	SDL_Surface* map_surface = SDL_CreateRGBSurface(0, image_width, image_height, 32, 0xFF, 0xFF<<8, 0xFF<<16, 0xFF<<24);
	if (map_surface == NULL) {
		sdlperror("SDL_CreateRGBSurface (map_surface)");
		//exit(1);
		return;
	}

	// TODO: Background color for places where there is no room?
	
	screen_updates_suspended = true;
	int old_room = drawn_room;
	for (int y=0;y<map_height;y++) {
		for (int x=0;x<map_width;x++) {
			int room = map[y][x];
			if (room) {
				SDL_Rect dest_rect;
				dest_rect.x = x*320;
				dest_rect.y = y*189;
				switch_to_room(room);
				
				// TODO: Show non-visible things, like: room numbers, loose floors, guard HPs, potion types, door links...
				// (this will make the function even more like a cheat)
				// TODO: Hide the status bar, or maybe show some custom text on it?
				
				SDL_BlitSurface(onscreen_surface_, NULL, map_surface, &dest_rect);
			}
		}
	}
	switch_to_room(old_room);
	screen_updates_suspended = false;

	IMG_SavePNG(map_surface, screenshot_filename);
	printf("Saved level screenshot to \"%s\".\n", screenshot_filename);
	
	SDL_FreeSurface(map_surface);
}

bool want_auto = false;
bool want_auto_whole_level;

void init_screenshot() {
	// Command-line options to automatically save a screenshot at startup.
	const char* screenshot_param = check_param("--screenshot");
	if (screenshot_param != NULL) {
		// We require megahit+levelnumber.
		// TODO: Allow a parameter like --screenshot=levelnumber. (That would allow level 0 and 15 to be screenshotted.)
		if (start_level <= 0) {
			printf("You must supply a level number if you want to make an automatic screenshot!\n");
			exit(1);
		} else {
			want_auto = true;
			want_auto_whole_level = (check_param("--screenshot-level") != NULL);
		}
	}
}

// TODO: Don't open a window if the user wants an auto screenshot.

// To skip cutscenes, etc.
bool want_auto_screenshot() {
	return want_auto;
}

// Called when the level is drawn for the first time.
void auto_screenshot() {
	if (!want_auto) return;
	
	if (want_auto_whole_level) {
		save_level_screenshot();
	} else {
		save_screenshot();
	}
	
	quit(1);
}

#endif

