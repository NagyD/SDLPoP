/*
SDLPoP, a port/conversion of the DOS game Prince of Persia.
Copyright (C) 2013-2018  Dávid Nagy

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

#ifdef USE_LIGHTING

image_type* lighting_mask = NULL;
image_type* screen_overlay = NULL;
Uint32 bgcolor;

const char mask_filename[] = "data/light.png";
const Uint8 ambient_level = 128;

// Called once at startup.
void init_lighting() {
	if (!enable_lighting) return;

	lighting_mask = IMG_Load(mask_filename);
	if (lighting_mask == NULL) {
		sdlperror("IMG_Load (lighting_mask)");
		enable_lighting = 0;
		return;
	}

	screen_overlay = SDL_CreateRGBSurface(0, 320, 192, 32, 0xFF << 0, 0xFF << 8, 0xFF << 16, 0xFF << 24);
	if (screen_overlay == NULL) {
		sdlperror("SDL_CreateRGBSurface (screen_overlay)");
		enable_lighting = 0;
		return;
	}

	// "color modulate", i.e. multiply.
	int result = SDL_SetSurfaceBlendMode(screen_overlay, SDL_BLENDMODE_MOD);
	if (result != 0) {
		sdlperror("SDL_SetSurfaceBlendMode (screen_overlay)");
	}

	result = SDL_SetSurfaceBlendMode(lighting_mask, SDL_BLENDMODE_ADD);
	if (result != 0) {
		sdlperror("SDL_SetSurfaceBlendMode (lighting_mask)");
	}

	// ambient lighting
	bgcolor = SDL_MapRGBA(screen_overlay->format, ambient_level, ambient_level, ambient_level, SDL_ALPHA_OPAQUE);
}

// Recreate the lighting overlay based on the torches in the current room.
// Called when the current room changes.
void redraw_lighting() {
	if (!enable_lighting) return;
	if (lighting_mask == NULL) return;
	if (curr_room_tiles == NULL) return;
	if (is_cutscene) return;

	int result = SDL_FillRect(screen_overlay, NULL, bgcolor);
	if (result != 0) {
		sdlperror("SDL_FillRect (screen_overlay)");
	}

	// TODO: Also process nearby offscreen torches?
	for (int tile_pos = 0; tile_pos < 30; tile_pos++) {
		int tile_type = curr_room_tiles[tile_pos] & 0x1F;
		if (tile_type == tiles_19_torch || tile_type == tiles_30_torch_with_debris) {
			// Center of the flame.
			int x = (tile_pos%10)*32+48;
			int y = (tile_pos/10)*63+22;

			// Align the center of lighting mask to the center of the flame.
			SDL_Rect dest_rect;
			dest_rect.x = x - lighting_mask->w / 2;
			dest_rect.y = y - lighting_mask->h / 2;
			dest_rect.w = lighting_mask->w;
			dest_rect.h = lighting_mask->h;

			int result = SDL_BlitSurface(lighting_mask, NULL, screen_overlay, &dest_rect);
			if (result != 0) {
				sdlperror("SDL_BlitSurface (lighting_mask)");
			}
		}
	}
	if (upside_down) {
		flip_screen(screen_overlay);
	}
}

// Copy a part of the lighting overlay onto the screen.
// Called when the screen is updated.
void update_lighting(const rect_type far *target_rect_ptr) {
	if (!enable_lighting) return;
	if (lighting_mask == NULL) return;
	if (curr_room_tiles == NULL) return;
	if (is_cutscene) return;

	SDL_Rect sdlrect;
	rect_to_sdlrect(target_rect_ptr, &sdlrect);
	int result = SDL_BlitSurface(screen_overlay, &sdlrect, onscreen_surface_, &sdlrect);
	if (result != 0) {
		sdlperror("SDL_BlitSurface (screen_overlay)");
	}
}

#endif // USE_LIGHTING
