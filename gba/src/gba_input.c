/* GBA button -> SDLPoP scancode mapping.
   Mapping is chosen so the on-screen actions match the PC keyboard defaults:
     D-Pad    -> arrow keys (move, jump, crouch)
     A        -> Shift (grab/draw sword/pick up)
     B        -> Space (no-op pause / map back to escape menu)
     L        -> Ctrl  (chop, fight)
     R        -> Alt   (modifier)
     Start    -> Enter (advance dialog/menus)
     Select   -> Backspace (cancel / quicksave toggle)
*/
#include <gba.h>
#include <SDL2/SDL.h>
#include "gba_port.h"

extern void gba_set_key(int scancode, int down);

static u16 s_prev_keys;

static const struct { u16 mask; int scancode; } s_map[] = {
    { KEY_LEFT,   SDL_SCANCODE_LEFT   },
    { KEY_RIGHT,  SDL_SCANCODE_RIGHT  },
    { KEY_UP,     SDL_SCANCODE_UP     },
    { KEY_DOWN,   SDL_SCANCODE_DOWN   },
    { KEY_A,      SDL_SCANCODE_LSHIFT },
    { KEY_B,      SDL_SCANCODE_SPACE  },
    { KEY_L,      SDL_SCANCODE_LCTRL  },
    { KEY_R,      SDL_SCANCODE_LALT   },
    { KEY_START,  SDL_SCANCODE_RETURN },
    { KEY_SELECT, SDL_SCANCODE_BACKSPACE },
};

void gba_input_init(void) {
    s_prev_keys = 0;
}

void gba_input_poll(void) {
    /* KEYINPUT bits are 0 when pressed; XOR-flip to "pressed = 1". */
    u16 keys = ~REG_KEYINPUT & 0x03FF;
    u16 changed = keys ^ s_prev_keys;
    if (!changed) return;

    /* DEBUG (visible with --agb-print): show raw button state each change so we
       can tell whether the emulator is delivering presses to the ROM at all. */
    { extern void gba_log(const char*,...); gba_log("KEYINPUT raw=0x%03x pressed=0x%03x", (unsigned)(REG_KEYINPUT & 0x3FF), (unsigned)keys); }

    for (unsigned i = 0; i < sizeof(s_map)/sizeof(s_map[0]); ++i) {
        u16 m = s_map[i].mask;
        if (changed & m) {
            gba_set_key(s_map[i].scancode, (keys & m) ? 1 : 0);
        }
    }
    s_prev_keys = keys;
}
