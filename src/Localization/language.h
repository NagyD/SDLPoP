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

#ifndef LANGUAGE_H
#define LANGUAGE_H

enum localization {
	EN = 0,
	FR,
	DE
};

enum localization set_language_param(const char *value);

void select_language_img(enum localization loc,
						 char* image_filename, size_t image_filename_size,
						 const char* filename_no_ext, int resource_id, const char* extension);

#endif
