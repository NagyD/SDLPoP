#include "common.h"

#ifdef USE_LIGHTING

image_type* lighting_mask = NULL;
image_type* screen_overlay = NULL;
Uint32 bgcolor;

const char mask_filename[] = "data/light.png";
const Uint8 ambient_level = 128;

// Called once at startup.
void init_lighting() {
	lighting_mask = IMG_Load(mask_filename);
	if (lighting_mask == NULL) {
		sdlperror("IMG_Load (lighting_mask)");
		return;
	}

	screen_overlay = SDL_CreateRGBSurface(0, 320, 192, 32, 0xFF << 0, 0xFF << 8, 0xFF << 16, 0xFF << 24);
	if (screen_overlay == NULL) {
		sdlperror("SDL_CreateRGBSurface (screen_overlay)");
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
