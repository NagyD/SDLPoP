/* GBA Mode 4 (240x160 @ 8bpp paletted) front-end for the SDL shim.
   Single framebuffer: VRAM page A (0x6000000) is the one and only surface,
   always displayed. The game maintains the full frame in onscreen_surface_
   and we copy it here once per frame right after VBlank. (The old A/B page
   flip is gone.) */
#include <gba.h>
#include <string.h>
#include "gba_port.h"

#define MODE4_FB_A ((u16*)0x06000000)
#define BG_PAL     ((u16*)0x05000000)

static int s_vp_x, s_vp_y;

/* Palette-row offset for the next decode_image call. Set by
   load_sprites_from_file before its inner load_image loop; reset to 0 after.
   Read by decode_image to bake row*16 into each non-zero sprite pixel value
   so 8bpp blits land at the correct slot of the global palette[]. */
int gba_decode_palette_offset = 0;

void gba_video_init(void) {
    /* IMPORTANT: GBA VRAM and PALRAM ignore 8-bit writes (or duplicate the
       byte into both halves of a u16). newlib's memset writes byte-at-a-time
       on unaligned tails, so we use explicit 32-bit stores here. */
    volatile u32* pageA = (volatile u32*)0x06000000;
    volatile u32* pal32 = (volatile u32*)0x05000000;
    for (int i = 0; i < (240*160)/4; ++i) pageA[i] = 0;
    for (int i = 0; i < 512/4; ++i) pal32[i] = 0;

    /* Single, always-displayed page A — no BACKBUFFER bit, never flipped. */
    SetMode(MODE_4 | BG2_ON);
    /* Centre the 240x160 viewport over the 320x192 PoP world so the first
       frame shows the playable area instead of the top-left padding/status. */
    s_vp_x = (320 - 240) / 2;  /* 40 */
    s_vp_y = (192 - 160) / 2;  /* 16 */
}

/* Visual panic screen: flood the screen with a solid color and halt. Used by
   our quit() override and assertion paths so we don't fall off the end into
   newlib's exit() loop. */
void gba_panic(uint8_t color_idx) {
    BG_PAL[color_idx] = RGB5(31, 0, 0); /* red */
    u16 fill = (u16)(color_idx | (color_idx << 8));
    for (int i = 0; i < 240*160/2; ++i) MODE4_FB_A[i] = fill;
    REG_DISPCNT = MODE_4 | BG2_ON;
    while (1) VBlankIntrWait();
}

void gba_set_viewport(int x, int y) { s_vp_x = x; s_vp_y = y; }
void gba_get_viewport(int* x, int* y) { if (x) *x = s_vp_x; if (y) *y = s_vp_y; }

void gba_apply_palette(const SDL_Color* colors, int first, int n) {
    for (int i = 0; i < n; ++i) {
        int idx = first + i;
        if ((unsigned)idx >= 256) break;
        u8 r = colors[i].r >> 3;
        u8 g = colors[i].g >> 3;
        u8 b = colors[i].b >> 3;
        BG_PAL[idx] = RGB5(r, g, b);
    }
}

/* SDLPoP maintains its own rgb_type palette[256] in seg009.c via set_pal*();
   it does not always sync that to SDL_Palette. We read both and let SDL_Palette
   override where it's been populated. */
struct rgb_type_ext { uint8_t r, g, b; };
extern struct rgb_type_ext palette[256];

/* Copy a clipped 240x160 view from the (possibly larger) source surface to
   the back framebuffer page, then flip pages on VBlank. */
int gba_present_surface(SDL_Surface* s) {
    if (!s || !s->pixels || s->format->BytesPerPixel != 1) return -1;

    /* Upload SDLPoP's authoritative palette[] to BG_PAL. Values are 6-bit
       VGA DAC (0..63), shift by 1 to map to GBA's 5-bit (0..31). The surface's
       own SDL_Palette is intentionally NOT applied: onscreen_surface_ is
       blitted to from sprite surfaces but its palette is never populated, so
       reading it would clobber BG_PAL with all-zero colors -> black screen.
       Only re-upload when palette[] actually changed (it changes per level /
       torch flicker, not every frame) — present() is called several times per
       game frame, so this avoids 256 RGB conversions + PALRAM writes each. */
    static struct rgb_type_ext s_pal_cache[256];
    if (memcmp(s_pal_cache, palette, sizeof(s_pal_cache)) != 0) {
        memcpy(s_pal_cache, palette, sizeof(s_pal_cache));
        for (int i = 0; i < 256; ++i) {
            u8 r = palette[i].r >> 1;
            u8 g = palette[i].g >> 1;
            u8 b = palette[i].b >> 1;
            if (r > 31) r = 31;
            if (g > 31) g = 31;
            if (b > 31) b = 31;
            BG_PAL[i] = RGB5(r, g, b);
        }
    }

    int sx = s_vp_x, sy = s_vp_y;
    if (sx < 0) sx = 0;
    if (sy < 0) sy = 0;
    if (sx > s->w - 240) sx = s->w - 240 > 0 ? s->w - 240 : 0;
    if (sy > s->h - 160) sy = s->h - 160 > 0 ? s->h - 160 : 0;

    int rows = s->h - sy; if (rows > 160) rows = 160;
    int cols = s->w - sx; if (cols > 240) cols = 240;

    /* Wait for VBlank, then write straight into the single displayed page so
       the copy starts in the blank interval. Mode 4 VRAM only accepts 16-bit
       writes (two pixels at once); pack pairs from the source row. */
    VBlankIntrWait();
    u16* dst = MODE4_FB_A;
    for (int y = 0; y < rows; ++y) {
        const u8* sp = (const u8*)s->pixels + (size_t)(sy + y) * s->pitch + sx;
        u16* dp = dst + (size_t)y * 120;
        int x = 0;
        for (; x + 1 < cols; x += 2) {
            dp[x >> 1] = (u16)(sp[x] | (sp[x + 1] << 8));
        }
        if (x < cols) {
            /* odd-pixel column: read-modify-write to preserve neighbor */
            u16 v = dp[x >> 1];
            dp[x >> 1] = (v & 0xFF00) | sp[x];
        }
    }
    /* zero-out unused right/bottom if source is smaller than 240x160 */
    for (int y = rows; y < 160; ++y) {
        u16* dp = dst + (size_t)y * 120;
        for (int x = 0; x < 120; ++x) dp[x] = 0;
    }
    return 0;
}
