#ifndef CONFIG_H
#define CONFIG_H

#define WINDOW_TITLE "Prince of Persia (SDLPoP) v1.15"

// Window size; game will be scaled accordingly
#define POP_WINDOW_WIDTH 640
#define POP_WINDOW_HEIGHT 400

// Enable or disable fading.
// Fading used to be very buggy, but now it works correctly.
#define USE_FADE

// Enable or disable the potions level. (copy protection)
//#define USE_COPYPROT

// Enable or disable flashing.
#define USE_FLASH

//#define USE_ALPHA

// Enable or disable texts.
#define USE_TEXT

// Use timers in a way that is more similar to the original game.
// Was needed for the correct fading of cutscenes.
// Disabled, because it introduces some timing bugs.
//#define USE_COMPAT_TIMER

// Use mixer and enable music.
#define USE_MIXER

// Enable quicksave/load feature.
#define USE_QUICKSAVE

// Bugfixes:

// If a room is linked to itself on the left, the closing sounds of the gates in that room can't be heard.
//#define FIX_GATE_SOUNDS

// An open gate or chomper may enable the Kid to go through walls. (Trick 7, 37, 62)
//#define FIX_TWO_COLL_BUG

// If a room is linked to itself at the bottom, and the Kid's column has no floors, the game hangs.
//#define FIX_INFINITE_DOWN_BUG

// When a gate is under another gate, the top of the bottom gate is not visible.
// But this fix causes a drawing bug when a gate opens.
//#define FIX_GATE_DRAWING_BUG

// When climbing up to a floor with a big pillar top behind, turned right, Kid sees through floor.
// The current fix causes glitches you can see on bug_chomper.PNG and bug_climb.PNG .
//#define FIX_BIGPILLAR_CLIMB

#endif
