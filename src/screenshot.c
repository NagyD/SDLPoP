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

// TODO: Use incrementing numbers, like DOSBox? Or allow custom filenames.
const char screenshot_filename[] = "screenshot.png";

void save_screenshot() {
	IMG_SavePNG(onscreen_surface_, screenshot_filename);
	printf("Saved screenshot to \"%s\".", screenshot_filename);
}

// TODO: Make it possible to "screenshot" the whole level.

bool want_auto = false;

void init_screenshot() {
	// Command-line options to automatically save a screenshot at startup.
	const char* screenshot_param = check_param("--screenshot");
	if (screenshot_param != NULL) {
		// We require megahit+levelnumber.
		if (start_level <= 0) {
			printf("You must supply a level number if you want to make an automatic screenshot!\n");
			exit(1);
		} else {
			want_auto = true;
		}
	}
}

// TODO: Don't open a window if the user wants an auto screenshot.
bool want_auto_screenshot() {
	return want_auto;
}

void auto_screenshot() {
	if (!want_auto) return;
	
	save_screenshot();
	
	quit(1);
}

#endif

