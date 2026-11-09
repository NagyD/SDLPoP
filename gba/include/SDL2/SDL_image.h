/* SDL_image stub: SDLPoP only uses IMG_Load for icon.png — we drop that. */
#ifndef SDL_IMAGE_GBA_SHIM_H
#define SDL_IMAGE_GBA_SHIM_H
#include "SDL.h"
#ifdef __cplusplus
extern "C" {
#endif
static inline SDL_Surface* IMG_Load(const char* file) { (void)file; return NULL; }
static inline SDL_Surface* IMG_Load_RW(SDL_RWops* src, int freesrc) {
    (void)src; (void)freesrc; return NULL;
}
static inline const char* IMG_GetError(void) { return ""; }
#ifdef __cplusplus
}
#endif
#endif
