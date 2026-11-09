/*
SDLPoP, a port/conversion of the DOS game Prince of Persia.
Copyright (C) 2013-2024  Dávid Nagy

SDL2 -> SDL3 compatibility shim.

Included by types.h ONLY when USE_SDL3 is defined, AFTER the SDL3 headers (which
are themselves pulled in with SDL_ENABLE_OLD_NAMES defined). SDL3's own
SDL_oldnames.h then maps the vast majority of SDL2 names 1:1 (event types,
gamepad enums, SDL_FreeSurface, SDL_RWops, etc.). This shim only covers what that
layer cannot:

  - signature changes (SDL_CreateWindow/Renderer arg counts, SDL_RWread/write,
    SDL_RenderSetLogicalSize, SDL_BlitScaled's new scale-mode arg, SDL_MapRGB);
  - return-value flips: SDL3 returns `bool` (true = success) where SDL2 returned
    `int` (0 = success), and SDLPoP checks `!= 0` for errors, so those calls are
    wrapped to return 0/-1;
  - APIs removed entirely (SDL_CreateRGBSurface, the callback-based audio API,
    SDL_AudioCVT, the SDL_version struct);
  - a few names oldnames maps to deprecated "remove this line" sentinels.

Where we override a name that SDL3/oldnames also defines as a macro, we #undef
first to avoid -Wmacro-redefined. Inline wrappers are defined BEFORE the SDL2-name
macros so their bodies call the real SDL3 functions, not themselves.

The timer-callback signature change (SDL_AddTimer) is handled by a small #ifdef on
the two callback definitions in seg000.c / seg009.c, not here.
*/

#ifndef SDL2_TO_SDL3_H
#define SDL2_TO_SDL3_H

#include <stdbool.h>

/* ---- constants oldnames/SDL3 don't provide (or map to sentinels) ---- */
#ifndef SDL_ENABLE
#define SDL_ENABLE  1
#endif
#ifndef SDL_DISABLE
#define SDL_DISABLE 0
#endif

/* SDL_INIT_TIMER / NOPARACHUTE were removed (timer is always available). */
#undef  SDL_INIT_TIMER
#define SDL_INIT_TIMER       0u
#undef  SDL_INIT_NOPARACHUTE
#define SDL_INIT_NOPARACHUTE 0u

/* oldnames maps this to a deprecated sentinel; we still pass it to a flags arg. */
#undef  SDL_WINDOW_FULLSCREEN_DESKTOP
#define SDL_WINDOW_FULLSCREEN_DESKTOP SDL_WINDOW_FULLSCREEN

/* Renderer creation flags are gone in SDL3; keep dummy bits that
   compat_SDL_CreateRenderer interprets (SOFTWARE) or ignores. */
#undef  SDL_RENDERER_SOFTWARE
#define SDL_RENDERER_SOFTWARE      0x1u
#undef  SDL_RENDERER_ACCELERATED
#define SDL_RENDERER_ACCELERATED   0x2u
#undef  SDL_RENDERER_TARGETTEXTURE
#define SDL_RENDERER_TARGETTEXTURE 0x4u

/* These hints were removed (oldnames -> deprecated sentinels). Keep them as
   harmless strings so SDL_SetHint() calls still compile; SDL3 ignores unknown
   hints. Fuzzy/blurry scaling is reproduced via SDL_SetTextureScaleMode in
   seg009.c instead. */
#undef  SDL_HINT_RENDER_SCALE_QUALITY
#define SDL_HINT_RENDER_SCALE_QUALITY "SDL_RENDER_SCALE_QUALITY"
#undef  SDL_HINT_RENDER_VSYNC
#define SDL_HINT_RENDER_VSYNC "SDL_RENDER_VSYNC"
#undef  SDL_HINT_WINDOWS_DISABLE_THREAD_NAMING
#define SDL_HINT_WINDOWS_DISABLE_THREAD_NAMING "SDL_WINDOWS_DISABLE_THREAD_NAMING"

/* ---- types removed from SDL3 ---- */
/* SDL_GetVersion now returns a packed int; recreate the SDL2 struct. */
typedef struct compat_SDL_version { int major, minor, patch; } SDL_version;

/* SDL3's SDL_AudioSpec dropped silence/samples/callback/userdata, which the game
   still sets and reads, so we keep an SDL2-shaped spec and bridge it. */
typedef void (*SDL_AudioCallback)(void* userdata, Uint8* stream, int len);
typedef struct compat_SDL_AudioSpec {
	int               freq;
	SDL_AudioFormat   format;
	Uint8             channels;
	Uint8             silence;
	Uint16            samples;
	Uint32            size;
	SDL_AudioCallback callback;
	void*             userdata;
} compat_SDL_AudioSpec;

/* ======================================================================
 * Inline wrappers (call the REAL SDL3 functions; SDL2-name macros below).
 * ====================================================================== */

static inline int compat_SDL_Init(Uint32 flags) { return SDL_Init(flags) ? 0 : -1; }
static inline int compat_SDL_InitSubSystem(Uint32 flags) { return SDL_InitSubSystem(flags) ? 0 : -1; }

static inline SDL_Surface* compat_SDL_CreateRGBSurface(Uint32 flags, int w, int h, int depth,
		Uint32 rmask, Uint32 gmask, Uint32 bmask, Uint32 amask) {
	(void)flags;
	SDL_PixelFormat fmt = SDL_GetPixelFormatForMasks(depth, rmask, gmask, bmask, amask);
	SDL_Surface* s = SDL_CreateSurface(w, h, fmt);
	/* SDL3 does not auto-attach a palette to indexed surfaces; the game sets
	   palette entries on its 8bpp surfaces, so create one here. */
	if (s != NULL && SDL_BITSPERPIXEL(fmt) <= 8) SDL_CreateSurfacePalette(s);
	return s;
}

/* SDL2 SDL_ConvertSurface(surface, fmt*, flags) / SDL_ConvertSurfaceFormat(surface,
   fmtenum, flags) both collapse to SDL3's SDL_ConvertSurface(surface, fmtenum). */
static inline SDL_Surface* compat_SDL_ConvertSurface(SDL_Surface* src, SDL_PixelFormat fmt, Uint32 flags) {
	(void)flags;
	return SDL_ConvertSurface(src, fmt);
}

static inline Uint32 compat_SDL_MapRGB(SDL_PixelFormat fmt, Uint8 r, Uint8 g, Uint8 b) {
	return SDL_MapRGB(SDL_GetPixelFormatDetails(fmt), NULL, r, g, b);
}
static inline Uint32 compat_SDL_MapRGBA(SDL_PixelFormat fmt, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	return SDL_MapRGBA(SDL_GetPixelFormatDetails(fmt), NULL, r, g, b, a);
}

static inline int compat_SDL_LockSurface(SDL_Surface* s) { return SDL_LockSurface(s) ? 0 : -1; }
static inline int compat_SDL_FillRect(SDL_Surface* dst, const SDL_Rect* rect, Uint32 color) {
	return SDL_FillSurfaceRect(dst, rect, color) ? 0 : -1;
}
static inline int compat_SDL_BlitSurface(SDL_Surface* src, const SDL_Rect* srcrect,
		SDL_Surface* dst, SDL_Rect* dstrect) {
	return SDL_BlitSurface(src, srcrect, dst, dstrect) ? 0 : -1;
}
static inline int compat_SDL_BlitScaled(SDL_Surface* src, const SDL_Rect* srcrect,
		SDL_Surface* dst, SDL_Rect* dstrect) {
	return SDL_BlitSurfaceScaled(src, srcrect, dst, dstrect, SDL_SCALEMODE_NEAREST) ? 0 : -1;
}
static inline int compat_SDL_SetColorKey(SDL_Surface* s, int enabled, Uint32 key) {
	return SDL_SetSurfaceColorKey(s, enabled != 0, key) ? 0 : -1;
}
static inline int compat_SDL_SetSurfaceAlphaMod(SDL_Surface* s, Uint8 alpha) {
	return SDL_SetSurfaceAlphaMod(s, alpha) ? 0 : -1;
}
static inline int compat_SDL_SetPaletteColors(SDL_Palette* palette, const SDL_Color* colors,
		int firstcolor, int ncolors) {
	return SDL_SetPaletteColors(palette, colors, firstcolor, ncolors) ? 0 : -1;
}

static inline SDL_Window* compat_SDL_CreateWindow(const char* title, int x, int y,
		int w, int h, Uint32 flags) {
	(void)x; (void)y; /* SDL3 has no creation position (was SDL_WINDOWPOS_UNDEFINED) */
	return SDL_CreateWindow(title, w, h, flags);
}
static inline int compat_SDL_SetWindowFullscreen(SDL_Window* win, Uint32 flags) {
	return SDL_SetWindowFullscreen(win, flags != 0) ? 0 : -1;
}

static inline SDL_Renderer* compat_SDL_CreateRenderer(SDL_Window* win, int index, Uint32 flags) {
	(void)index;
	SDL_Renderer* r = SDL_CreateRenderer(win, (flags & SDL_RENDERER_SOFTWARE) ? "software" : NULL);
	if (r != NULL) SDL_SetRenderVSync(r, 0); /* keep VSync off (timer relies on it) */
	return r;
}
/* SDL3 merged logical size + integer scaling into one presentation call with a
   mode; keep them independent by re-reading the current size/mode each time. */
static inline int compat_SDL_RenderSetLogicalSize(SDL_Renderer* r, int w, int h) {
	int cw, ch; SDL_RendererLogicalPresentation m = SDL_LOGICAL_PRESENTATION_LETTERBOX;
	SDL_GetRenderLogicalPresentation(r, &cw, &ch, &m);
	if (m != SDL_LOGICAL_PRESENTATION_INTEGER_SCALE) m = SDL_LOGICAL_PRESENTATION_LETTERBOX;
	return SDL_SetRenderLogicalPresentation(r, w, h, m) ? 0 : -1;
}
static inline int compat_SDL_RenderSetIntegerScale(SDL_Renderer* r, bool enable) {
	int w, h; SDL_RendererLogicalPresentation m;
	SDL_GetRenderLogicalPresentation(r, &w, &h, &m);
	if (w == 0 || h == 0) { w = 320; h = 200; }
	return SDL_SetRenderLogicalPresentation(r, w, h,
		enable ? SDL_LOGICAL_PRESENTATION_INTEGER_SCALE : SDL_LOGICAL_PRESENTATION_LETTERBOX) ? 0 : -1;
}
static inline void compat_SDL_RenderGetLogicalSize(SDL_Renderer* r, int* w, int* h) {
	SDL_RendererLogicalPresentation m;
	SDL_GetRenderLogicalPresentation(r, w, h, &m);
}

static inline int compat_SDL_ShowCursor(int toggle) {
	if (toggle) SDL_ShowCursor(); else SDL_HideCursor();
	return 0;
}

/* SDL3 reports mouse position as float; the game uses int. */
static inline Uint32 compat_SDL_GetMouseState(int* x, int* y) {
	float fx = 0.0f, fy = 0.0f;
	Uint32 buttons = (Uint32)SDL_GetMouseState(&fx, &fy);
	if (x) *x = (int)fx;
	if (y) *y = (int)fy;
	return buttons;
}

/* SDL_RWread/RWwrite returned a count of OBJECTS; SDL_ReadIO/WriteIO work in
   bytes, so divide back by the element size to preserve the old semantics. */
static inline size_t compat_SDL_RWread(SDL_IOStream* ctx, void* ptr, size_t size, size_t maxnum) {
	if (size == 0) return 0;
	return SDL_ReadIO(ctx, ptr, size * maxnum) / size;
}
static inline size_t compat_SDL_RWwrite(SDL_IOStream* ctx, const void* ptr, size_t size, size_t num) {
	if (size == 0) return 0;
	return SDL_WriteIO(ctx, ptr, size * num) / size;
}
static inline int compat_SDL_RWclose(SDL_IOStream* ctx) { return SDL_CloseIO(ctx) ? 0 : -1; }

static inline void compat_SDL_GetVersion(SDL_version* v) {
	int n = SDL_GetVersion();
	v->major = SDL_VERSIONNUM_MAJOR(n);
	v->minor = SDL_VERSIONNUM_MINOR(n);
	v->patch = SDL_VERSIONNUM_MICRO(n);
}

/* Audio bridge + fast-forward resampler live in sdl2_to_sdl3.c (shared state). */
extern int  compat_SDL_OpenAudio(compat_SDL_AudioSpec* desired, compat_SDL_AudioSpec* obtained);
extern void compat_SDL_PauseAudio(int pause_on);
extern void compat_SDL_LockAudio(void);
extern void compat_SDL_UnlockAudio(void);
extern void compat_SDL_CloseAudio(void);
extern void compat_SDL_AudioResampleFF(SDL_AudioFormat fmt, int channels,
		int src_freq, int dst_freq, const Uint8* in, int in_len, Uint8* out, int out_len);

/* ======================================================================
 * SDL2-name macros. #undef first (oldnames/SDL3 may already define them as
 * macros) to avoid -Wmacro-redefined; defined last so the wrappers above are
 * never rewritten.
 * ====================================================================== */

/* Surfaces / pixels */
#undef  SDL_CreateRGBSurface
#define SDL_CreateRGBSurface             compat_SDL_CreateRGBSurface
#undef  SDL_ConvertSurface
#define SDL_ConvertSurface(s,f,fl)       compat_SDL_ConvertSurface((s),(f),(fl))
#undef  SDL_ConvertSurfaceFormat
#define SDL_ConvertSurfaceFormat(s,f,fl) compat_SDL_ConvertSurface((s),(f),(fl))
#undef  SDL_SetColorKey
#define SDL_SetColorKey                  compat_SDL_SetColorKey
#undef  SDL_SetSurfaceAlphaMod
#define SDL_SetSurfaceAlphaMod           compat_SDL_SetSurfaceAlphaMod
#undef  SDL_SetPaletteColors
#define SDL_SetPaletteColors             compat_SDL_SetPaletteColors
#undef  SDL_LockSurface
#define SDL_LockSurface                  compat_SDL_LockSurface
#undef  SDL_FillRect
#define SDL_FillRect                     compat_SDL_FillRect
#undef  SDL_BlitSurface
#define SDL_BlitSurface                  compat_SDL_BlitSurface
#undef  SDL_BlitScaled
#define SDL_BlitScaled                   compat_SDL_BlitScaled
#undef  SDL_MapRGB
#define SDL_MapRGB                       compat_SDL_MapRGB
#undef  SDL_MapRGBA
#define SDL_MapRGBA                      compat_SDL_MapRGBA

/* Window / renderer */
#undef  SDL_CreateWindow
#define SDL_CreateWindow                 compat_SDL_CreateWindow
#undef  SDL_SetWindowFullscreen
#define SDL_SetWindowFullscreen          compat_SDL_SetWindowFullscreen
#undef  SDL_CreateRenderer
#define SDL_CreateRenderer               compat_SDL_CreateRenderer
#undef  SDL_RenderSetLogicalSize
#define SDL_RenderSetLogicalSize         compat_SDL_RenderSetLogicalSize
#undef  SDL_RenderSetIntegerScale
#define SDL_RenderSetIntegerScale        compat_SDL_RenderSetIntegerScale
#undef  SDL_RenderGetLogicalSize
#define SDL_RenderGetLogicalSize         compat_SDL_RenderGetLogicalSize

/* Cursor / version */
#undef  SDL_ShowCursor
#define SDL_ShowCursor                   compat_SDL_ShowCursor
#undef  SDL_GetMouseState
#define SDL_GetMouseState                compat_SDL_GetMouseState
#undef  SDL_GetVersion
#define SDL_GetVersion                   compat_SDL_GetVersion
#undef  SDL_VERSION
#define SDL_VERSION(v) do { (v)->major = SDL_MAJOR_VERSION; (v)->minor = SDL_MINOR_VERSION; (v)->patch = SDL_MICRO_VERSION; } while (0)

/* RWops / IO (oldnames renames these but doesn't fix the signature/return) */
#undef  SDL_RWread
#define SDL_RWread                       compat_SDL_RWread
#undef  SDL_RWwrite
#define SDL_RWwrite                      compat_SDL_RWwrite
#undef  SDL_RWclose
#define SDL_RWclose                      compat_SDL_RWclose

/* SDL_image (SDL3_image; not covered by SDL_oldnames) */
#undef  IMG_Load_RW
#define IMG_Load_RW(rw,f)                IMG_Load_IO((rw),(f))
#undef  IMG_GetError
#define IMG_GetError                     SDL_GetError
#undef  IMG_SavePNG
#define IMG_SavePNG(s,f)                 (IMG_SavePNG((s),(f)) ? 0 : -1)

/* Text input now takes the window; SDL_SetTextInputRect became SDL_SetTextInputArea.
   These expand at call sites in seg009.c where `window_` (data.h) is in scope. */
#undef  SDL_StartTextInput
#define SDL_StartTextInput()             SDL_StartTextInput(window_)
#undef  SDL_StopTextInput
#define SDL_StopTextInput()              SDL_StopTextInput(window_)
#undef  SDL_SetTextInputRect
#define SDL_SetTextInputRect(r)          SDL_SetTextInputArea(window_, (r), 0)

/* Audio (init/return wrappers + the bridge) */
#undef  SDL_Init
#define SDL_Init                         compat_SDL_Init
#undef  SDL_InitSubSystem
#define SDL_InitSubSystem                compat_SDL_InitSubSystem
#undef  SDL_AudioSpec
#define SDL_AudioSpec                    compat_SDL_AudioSpec
#undef  SDL_OpenAudio
#define SDL_OpenAudio                    compat_SDL_OpenAudio
#undef  SDL_PauseAudio
#define SDL_PauseAudio                   compat_SDL_PauseAudio
#undef  SDL_LockAudio
#define SDL_LockAudio                    compat_SDL_LockAudio
#undef  SDL_UnlockAudio
#define SDL_UnlockAudio                  compat_SDL_UnlockAudio
#undef  SDL_CloseAudio
#define SDL_CloseAudio                   compat_SDL_CloseAudio

#endif /* SDL2_TO_SDL3_H */
