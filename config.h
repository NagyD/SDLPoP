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

// Adds a way to crouch immediately after climbing up: press down and forward simultaneously
// In the original game, this could not be done (pressing down always causes the kid to climb down).
#define ALLOW_CROUCH_AFTER_CLIMBING

// Enable one-minute penalty for quickloading
#define USE_QUICKLOAD_PENALTY

// Time passes while the level ending music plays; however, this can be skipped by disabling sound.
// This disables time passing while the ending music is playing, so you can leave sounds on.
#define DISABLE_TIME_DURING_END_MUSIC

// Bugfixes:

// The mentioned tricks can be found here: http://www.popot.org/documentation.php?doc=Tricks

// If a room is linked to itself on the left, the closing sounds of the gates in that room can't be heard.
#define FIX_GATE_SOUNDS

// An open gate or chomper may enable the Kid to go through walls. (Trick 7, 37, 62)
#define FIX_TWO_COLL_BUG

// If a room is linked to itself at the bottom, and the Kid's column has no floors, the game hangs.
#define FIX_INFINITE_DOWN_BUG

// When a gate is under another gate, the top of the bottom gate is not visible.
// But this fix causes a drawing bug when a gate opens.
//#define FIX_GATE_DRAWING_BUG

// When climbing up to a floor with a big pillar top behind, turned right, Kid sees through floor.
// The current fix causes glitches you can see on bug_chomper.PNG and bug_climb.PNG .
//#define FIX_BIGPILLAR_CLIMB

// When climbing up two floors, turning around and jumping upward, the kid falls down.
// This fix makes the workaround of Trick 25 unnecessary.
#define FIX_JUMP_DISTANCE_AT_EDGE

// When climbing to a higher floor, the game unnecessarily checks how far away the edge below is;
// This contributes to sometimes "teleporting" considerable distances when climbing from firm ground
#define FIX_EDGE_DISTANCE_CHECK_WHEN_CLIMBING

// Falling from a great height directly on top of guards does not hurt.
#define FIX_PAINLESS_FALL_ON_GUARD

// Bumping against a wall may cause a loose floor below to drop, even though it has not been touched (Trick 18, 34)
#define FIX_WALL_BUMP_TRIGGERS_TILE_BELOW

// When pressing a loose tile, you can temporarily stand on thin air by standing up from crouching.
#define FIX_STAND_ON_THIN_AIR

// Buttons directly to the right of gates can be pressed even though the gate is closed (Trick 1)
#define FIX_PRESS_THROUGH_CLOSED_GATES

// By jumping and bumping into a wall, you can sometimes grab a ledge two stories down (which should not be possible).
#define FIX_GRAB_FALLING_SPEED

// When chomped, skeletons cause the chomper to become bloody even though skeletons do not have blood.
#define FIX_SKELETON_CHOMPER_BLOOD

// Controls do not get released properly when drinking a potion, sometimes causing unintended movements.
#define FIX_MOVE_AFTER_DRINK

// Guards may "follow" the kid to the room on the left, even though there is a closed gate in between.
#define FIX_GUARD_FOLLOWING_THROUGH_CLOSED_GATES


// Debug features:

// When the program starts, check whether the deobfuscated sequence table (seqtbl.c) is correct.
//#define CHECK_SEQTABLE_MATCHES_ORIGINAL

// Enable debug cheats
// "[" and "]" : nudge x position by one pixel
// "T" : display remaining time in minutes, seconds and ticks
//#define USE_DEBUG_CHEATS

#endif
