/* Viewport pan/clamp for the 240x160 GBA screen.
   The PoP game world renders into a 320x192 offscreen surface (one room).
   We track the prince's logical X position and clamp the camera so he stays
   in a centre band. Called once per frame from a hook in seg009.c update path. */
#include "gba_port.h"

extern void   gba_set_viewport(int x, int y);
extern void   gba_get_viewport(int* x, int* y);

#define WORLD_W 320
#define WORLD_H 192
#define VIEW_W  240
#define VIEW_H  160

/* The prince's ACTUAL drawn pixel position in the 320x192 onscreen buffer,
   recorded each frame by draw_mid() (see src/seg008.c, #ifdef __GBA__). This is
   the real blit position, unlike Char.x which is PoP's internal coordinate and
   does not map linearly to buffer pixels. gba_kid_valid stays 0 until the prince
   has been drawn at least once (menus/cutscenes), in which case we keep the
   centred default. */
int gba_kid_px = 0, gba_kid_py = 0, gba_kid_valid = 0;

/* Set by draw_menu() (src/menu.c) while the pause/settings menu is open. The
   menu pins the viewport itself (see gba_menu_set_viewport there), so the
   per-frame camera follow below must not fight it. */
int gba_menu_active = 0;

static int clampi(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

void gba_camera_update(void) {
    if (gba_menu_active) return; /* menu owns the viewport while open */

    /* Default: centre the 240x160 viewport over the 320x192 world. */
    int target_x = (WORLD_W - VIEW_W) / 2;  /* 40 */
    int target_y = (WORLD_H - VIEW_H) / 2;  /* 16 */

    if (gba_kid_valid) {
        /* Centre on the prince's drawn pixel position, clamped to the world.
           This follows him on BOTH axes (the X range is 0..80, the Y range is
           0..32 — the room is only 32px taller than the view). */
        target_x = clampi(gba_kid_px - VIEW_W / 2, 0, WORLD_W - VIEW_W);
        target_y = clampi(gba_kid_py - VIEW_H / 2, 0, WORLD_H - VIEW_H);
    }

    /* Follow responsively: close most of the gap each frame (a quarter-step,
       min 1px) so running/falling stays framed without 1px-per-frame lag, yet
       still glides rather than hard-snapping. */
    int cx, cy;
    gba_get_viewport(&cx, &cy);
    int dx = target_x - cx, dy = target_y - cy;
    cx += (dx > 0 ? (dx + 3) / 4 : -((-dx + 3) / 4));
    cy += (dy > 0 ? (dy + 3) / 4 : -((-dy + 3) / 4));
    gba_set_viewport(cx, cy);
}
