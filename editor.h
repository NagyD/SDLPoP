
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

#ifndef EDITOR_H
#define EDITOR_H

typedef short tile_global_location_type;

#define R(t) (((t)/30)+1)
#define R_(t) (((t)/30))
#define T(r,t) (((r)-1)*30+(t))
#define P(t) ((t)%30)



#define NO_TILE ((Uint16)(-1))

void editor__process_key(int key,const char** answer_text, word* need_show_text);
void editor__handle_mouse_button(SDL_MouseButtonEvent e,int shift, int ctrl, int alt, int m);
void editor__loading_dat();

void get_doorlink(Uint16 value, tile_global_location_type* tp, short* next);
void set_doorlink(Uint16* value, tile_global_location_type tp, short next);
#endif
