/*
SDLPoP, a port/conversion of the DOS game Prince of Persia.
Copyright (C) 2013-2017  Dávid Nagy

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

// WINDOWS overrides
#ifdef _MSC_VER
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif

#define POP_MAX_PATH 256
#define POP_MAX_OPTIONS_SIZE 256

#define SDLPOP_VERSION_NUMBER "1.17"

// Enable or disable fading.
// Fading used to be very buggy, but now it works correctly.
#define USE_FADE

// Enable or disable the potions level. (copy protection)
#define USE_COPYPROT

// Enable or disable the editor.
#define USE_EDITOR

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

// Try to let time keep running out when quickloading. (similar to Ctrl+A)
// Technically, the 'remaining time' is still restored, but with a penalty for elapsed time (up to 1 minute).
// The one minute penalty will also be applied when quickloading from e.g. the title screen.
#define USE_QUICKLOAD_PENALTY

// Enable recording/replay feature.
#define USE_REPLAY

// Adds a way to crouch immediately after climbing up: press down and forward simultaneously.
// In the original game, this could not be done (pressing down always causes the kid to climb down).
#define ALLOW_CROUCH_AFTER_CLIMBING

// Time runs out while the level ending music plays; however, the music can be skipped by disabling sound.
// This option stops time while the ending music is playing (so there is no need to disable sound).
#define FREEZE_TIME_DURING_END_MUSIC

// Enable fake/invisible tiles feature. Tiles may look like one tiletype but behave like another.
// Currently works for empty tiles, walls, floors.
// Use tile modifier 4 to display a fake floor, 5 to display a fake wall, 6 to display an empty tile
// For now, for fake dungeon walls, the wall neighbors must be specified for now using tile modifiers:
//      5 or 50 = no neighbors; 51 = wall to the right; 52 = wall to the left; 53 = walls on both sides
// For fake palace walls:
//      5 = wall including blue line; 50 = no blue
#define USE_FAKE_TILES

// Allow guard hitpoints not resetting to their default (maximum) value when re-entering the room
#define REMEMBER_GUARD_HP

// Enable completely disabling the time limit. To use this feature, set the starting time to -1.
// This also disables the in-game messages that report how much time is left every minute.
// The elasped time is still kept track of, so that the shortest times will appear in the Hall of Fame.
#define ALLOW_INFINITE_TIME


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
#define FIX_GATE_DRAWING_BUG

// When climbing up to a floor with a big pillar top behind, turned right, Kid sees through floor.
#define FIX_BIGPILLAR_CLIMB

// When climbing up two floors, turning around and jumping upward, the kid falls down.
// This fix makes the workaround of Trick 25 unnecessary.
#define FIX_JUMP_DISTANCE_AT_EDGE

// When climbing to a higher floor, the game unnecessarily checks how far away the edge below is;
// This contributes to sometimes "teleporting" considerable distances when climbing from firm ground
#define FIX_EDGE_DISTANCE_CHECK_WHEN_CLIMBING

// Falling from a great height directly on top of guards does not hurt.
#define FIX_PAINLESS_FALL_ON_GUARD

// Bumping against a wall may cause a loose floor below to drop, even though it has not been touched. (Trick 18, 34)
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

// A drawing bug occurs when a loose tile is placed to the left of a potion (or sword).
#define FIX_LOOSE_LEFT_OF_POTION

// Guards may "follow" the kid to the room on the left or right, even though there is a closed gate in between.
#define FIX_GUARD_FOLLOWING_THROUGH_CLOSED_GATES

// When landing on the edge of a spikes tile, it is considered safe. (Trick 65)
#define FIX_SAFE_LANDING_ON_SPIKES

// The kid may glide through walls after turning around while running (especially when weightless).
#define FIX_GLIDE_THROUGH_WALL

// The kid can drop down through a closed gate, when there is a tapestry (doortop) above the gate.
#define FIX_DROP_THROUGH_TAPESTRY

// When dropping down and landing right in front of a wall, the entire landing animation should normally play.
// However, when falling against a closed gate or a tapestry(+floor) tile, the animation aborts.
// (The game considers these tiles floor tiles; so it mistakenly assumes that no x-position adjustment is needed)
#define FIX_LAND_AGAINST_GATE_OR_TAPESTRY

// Sometimes, the kid may automatically strike immediately after drawing the sword.
// This especially happens when dropping down from a higher floor and then turning towards the opponent.
#define FIX_UNINTENDED_SWORD_STRIKE

// By repeatedly pressing 'back' in a swordfight, you can retreat out of a room without the room changing. (Trick 35)
#define FIX_RETREAT_WITHOUT_LEAVING_ROOM

// The kid can jump through a tapestry with a running jump to the left, if there is a floor above it.
#define FIX_RUNNING_JUMP_THROUGH_TAPESTRY

// Guards can be pushed into walls, because the game does not correctly check for walls located behind a guard.
#define FIX_PUSH_GUARD_INTO_WALL

// By doing a running jump into a wall, you can fall behind a closed gate two floors down. (e.g. skip in Level 7)
#define FIX_JUMP_THROUGH_WALL_ABOVE_GATE

// If you grab a ledge that is one or more floors down, the chompers on that row will not start.
#define FIX_CHOMPERS_NOT_STARTING

// As soon as a level door has completely opened, the feather fall effect is interrupted because the sound stops.
#define FIX_FEATHER_INTERRUPTED_BY_LEVELDOOR

// Guards will often not reappear in another room if they have been pushed (partly or entirely) offscreen.
#define FIX_OFFSCREEN_GUARDS_DISAPPEARING

// Controls do not get released properly when putting the sword away, leading to unintended movement.
#define FIX_MOVE_AFTER_SHEATHE


// Debug features:

// When the program starts, check whether the deobfuscated sequence table (seqtbl.c) is correct.
//#define CHECK_SEQTABLE_MATCHES_ORIGINAL

// Enable debug cheats (with command-line argument "debug")
// "[" and "]" : nudge x position by one pixel
// "T" : display remaining time in minutes, seconds and ticks
#define USE_DEBUG_CHEATS



#ifdef USE_EDITOR
#define SDLPOP_VERSION_NAME SDLPOP_VERSION_NUMBER " - editor"
#else
#define SDLPOP_VERSION_NAME SDLPOP_VERSION_NUMBER
#endif

#define WINDOW_TITLE "Prince of Persia (SDLPoP) v" SDLPOP_VERSION_NAME

#endif
