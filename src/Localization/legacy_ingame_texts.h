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

#ifndef INGAME_TEXTS_H
#define INGAME_TEXTS_H

// This file contains text from the orginal DOS game

#include "../types.h"
#include "language.h"

void str_one_second_left(enum localization loc, char* string, size_t string_size);
void str_seconds_left(enum localization loc, char* string, size_t string_size, word rem_sec);
void str_minutes_left(enum localization loc, char* string, size_t string_size, word rem_min);

void str_time_expired(enum localization loc, char* string, size_t string_size);

void str_level(enum localization loc, char* string, size_t string_size, byte disp_level);

void str_pressbutton(enum localization loc, char* string, size_t string_size);
void str_pause(enum localization loc, char* string, size_t string_size);
void str_save(enum localization loc, char* string, size_t string_size);
void str_unable_save(enum localization loc, char* string, size_t string_size);

void str_copy_protection_bottom(enum localization loc, char* string, size_t string_size,
								word copy_word, word copy_line, word copy_page);
void str_copy_protection_dialog(enum localization loc, char* string, size_t string_size,
								word copy_word, word copy_line, word copy_page);

void str_dialog(enum localization loc, char* string, size_t string_size, const char* text);

void str_loading(enum localization loc, char* string, size_t string_size);

#endif
