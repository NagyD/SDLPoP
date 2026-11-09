/* SDL2 shim implementation for GBA. */
#include <SDL2/SDL.h>
#include <gba.h>
#include <stdlib.h>
#include <string.h>
#include "gba_port.h"

static char sdl_error[256];

const char* SDL_GetError(void) { return sdl_error; }
void SDL_SetError(const char* fmt, ...) { (void)fmt; sdl_error[0] = 0; }
void SDL_free(void* p) { gba_free(p); }

int  SDL_Init(Uint32 flags)            { (void)flags; sdl_error[0] = 0; return 0; }
int  SDL_InitSubSystem(Uint32 flags)   { return SDL_Init(flags); }
void SDL_Quit(void)                    { }
void SDL_SetHint(const char* a, const char* b) { (void)a; (void)b; }
void SDL_PumpEvents(void)              { gba_input_poll(); }

/* ---- pixel format helpers ---- */
static void format_init(SDL_PixelFormat* f, SDL_Palette* p, int bpp) {
    f->BitsPerPixel  = (Uint8)bpp;
    f->BytesPerPixel = (Uint8)((bpp + 7) / 8);
    f->palette       = (bpp == 8) ? p : NULL;
    f->format        = (bpp == 8) ? SDL_PIXELFORMAT_INDEX8 :
                       (bpp == 24) ? SDL_PIXELFORMAT_RGB24 : SDL_PIXELFORMAT_ARGB8888;
    f->Rmask = 0x000000ff; f->Gmask = 0x0000ff00;
    f->Bmask = 0x00ff0000; f->Amask = (bpp == 32) ? 0xff000000u : 0;
}

Uint32 SDL_MapRGB(const SDL_PixelFormat* fmt, Uint8 r, Uint8 g, Uint8 b) {
    if (fmt->BitsPerPixel == 8 && fmt->palette) {
        int best = 0, bestd = 0x7fffffff;
        const SDL_Color* c = fmt->palette->colors;
        for (int i = 0; i < fmt->palette->ncolors; ++i) {
            int dr = (int)c[i].r - r, dg = (int)c[i].g - g, db = (int)c[i].b - b;
            int d = dr*dr + dg*dg + db*db;
            if (d < bestd) { bestd = d; best = i; }
        }
        return (Uint32)best;
    }
    return ((Uint32)b << 16) | ((Uint32)g << 8) | r;
}
Uint32 SDL_MapRGBA(const SDL_PixelFormat* fmt, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    (void)a; return SDL_MapRGB(fmt, r, g, b);
}
const char* SDL_GetPixelFormatName(Uint32 fmt) {
    switch (fmt) {
        case SDL_PIXELFORMAT_INDEX8:   return "SDL_PIXELFORMAT_INDEX8";
        case SDL_PIXELFORMAT_RGB24:    return "SDL_PIXELFORMAT_RGB24";
        case SDL_PIXELFORMAT_ARGB8888: return "SDL_PIXELFORMAT_ARGB8888";
    }
    return "?";
}

/* ---- surfaces ---- */
SDL_Surface* SDL_CreateRGBSurface(Uint32 flags, int w, int h, int depth,
                                  Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask) {
    (void)flags; (void)Rmask; (void)Gmask; (void)Bmask; (void)Amask;
    SDL_Surface* s = (SDL_Surface*)gba_calloc(1, sizeof(SDL_Surface));
    if (!s) return NULL;
    s->w = w; s->h = h;
    int bpp = depth > 0 ? depth : 8;
    s->pitch = w * ((bpp + 7) / 8);
    s->pixels = gba_calloc(1, (size_t)s->pitch * h);
    if (!s->pixels) { gba_free(s); return NULL; }
    s->_palette.ncolors = SDL_PALETTE_SIZE;
    format_init(&s->_format, &s->_palette, bpp);
    s->format = &s->_format;
    s->clip_rect.x = 0; s->clip_rect.y = 0;
    s->clip_rect.w = w; s->clip_rect.h = h;
    s->refcount = 1; s->alpha_mod = 255;
    s->blend_mode = SDL_BLENDMODE_NONE;
    s->has_colorkey = 0;
    return s;
}
/* GBA: allocate ONLY the SDL_Surface struct (no pixel buffer); the caller
   points ->pixels straight at the sprite data fixed in cartridge ROM.
   decode_image uses this for every sprite, so we never allocate (and free) a
   throwaway width*height buffer per sprite. That temp buffer used to leave a
   free-list hole between each persistent surface struct, growing the free list
   without bound and making first-fit malloc go quadratic — which stalled level
   loading (hundreds of optgraf/environment sprites) on a frozen screen. */
SDL_Surface* gba_new_rom_surface(int w, int h, int bpp) {
    SDL_Surface* s = (SDL_Surface*)gba_calloc(1, sizeof(SDL_Surface));
    if (!s) return NULL;
    if (bpp <= 0) bpp = 8;
    s->w = w; s->h = h;
    s->pitch = w * ((bpp + 7) / 8);
    s->pixels = NULL;             /* caller assigns the ROM pointer */
    s->_palette.ncolors = SDL_PALETTE_SIZE;
    format_init(&s->_format, &s->_palette, bpp);
    s->format = &s->_format;
    s->clip_rect.x = 0; s->clip_rect.y = 0;
    s->clip_rect.w = w; s->clip_rect.h = h;
    s->refcount = 1; s->alpha_mod = 255;
    s->blend_mode = SDL_BLENDMODE_NONE;
    s->has_colorkey = 0;
    return s;
}

void SDL_FreeSurface(SDL_Surface* s) {
    if (!s) return;
    if (--s->refcount > 0) return;
    /* GBA ROM-backed sprite: pixels points into cartridge ROM, must not
       be freed. __wrap_free already early-outs on the ROM range, but skip
       explicitly so the heap allocator doesn't even see it. */
    if (!s->gba_rom_compressed) gba_free(s->pixels);
    gba_free(s);
}
int SDL_LockSurface(SDL_Surface* s)   { (void)s; return 0; }
void SDL_UnlockSurface(SDL_Surface* s){ (void)s; }

int SDL_SetColorKey(SDL_Surface* s, int flag, Uint32 key) {
    if (!s) return -1;
    s->has_colorkey = flag ? 1 : 0;
    s->colorkey = key;
    return 0;
}
int SDL_SetSurfaceAlphaMod(SDL_Surface* s, Uint8 a)              { if(s) s->alpha_mod = a; return 0; }
int SDL_SetSurfaceBlendMode(SDL_Surface* s, SDL_BlendMode m)     { if(s) s->blend_mode = m; return 0; }
int SDL_SetClipRect(SDL_Surface* s, const SDL_Rect* r) {
    if (!s) return -1;
    if (!r) { s->clip_rect.x=0; s->clip_rect.y=0; s->clip_rect.w=s->w; s->clip_rect.h=s->h; }
    else s->clip_rect = *r;
    return 0;
}
int SDL_SetPaletteColors(SDL_Palette* p, const SDL_Color* colors, int first, int n) {
    if (!p) return -1;
    if (first < 0) first = 0;
    if (first >= SDL_PALETTE_SIZE) return 0;
    if (first + n > SDL_PALETTE_SIZE) n = SDL_PALETTE_SIZE - first;
    memcpy(&p->colors[first], colors, sizeof(SDL_Color) * n);
    if (p->ncolors < first + n) p->ncolors = first + n;
    return 0;
}

/* clip to dst clip_rect */
static void clip_rects(const SDL_Surface* src, const SDL_Rect* sr,
                       const SDL_Surface* dst, SDL_Rect* dr,
                       SDL_Rect* out_src, SDL_Rect* out_dst) {
    SDL_Rect s = sr ? *sr : (SDL_Rect){0, 0, src->w, src->h};
    SDL_Rect d = dr ? *dr : (SDL_Rect){0, 0, s.w, s.h};
    if (d.w == 0) d.w = s.w;
    if (d.h == 0) d.h = s.h;
    /* match dst clip */
    int cx0 = dst->clip_rect.x, cy0 = dst->clip_rect.y;
    int cx1 = cx0 + dst->clip_rect.w, cy1 = cy0 + dst->clip_rect.h;
    int dx0 = d.x, dy0 = d.y, dx1 = d.x + d.w, dy1 = d.y + d.h;
    if (dx0 < cx0) { s.x += cx0 - dx0; s.w -= cx0 - dx0; dx0 = cx0; }
    if (dy0 < cy0) { s.y += cy0 - dy0; s.h -= cy0 - dy0; dy0 = cy0; }
    if (dx1 > cx1) { s.w -= dx1 - cx1; dx1 = cx1; }
    if (dy1 > cy1) { s.h -= dy1 - cy1; dy1 = cy1; }
    if (s.w < 0) s.w = 0;
    if (s.h < 0) s.h = 0;
    d.x = dx0; d.y = dy0; d.w = dx1 - dx0; d.h = dy1 - dy0;
    *out_src = s; *out_dst = d;
}

/* Sprite decode scratch — only used as a fallback for sprites that did
   NOT get pre-decoded by tools/preproc_dat.  In normal operation this
   covers just the built-in hc_font_data glyphs (max ~16x16 / 256 bytes
   8bpp), so a tiny scratch is enough.  Any DAT-loaded sprite hits the
   pre-decoded fast path in SDL_BlitSurface and never touches this. */
#define GBA_SPRITE_DECOMPR_BYTES 1024
#define GBA_SPRITE_UNPACK_BYTES  2048
static uint8_t gba_decompr_buf [GBA_SPRITE_DECOMPR_BYTES] __attribute__((aligned(4)));
static uint8_t gba_unpacked_buf[GBA_SPRITE_UNPACK_BYTES]  __attribute__((aligned(4)));

extern void  decompr_img(uint8_t* dest, const void* source, int decomp_size,
                          int cmeth, int stride);
extern uint8_t* conv_to_8bpp(uint8_t* in_data, int width, int height,
                              int stride, int depth);
extern int   calc_stride(void* image_data);

uint8_t* gba_decode_compressed_surface(SDL_Surface* src) {
    /* Walks the compressed image_data_type at src->pixels (ROM), decodes
       into gba_unpacked_buf, applies the sprite's palette-row offset, and
       returns the unpacked buffer. Caller treats it as a contiguous w*h
       8bpp image with rows of length src->w. */
    int w = src->w, h = src->h;
    int depth = src->gba_depth;
    int cmeth = src->gba_cmeth;
    int stride = (depth * w + 7) / 8;
    int dest_size = stride * h;
    if (dest_size > GBA_SPRITE_DECOMPR_BYTES || (w * h) > GBA_SPRITE_UNPACK_BYTES) return NULL;
    memset(gba_decompr_buf, 0, dest_size);
    /* image_data_type in ROM: {height u16, width u16, flags u16, data[]}.
       decompr_img takes a pointer to it and reads .data via offset. */
    decompr_img(gba_decompr_buf, src->pixels, dest_size, cmeth, stride);
    /* Inline conv_to_8bpp directly into gba_unpacked_buf so we don't malloc. */
    int pixels_per_byte = 8 / depth;
    int mask = (1 << depth) - 1;
    for (int y = 0; y < h; ++y) {
        const uint8_t* in_pos = gba_decompr_buf + y * stride;
        uint8_t* out_pos = gba_unpacked_buf + y * w;
        int x_pixel = 0;
        for (int x_byte = 0; x_byte < stride; ++x_byte) {
            uint8_t v = *in_pos++;
            int shift = 8;
            for (int p = 0; p < pixels_per_byte && x_pixel < w; ++p, ++x_pixel) {
                shift -= depth;
                *out_pos++ = (v >> shift) & mask;
            }
        }
    }
    /* Do NOT bake the palette-row offset here — SDL_BlitSurface applies
       it per-pixel during the blit so the same scratch contents work for
       any destination, and so the fast path for pre-decoded sprites
       (which also have raw 0..15 values in their ROM-backed pixels) goes
       through the same blit logic. */
    return gba_unpacked_buf;
}

int SDL_BlitSurface(SDL_Surface* src, const SDL_Rect* srcrect,
                    SDL_Surface* dst, SDL_Rect* dstrect) {
    if (!src || !dst) return -1;
    /* GBA: decode ROM-resident sprites into the scratch buffer first; from
       here on src->pixels behaves like a normal 8bpp surface. */
    uint8_t* saved_pixels = NULL;
    int      saved_pitch  = 0;
    if (src->gba_rom_compressed) {
        uint8_t* unpacked = gba_decode_compressed_surface(src);
        if (unpacked == NULL) return -1;
        saved_pixels = (uint8_t*)src->pixels;
        saved_pitch  = src->pitch;
        src->pixels  = unpacked;
        src->pitch   = src->w;
    }

    SDL_Rect s, d;
    clip_rects(src, srcrect, dst, dstrect, &s, &d);
    if (s.w <= 0 || s.h <= 0) {
        if (saved_pixels) { src->pixels = saved_pixels; src->pitch = saved_pitch; }
        return 0;
    }
    int sbpp = src->format->BytesPerPixel;
    int dbpp = dst->format->BytesPerPixel;
    int n    = (s.w < d.w ? s.w : d.w);
    int rows = (s.h < d.h ? s.h : d.h);

    if (sbpp == 1 && dbpp == 1) {
        /* Source pixels are raw 0..15 values (4bpp expanded, or 8bpp
           that happens to use only the bottom nibble — PoP sprites are
           4bpp). We map each non-zero pixel to its slot in the global
           PoP palette[] by adding the per-surface row offset baked at
           decode_image time. Pixel 0 is the universal colorkey and
           stays transparent. */
        int has_key = src->has_colorkey;
        Uint8 key  = (Uint8)src->colorkey;
        Uint8 off  = src->gba_palette_offset;

        for (int y = 0; y < rows; ++y) {
            const Uint8* sp = (const Uint8*)src->pixels + (size_t)(s.y + y) * src->pitch + s.x;
            Uint8* dp = (Uint8*)dst->pixels + (size_t)(d.y + y) * dst->pitch + d.x;
            if (off != 0) {
                if (has_key) {
                    for (int x = 0; x < n; ++x) {
                        Uint8 v = sp[x];
                        if (v == key) continue;
                        dp[x] = (Uint8)(off + (v & 0x0F));
                    }
                } else {
                    for (int x = 0; x < n; ++x) {
                        dp[x] = (Uint8)(off + (sp[x] & 0x0F));
                    }
                }
            } else if (has_key) {
                for (int x = 0; x < n; ++x) {
                    Uint8 v = sp[x];
                    if (v != key) dp[x] = v;
                }
            } else {
                memcpy(dp, sp, (size_t)n);
            }
        }
    } else {
        /* generic 1->N or N->N copy; we mostly stay in 8bpp on GBA */
        for (int y = 0; y < rows; ++y) {
            const Uint8* sp = (const Uint8*)src->pixels + (size_t)(s.y + y) * src->pitch + s.x * sbpp;
            Uint8* dp = (Uint8*)dst->pixels + (size_t)(d.y + y) * dst->pitch + d.x * dbpp;
            int bytes = n * (sbpp < dbpp ? sbpp : dbpp);
            memcpy(dp, sp, (size_t)bytes);
        }
    }
    if (dstrect) *dstrect = d;
    if (saved_pixels) { src->pixels = saved_pixels; src->pitch = saved_pitch; }
    return 0;
}

int SDL_BlitScaled(SDL_Surface* src, const SDL_Rect* sr,
                   SDL_Surface* dst, SDL_Rect* dr) {
    /* On GBA, "scaling" just becomes a clipped blit — final output is 240x160. */
    return SDL_BlitSurface(src, sr, dst, dr);
}

int SDL_FillRect(SDL_Surface* dst, const SDL_Rect* rect, Uint32 color) {
    if (!dst) return -1;
    SDL_Rect r = rect ? *rect : (SDL_Rect){0, 0, dst->w, dst->h};
    int cx0 = dst->clip_rect.x, cy0 = dst->clip_rect.y;
    int cx1 = cx0 + dst->clip_rect.w, cy1 = cy0 + dst->clip_rect.h;
    int x0 = r.x < cx0 ? cx0 : r.x;
    int y0 = r.y < cy0 ? cy0 : r.y;
    int x1 = r.x + r.w > cx1 ? cx1 : r.x + r.w;
    int y1 = r.y + r.h > cy1 ? cy1 : r.y + r.h;
    if (x1 <= x0 || y1 <= y0) return 0;
    int bpp = dst->format->BytesPerPixel;
    for (int y = y0; y < y1; ++y) {
        Uint8* dp = (Uint8*)dst->pixels + (size_t)y * dst->pitch + x0 * bpp;
        if (bpp == 1) memset(dp, (int)color, (size_t)(x1 - x0));
        else {
            for (int x = 0; x < x1 - x0; ++x) {
                memcpy(dp + x * bpp, &color, (size_t)bpp);
            }
        }
    }
    return 0;
}

SDL_Surface* SDL_ConvertSurfaceFormat(SDL_Surface* src, Uint32 fmt, Uint32 flags) {
    (void)flags;
    if (!src) return NULL;
    /* If caller wants the same format, refcount-bump and return. Otherwise
       we MUST actually allocate a new buffer at the target pixel size — the
       upstream method_3_blit_mono path converts 8bpp font glyphs to
       ARGB8888 and then writes 4 bytes per pixel into the returned buffer.
       The previous stub that just incremented refcount caused a 4x heap
       overrun on every text character drawn, scribbling over malloc'd
       data and producing the "stuck on partial frame" behaviour. */
    if (fmt == src->format->format) {
        src->refcount++;
        return src;
    }
    int dst_bpp = (fmt == SDL_PIXELFORMAT_INDEX8) ? 8 :
                  (fmt == SDL_PIXELFORMAT_RGB24)  ? 24 : 32;
    SDL_Surface* dst = SDL_CreateRGBSurface(0, src->w, src->h, dst_bpp,
                                            0, 0, 0, 0);
    if (!dst) return NULL;
    if (src->has_colorkey) SDL_SetColorKey(dst, 1, src->colorkey);

    if (src->format->BytesPerPixel == 1 && dst_bpp == 32) {
        /* 8bpp paletted -> ARGB8888 via the source palette. Memory layout
           (little-endian, our shim): byte0=R, byte1=G, byte2=B, byte3=A. */
        const SDL_Color* pal = src->format->palette ? src->format->palette->colors : NULL;
        for (int y = 0; y < src->h; ++y) {
            const Uint8* sp = (const Uint8*)src->pixels + (size_t)y * src->pitch;
            Uint8* dp = (Uint8*)dst->pixels + (size_t)y * dst->pitch;
            for (int x = 0; x < src->w; ++x) {
                Uint8 idx = sp[x];
                SDL_Color c = pal ? pal[idx] : (SDL_Color){idx, idx, idx, 255};
                dp[0] = c.r; dp[1] = c.g; dp[2] = c.b; dp[3] = c.a ? c.a : 255;
                dp += 4;
            }
        }
    } else {
        /* generic same-bpp copy fallback */
        int min_bpp = src->format->BytesPerPixel < dst->format->BytesPerPixel ?
                      src->format->BytesPerPixel : dst->format->BytesPerPixel;
        for (int y = 0; y < src->h; ++y) {
            const Uint8* sp = (const Uint8*)src->pixels + (size_t)y * src->pitch;
            Uint8* dp = (Uint8*)dst->pixels + (size_t)y * dst->pitch;
            for (int x = 0; x < src->w; ++x)
                memcpy(dp + x * dst->format->BytesPerPixel,
                       sp + x * src->format->BytesPerPixel, min_bpp);
        }
    }
    return dst;
}
SDL_Surface* SDL_ConvertSurface(SDL_Surface* src, const SDL_PixelFormat* fmt, Uint32 flags) {
    (void)fmt; (void)flags;
    if (!src) return NULL;
    /* Return a copy of src so that callers can free both independently. */
    SDL_Surface* dst = SDL_CreateRGBSurface(0, src->w, src->h,
                                            src->format->BitsPerPixel, 0, 0, 0, 0);
    if (!dst) return NULL;
    if (src->format->palette) {
        SDL_SetPaletteColors(dst->format->palette, src->format->palette->colors,
                             0, src->format->palette->ncolors);
    }
    if (src->has_colorkey) SDL_SetColorKey(dst, 1, src->colorkey);
    memcpy(dst->pixels, src->pixels, (size_t)src->pitch * src->h);
    return dst;
}
int SDL_SetSurfacePalette(SDL_Surface* s, SDL_Palette* palette) {
    if (!s || !palette) return -1;
    SDL_SetPaletteColors(&s->_palette, palette->colors, 0, palette->ncolors);
    return 0;
}

/* ---- RWops, memory-backed ---- */
static Sint64 mem_size(SDL_RWops* rw)            { return rw->hidden.mem.stop - rw->hidden.mem.base; }
static Sint64 mem_seek(SDL_RWops* rw, Sint64 ofs, int whence) {
    const Uint8* p = rw->hidden.mem.here;
    if (whence == RW_SEEK_SET) p = rw->hidden.mem.base + ofs;
    else if (whence == RW_SEEK_CUR) p = rw->hidden.mem.here + ofs;
    else if (whence == RW_SEEK_END) p = rw->hidden.mem.stop + ofs;
    if (p < rw->hidden.mem.base) p = rw->hidden.mem.base;
    if (p > rw->hidden.mem.stop) p = rw->hidden.mem.stop;
    rw->hidden.mem.here = p;
    return p - rw->hidden.mem.base;
}
static size_t mem_read(SDL_RWops* rw, void* ptr, size_t size, size_t n) {
    size_t avail = (size_t)(rw->hidden.mem.stop - rw->hidden.mem.here) / (size ? size : 1);
    size_t got = n < avail ? n : avail;
    memcpy(ptr, rw->hidden.mem.here, got * size);
    rw->hidden.mem.here += got * size;
    return got;
}
static size_t mem_write(SDL_RWops* rw, const void* ptr, size_t size, size_t n) {
    (void)rw; (void)ptr; (void)size; (void)n; return 0;
}
/* Static RWops pool: SDLPoP only ever holds a handful of memory RWops open at
   once (font/data loads), so a small fixed pool avoids any allocation. */
#define RWOPS_POOL 8
static SDL_RWops s_rwops[RWOPS_POOL];
static Uint8     s_rwops_used[RWOPS_POOL];

static int mem_close(SDL_RWops* rw) {
    for (int i = 0; i < RWOPS_POOL; ++i) {
        if (rw == &s_rwops[i]) { s_rwops_used[i] = 0; break; }
    }
    return 0;
}
SDL_RWops* SDL_RWFromConstMem(const void* m, int sz) {
    SDL_RWops* rw = NULL;
    for (int i = 0; i < RWOPS_POOL; ++i) {
        if (!s_rwops_used[i]) { s_rwops_used[i] = 1; rw = &s_rwops[i]; break; }
    }
    if (!rw) return NULL;
    memset(rw, 0, sizeof(*rw));
    rw->size = mem_size; rw->seek = mem_seek; rw->read = mem_read;
    rw->write = mem_write; rw->close = mem_close;
    rw->hidden.mem.base = rw->hidden.mem.here = (const Uint8*)m;
    rw->hidden.mem.stop = rw->hidden.mem.base + sz;
    return rw;
}
SDL_RWops* SDL_RWFromMem(void* m, int sz) { return SDL_RWFromConstMem(m, sz); }
SDL_RWops* SDL_RWFromFile(const char* file, const char* mode) { (void)file; (void)mode; return NULL; }
int    SDL_RWclose(SDL_RWops* rw)                       { return rw && rw->close ? rw->close(rw) : 0; }
size_t SDL_RWread(SDL_RWops* rw, void* p, size_t s, size_t n) { return rw->read(rw, p, s, n); }
size_t SDL_RWwrite(SDL_RWops* rw, const void* p, size_t s, size_t n) { return rw->write(rw, p, s, n); }
Sint64 SDL_RWseek(SDL_RWops* rw, Sint64 o, int w)       { return rw->seek(rw, o, w); }
Sint64 SDL_RWtell(SDL_RWops* rw)                        { return rw->seek(rw, 0, RW_SEEK_CUR); }
Sint64 SDL_RWsize(SDL_RWops* rw)                        { return rw->size(rw); }

/* ---- Timing ---- */
Uint64 SDL_GetPerformanceCounter(void)   { return (Uint64)gba_get_ticks_ms() * 1000ULL; }
Uint64 SDL_GetPerformanceFrequency(void) { return 1000000ULL; }
Uint32 SDL_GetTicks(void)                { return gba_get_ticks_ms(); }
void   SDL_Delay(Uint32 ms)              { gba_delay_ms(ms); }

/* ---- Cursor / text input — no-ops on GBA ---- */
int  SDL_ShowCursor(int t)                 { (void)t; return 0; }
void SDL_StartTextInput(void)              { }
void SDL_StopTextInput(void)               { }
int  SDL_SetTextInputRect(const SDL_Rect* r){ (void)r; return 0; }

/* ---- Window/renderer/texture stubs (we draw directly to GBA VRAM) ---- */
static SDL_Window      _win;
static SDL_Renderer    _rend;
static SDL_Texture     _tex[4];
SDL_Window*   SDL_CreateWindow(const char* t, int x, int y, int w, int h, Uint32 f) {
    (void)t; (void)x; (void)y; (void)w; (void)h; (void)f; return &_win;
}
void          SDL_DestroyWindow(SDL_Window* w)  { (void)w; }
SDL_Renderer* SDL_CreateRenderer(SDL_Window* w, int i, Uint32 f) { (void)w; (void)i; (void)f; return &_rend; }
void          SDL_DestroyRenderer(SDL_Renderer* r) { (void)r; }
SDL_Texture*  SDL_CreateTexture(SDL_Renderer* r, Uint32 fmt, int acc, int w, int h) {
    (void)r; (void)fmt; (void)acc; (void)w; (void)h;
    static int idx = 0;
    return &_tex[idx++ & 3];
}
void          SDL_DestroyTexture(SDL_Texture* t)   { (void)t; }
int  SDL_RenderClear(SDL_Renderer* r)              { (void)r; return 0; }
int  SDL_RenderCopy(SDL_Renderer* r, SDL_Texture* t, const SDL_Rect* a, const SDL_Rect* b) {
    (void)r; (void)t; (void)a; (void)b; return 0;
}
void SDL_RenderPresent(SDL_Renderer* r)            { (void)r; }
int  SDL_SetRenderTarget(SDL_Renderer* r, SDL_Texture* t) { (void)r; (void)t; return 0; }
int  SDL_UpdateTexture(SDL_Texture* t, const SDL_Rect* r, const void* p, int pitch) {
    (void)t; (void)r; (void)p; (void)pitch; return 0;
}
int  SDL_RenderSetLogicalSize(SDL_Renderer* r, int w, int h)  { (void)r; (void)w; (void)h; return 0; }
int  SDL_RenderSetIntegerScale(SDL_Renderer* r, SDL_bool e)   { (void)r; (void)e; return 0; }
int  SDL_RenderGetLogicalSize(SDL_Renderer* r, int* w, int* h){ (void)r; if(w)*w=240; if(h)*h=160; return 0; }
/* No scaling/letterboxing on GBA: 1:1 to the 240x160 framebuffer. */
void SDL_RenderGetScale(SDL_Renderer* r, float* sx, float* sy){ (void)r; if(sx)*sx=1.0f; if(sy)*sy=1.0f; }
void SDL_RenderGetViewport(SDL_Renderer* r, SDL_Rect* rect){ (void)r; if(rect){ rect->x=0; rect->y=0; rect->w=240; rect->h=160; } }
int  SDL_GetRendererOutputSize(SDL_Renderer* r, int* w, int* h){ (void)r; if(w)*w=240; if(h)*h=160; return 0; }
int  SDL_GetWindowFlags(SDL_Window* w)             { (void)w; return 0; }
void SDL_SetWindowFullscreen(SDL_Window* w, Uint32 f) { (void)w; (void)f; }
void SDL_SetWindowIcon(SDL_Window* w, SDL_Surface* s) { (void)w; (void)s; }
int  SDL_GetRendererInfo(SDL_Renderer* r, SDL_RendererInfo* info) {
    (void)r;
    if (info) {
        info->name = "gba";
        info->flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE;
        info->num_texture_formats = 1;
        info->texture_formats[0]  = SDL_PIXELFORMAT_INDEX8;
        info->max_texture_width   = 240;
        info->max_texture_height  = 160;
    }
    return 0;
}
void SDL_GL_GetDrawableSize(SDL_Window* w, int* a, int* b) { (void)w; if(a)*a=240; if(b)*b=160; }
void SDL_GetWindowSize     (SDL_Window* w, int* a, int* b) { (void)w; if(a)*a=240; if(b)*b=160; }
int  SDL_UpdateRect(SDL_Surface* s, int x, int y, int w, int h) {
    (void)x; (void)y; (void)w; (void)h;
    return gba_present_surface(s);
}
int  SDL_Flip(SDL_Surface* s) { return gba_present_surface(s); }

/* ---- Audio (only minimal API; the real mixer is in gba_audio.c) ---- */
int  SDL_OpenAudio(SDL_AudioSpec* desired, SDL_AudioSpec* obtained) {
    if (obtained) *obtained = *desired;
    gba_audio_init(desired);
    return 0;
}
void SDL_CloseAudio(void) { gba_audio_shutdown(); }
void SDL_PauseAudio(int on) { gba_audio_set_paused(on); }
void SDL_LockAudio(void)    { gba_audio_lock(); }
void SDL_UnlockAudio(void)  { gba_audio_unlock(); }
int  SDL_GetAudioStatus(void) { return gba_audio_status(); }
int  SDL_BuildAudioCVT(SDL_AudioCVT* cvt, SDL_AudioFormat sf, Uint8 sc, int sr,
                       SDL_AudioFormat df, Uint8 dc, int dr) {
    (void)sf; (void)sc; (void)sr; (void)df; (void)dc; (void)dr;
    if (cvt) { cvt->needed = 0; cvt->len_mult = 1; cvt->len_ratio = 1.0; }
    return 0;
}
int  SDL_ConvertAudio(SDL_AudioCVT* cvt) { if (cvt) cvt->len_cvt = cvt->len; return 0; }

/* ---- Joystick / haptic — full stubs ---- */
int  SDL_NumJoysticks(void) { return 0; }
SDL_GameController* SDL_GameControllerOpen(int i)  { (void)i; return NULL; }
void SDL_GameControllerClose(SDL_GameController* c){ (void)c; }
SDL_bool SDL_IsGameController(int i) { (void)i; return SDL_FALSE; }
SDL_GameController* SDL_GameControllerFromInstanceID(int i) { (void)i; return NULL; }
SDL_Joystick* SDL_JoystickOpen(int i) { (void)i; return NULL; }
int  SDL_GameControllerAddMappingsFromFile(const char* f) { (void)f; return 0; }
SDL_Haptic* SDL_HapticOpen(int i) { (void)i; return NULL; }
int  SDL_HapticRumbleInit(SDL_Haptic* h) { (void)h; return 0; }
int  SDL_HapticRumblePlay(SDL_Haptic* h, float a, Uint32 b) { (void)h; (void)a; (void)b; return 0; }

/* ---- Timers (one shot for VBlank ticking; we wire SDLPoP's frame timer here) ---- */
static SDL_TimerCallback s_tcb;
static Uint32 s_tinterval;
static void* s_tparam;
SDL_TimerID SDL_AddTimer(Uint32 interval, SDL_TimerCallback cb, void* p) {
    s_tcb = cb; s_tinterval = interval; s_tparam = p;
    gba_timer_install(interval);
    return 1;
}
int  SDL_RemoveTimer(SDL_TimerID id) { (void)id; s_tcb = NULL; gba_timer_uninstall(); return 1; }

/* Called by gba_timer_tick() (VBlank handler) at the configured cadence. */
void gba_timer_fire_cb(void) {
    if (s_tcb) {
        Uint32 next = s_tcb(s_tinterval, s_tparam);
        if (!next) { s_tcb = NULL; gba_timer_uninstall(); }
        else if (next != s_tinterval) { s_tinterval = next; gba_timer_install(next); }
    }
}

/* ---- iconv / wchar — not used on GBA path ---- */
char* SDL_iconv_string(const char* a, const char* b, const char* c, size_t d) {
    (void)a; (void)b; (void)c; (void)d; return NULL;
}
size_t SDL_wcslen(const Uint16* w) {
    const Uint16* p = w; while (*p) ++p; return (size_t)(p - w);
}
size_t SDL_strlen(const char* s) { return strlen(s); }

/* ---- Event queue ---- */
#define EVQ_CAP 32
static SDL_Event s_evq[EVQ_CAP];
static int s_evq_head, s_evq_tail;

int SDL_PollEvent(SDL_Event* event) {
    gba_input_poll();
    if (s_evq_head == s_evq_tail) return 0;
    if (event) *event = s_evq[s_evq_head];
    s_evq_head = (s_evq_head + 1) % EVQ_CAP;
    return 1;
}
int SDL_PushEvent(SDL_Event* event) {
    int next = (s_evq_tail + 1) % EVQ_CAP;
    if (next == s_evq_head) return 0;
    s_evq[s_evq_tail] = *event;
    s_evq_tail = next;
    return 1;
}

static Uint8 s_keystate[SDL_NUM_SCANCODES];
const Uint8* SDL_GetKeyboardState(int* numkeys) {
    if (numkeys) *numkeys = SDL_NUM_SCANCODES;
    return s_keystate;
}
/* GBA has no mouse: report (0,0) with no buttons held so the menu falls back to
   D-pad/button navigation. */
Uint32 SDL_GetMouseState(int* x, int* y) {
    if (x) *x = 0;
    if (y) *y = 0;
    return 0;
}
/* The controls menu prints key-binding names. We don't ship a scancode-name
   table on GBA, so return an empty string (the numeric code is shown too). */
const char* SDL_GetScancodeName(SDL_Scancode scancode) {
    (void)scancode;
    return "";
}
void gba_set_key(int scancode, int down) {
    if ((unsigned)scancode >= SDL_NUM_SCANCODES) return;
    Uint8 was = s_keystate[scancode];
    s_keystate[scancode] = down ? 1 : 0;
    if ((!was) != (!down)) {
        SDL_Event e; memset(&e, 0, sizeof(e));
        e.type = down ? SDL_KEYDOWN : SDL_KEYUP;
        e.key.timestamp = SDL_GetTicks();
        e.key.state = down ? 1 : 0;
        e.key.keysym.scancode = scancode;
        e.key.keysym.sym = scancode;
        SDL_PushEvent(&e);
    }
}
