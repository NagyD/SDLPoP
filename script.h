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

#ifndef SCRIPT_H

int init_script();

void script__write_savelist(FILE* stream);
void script__read_savelist(FILE* stream);

void script__on_load_room(int room);
void script__on_start_game();
void script__on_load_level(int level_number);
void script__on_end_level(int level_number, word* next_level_number);
void script__on_drink_potion(int potion_id);
void script__custom_potion_anim(int potion_id, word* color, word* pot_size);
void script__custom_timers();

void script__apply_set_level_start_sequence();



#endif //SCRIPT_H
