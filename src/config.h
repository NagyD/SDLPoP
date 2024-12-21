/*
SDLPoP, a port/conversion of the DOS game Prince of Persia.
Copyright (C) 2013-2023  Dávid Nagy

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

The authors of this program may be contacted at https://forum.princed.org
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

#define SDLPOP_VERSION "1.23"
#define WINDOW_TITLE "Prince of Persia (SDLPoP) v" SDLPOP_VERSION

#if ! (defined WIN32 || _WIN32 || WIN64 || _WIN64)
#define POP_DIR_NAME "SDLPoP"
#define SHARE_PATH "/usr/share"
#endif

// Enable or disable fading.
// Fading used to be very buggy, but now it works correctly.
#define USE_FADE

// Enable or disable the potions level. (copy protection)
#define USE_COPYPROT

// Enable or disable flashing.
#define USE_FLASH

//#define USE_ALPHA

// Enable or disable texts.
#define USE_TEXT

// Use timers in a way that is more similar to the original game.
// Was needed for the correct fading of cutscenes.
// Disabled, because it introduces some timing bugs.
//#define USE_COMPAT_TIMER

// Enable quicksave/load feature.
#define USE_QUICKSAVE

// Try to let time keep running out when quickloading. (similar to Ctrl+A)
// Technically, the 'remaining time' is still restored, but with a penalty for elapsed time (up to 1 minute).
// The one minute penalty will also be applied when quickloading from e.g. the title screen.
#define USE_QUICKLOAD_PENALTY

#ifdef USE_QUICKSAVE // Replay relies on quicksave, because the replay file begins with a quicksave of the initial state.

// Enable recording/replay feature.
#define USE_REPLAY

#endif

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

// The mentioned tricks can be found here: https://www.popot.org/documentation.php?doc=Tricks

// A compilation-time option to disable all fixes. Useful for automated solving tools that require vanilla emulation. 
#ifndef DISABLE_ALL_FIXES

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

// While putting the sword away, if you press forward and down, and then release down, the kid will still duck.
#define FIX_MOVE_AFTER_SHEATHE

// After uniting with the shadow in level 12, the hidden floors will not appear until after the flashing stops.
#define FIX_HIDDEN_FLOORS_DURING_FLASHING

// By jumping towards one of the bottom corners of the room and grabbing a ledge, you can teleport to the room above.
#define FIX_HANG_ON_TELEPORT

// Fix priorities of sword and spike sounds. (As in PoP 1.3.)
#define FIX_SOUND_PRIORITIES

// Don't draw the right edge of loose floors on the left side of a potion or sword.
#define FIX_LOOSE_NEXT_TO_POTION

// A guard standing on a door top (with floor) should not become inactive.
#define FIX_DOORTOP_DISABLING_GUARD

// Fix graphical glitches with an opening gate:
// 1. with a loose floor above and a wall above-right.
// 2. with the top half of a big pillar above-right.
// Details: https://forum.princed.org/viewtopic.php?p=31884#p31884
#define FIX_ABOVE_GATE

// Disable this fix to make it possible to go through a certain closed gate on level 11 of Demo by Suave Prince.
// Details: https://forum.princed.org/viewtopic.php?p=32326#p32326
// Testcase: doc/replays-testcases/Demo by Suave Prince level 11.p1r
//#define FIX_COLL_FLAGS

// The prince can now grab a ledge at the bottom right corner of a room with no room below.
// Details: https://forum.princed.org/viewtopic.php?p=30410#p30410
// Testcase: doc/replays-testcases/SNES-PC-set level 11.p1r
#define FIX_CORNER_GRAB

// When the prince jumps up at the bottom of a big pillar split between two rooms, a part near the top of the screen disappears.
// Example: The top row in the first room of the original level 5.
// Videos: https://forum.princed.org/viewtopic.php?p=32227#p32227
// Explanation: https://forum.princed.org/viewtopic.php?p=32414#p32414
#define FIX_BIGPILLAR_JUMP_UP

// When the prince dies behind a wall, and he is revived with R, he appears in a glitched room.
// (Example: The bottom right part of the bottom right room of level 3.)
// The same room can also be reached by falling into a wall. (Falling into the wall, itself, is a different glitch, though.)
// Testcase: doc/replays-testcases/Original level 2 falling into wall.p1r
// More info: https://forum.princed.org/viewtopic.php?f=68&t=4467
#define FIX_ENTERING_GLITCHED_ROOMS

// If you are using the caped prince graphics, and crouch with your back towards a closed gate on the left edge on the room, then the prince will slide through the gate.
// You can also try this with the original graphics if your use the debug cheat "[" to push the prince into the gate.
// This option fixes that.
// You can get the caped prince graphics here: https://www.popot.org/custom_levels.php?action=KID.DAT (it's the one by Veke)
// Video: https://www.popot.org/documentation.php?doc=TricksPage3#83
// Explanation: https://forum.princed.org/viewtopic.php?p=32701#p32701
// This also fixes the bug described at FIX_COLL_FLAGS.
#define FIX_CAPED_PRINCE_SLIDING_THROUGH_GATE

// If the prince dies on level 14, restarting the level will not stop the "Press Button to Continue" timer, and the game will return to the intro after a few seconds.
// How to reproduce: https://forum.princed.org/viewtopic.php?p=16926#p16926
// Technical explanation: https://forum.princed.org/viewtopic.php?p=16408#p16408 (the second half of the post)
#define FIX_LEVEL_14_RESTARTING

// If a sprite's xpos is negative and divisible by 8, it will appear shifted 8 pixels to the left.
// Testcase: doc/replays-testcases/Original level 12 xpos glitch.p1r
// Explanation: https://forum.princed.org/viewtopic.php?p=33336#p33336
#define FIX_SPRITE_XPOS

// On the original level 5, if you leave the room while the shadow is drinking the potion, the shadow runs to the right.
// He will eventually fall into the wall, at this point a black rectangle appears on the wall.
// It's also related to the raise button next to the wall. The rectangle won't appear without that button.
// Testcase: doc/replays-testcases/Original level 5 shadow into wall.p1r
// See also: https://github.com/NagyD/SDLPoP/issues/254
#define FIX_BLACK_RECT
// TODO: Also fix the shadow not turning around and/or falling into the wall? Or would that break mods?

// The prince can jump over a guard with a properly timed running jump.
// His character's x coordinate ends up in the column behind the guard which causes the bump sequence
// not to work correctly.
#define FIX_JUMPING_OVER_GUARD

// The prince can fall down 2 rooms while climbing a loose tile located in the room above. (Trick 153)
// It happens when the player hangs on the loose tile holding Shift for a second before climbing up.
// The fix ensures the tile does not start to fall until the climbing sequence changes prince's current row.
// Testcase: doc/replays-testcases/trick_153.p1r
// See also: https://github.com/NagyD/SDLPoP/pull/272
#define FIX_DROP_2_ROOMS_CLIMBING_LOOSE_TILE

// The prince or a guard can fall through a floor during a sword strike even though there is a floor tile in front of him.
// A strike sequence consists of 4 important frames, 151-154. Frame 153 is different from the other 3 because has a flag
// that it "needs a floor". The problem is strike frames a pretty wide so the character's tile is not calculated correctly
// causing him to visually fall through the floor.
// This fix prevents falling during that frame treating it like it does not require a floor.
// Testcase: doc/replays-testcases/Falling through floor (PR274).p1r
// See also: https://github.com/NagyD/SDLPoP/pull/274
#define FIX_FALLING_THROUGH_FLOOR_DURING_SWORD_STRIKE

// Prince can start running when he is standing really close to a wall/gate/tapestry but facing in a different direction.
// He can either press buttons through gates or falling back when facing right against a closed gate when there is an abiss behind.
// This fix ensures the logic that makes Prince safe-stepping instead of running is also working when he is facing in the opposite direction.
#define FIX_TURN_RUN_NEAR_WALL

// When the player (re-)enters the currently shown room, the guard disappears from the screen.
// This can happen when:
// * A room is linked to itself (broken link).
// * The player used the "show other room" cheat.
// * A teleport and its pair are in the same room.
// There are two fixes for this:
//#define FIX_DISAPPEARING_GUARD_A // Inserts a black screen.
//#define FIX_DISAPPEARING_GUARD_B // Doesn't insert a black screen.

#endif // ifndef DISABLE_ALL_FIXES

// Prince can jump 2 stories up in feather fall mode
#define USE_SUPER_HIGH_JUMP

// Prince can grab tiles on the row above from a standing or a running jump
#define USE_JUMP_GRAB

// Debug features:

// When the program starts, check whether the deobfuscated sequence table (seqtbl.c) is correct.
//#define CHECK_SEQTABLE_MATCHES_ORIGINAL

// Print out every second how closely the in-game elapsed time corresponds to the actual elapsed time.
//#define CHECK_TIMING


// Enable debug cheats (with command-line argument "debug")
// "[" and "]" : nudge x position by one pixel
// "T" : display remaining time in minutes, seconds and ticks
#define USE_DEBUG_CHEATS



// Darken those parts of the screen which are not near a torch.
#define USE_LIGHTING

// Enable screenshot features.
#define USE_SCREENSHOT

// Automatically switch to keyboard or joystick/gamepad mode if there is input from that device.
// Useful if SDL detected a gamepad but there is none.
#define USE_AUTO_INPUT_MODE

#ifdef USE_TEXT // The menu won't work without text.

// Display the in-game menu.
#define USE_MENU

#endif

// Enable colored torches. A torch can be colored by changing its modifier in a level editor.
#define USE_COLORED_TORCHES

// Enable fast forwarding with the backtick key.
#define USE_FAST_FORWARD

// Set how much should the fast forwarding speed up the game.
#define FAST_FORWARD_RATIO 10

// Speed up the sound during fast forward using resampling.
// If disabled, the sound is sped up by clipping out parts from it.
//#define FAST_FORWARD_RESAMPLE_SOUND

// Mute the sound during fast forward.
//#define FAST_FORWARD_MUTE

// Briefly show a dark screen when changing rooms, like in the original game.
#define USE_DARK_TRANSITION

// Turn the balconies into teleports.
// Each balcony (whose left half has a non-zero modifier) will behave as a teleport to another balcony with the same modifier.
// The right half of such balconies must have modifier == 1.
#define USE_TELEPORTS


// Default SDL_Joystick button values
#define SDL_JOYSTICK_BUTTON_Y 2
#define SDL_JOYSTICK_BUTTON_X 3
#define SDL_JOYSTICK_X_AXIS 0
#define SDL_JOYSTICK_Y_AXIS 1


#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  #define Rmsk 0x00ff0000
  #define Gmsk 0x0000ff00
  #define Bmsk 0x000000ff
  #define Amsk 0xff000000
#else
  #define Rmsk 0x000000ff
  #define Gmsk 0x0000ff00
  #define Bmsk 0x00ff0000
  #define Amsk 0xff000000
#endif


#endif
