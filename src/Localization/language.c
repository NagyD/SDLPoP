/*
SDLPoP, a port/conversion of the DOS game Prince of Persia.
Copyright (C) 2013-2023  Dávid Nagy

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

#include <string.h>
#include <stdio.h>

#include "../proto.h"
#include "language.h"

enum localization set_language_param(const char *value) {
	if	    (strcmp(value, "EN") == 0) return EN;
	else if (strcmp(value, "FR") == 0) return FR;
	else if (strcmp(value, "DE") == 0) return DE;
	else							   return EN;
}

void select_language_img(enum localization loc,
						 char* image_filename, size_t image_filename_size,
						 const char* filename_no_ext, int resource_id, const char* extension) {
	char lang_append[10];

	switch (loc) {
	case EN:
		strncpy(lang_append, "", sizeof(lang_append)); break;
	case FR:
		strncpy(lang_append, "_FR", sizeof(lang_append)); break;
	case DE:
		strncpy(lang_append, "_DE", sizeof(lang_append)); break;
	default:
		strncpy(lang_append, "", sizeof(lang_append)); break;
	}

	// First check if there is a file specific to the selected language
	snprintf_check(image_filename,image_filename_size,"data/%s/res%d%s.%s",filename_no_ext, resource_id, lang_append, extension);

	// try to open the file to check if it exists
	FILE* fp = fopen(locate_file(image_filename), "rb");
	if (fp != NULL) {
		fclose(fp);
	}
	else {
		// Specific file does not exist - fallback to regular file
		snprintf_check(image_filename,image_filename_size,"data/%s/res%d.%s",filename_no_ext, resource_id, extension);
	}
	return;
}
