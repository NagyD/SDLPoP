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

#include <stdbool.h>
#include <string.h>

#include "non_ascii_code.h"

bool is_letter(const char* letter, const char* text, int* nb_bytes) {
	*nb_bytes = (int) strlen(letter);
	return strncmp(letter, text, *nb_bytes) == 0;
}

byte get_non_ascii_code(const char* text, int* nb_bytes) {
	if ((byte) *text <= 127) {
		*nb_bytes = 1;
		return (byte) *text;
	}

	if (is_letter("à", text, nb_bytes)) return 132;
	if (is_letter("é", text, nb_bytes)) return 133;
	if (is_letter("è", text, nb_bytes)) return 134;
	if (is_letter("ü", text, nb_bytes)) return 135;

	*nb_bytes = 1;
	return 32;	// Default to space
}
