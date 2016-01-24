Name of program: SDLPoP
(Earlier name: David's open-source port of PoP)

Author: David from forum.princed.org (NagyD on GitHub)
Contributors:
* Andrew (bug reports)
* htamas (dungeon wall drawing algorithm, bug reports)
* Norbert (EndeavourAccuracy on GitHub) (bug reports, suggestions, improved gamepad support)
* musa (bug reports)
* Eugene (bug reports)
* StaticReturn (Mac OS X: Makefile (for older SDL1 version), bug reports)
* Poirot (ecalot on GitHub) (Mac OS X: Now compatible with Falcury SDL2 port)
* kees (bugfixes)
* Falcury (porting to SDL2, bugfixes, improvements, additions)
* segra (segrax on GitHub) (Joystick support, resizable window)

Topic in forum: http://forum.princed.org/viewtopic.php?f=69&t=3512
GitHub: https://github.com/NagyD/SDLPoP

GENERAL
=======

Q: What is this?
A: This is an open-source port/conversion of the DOS game Prince of Persia.
It is based on the disassembly of the original PoP1 for DOS.

Q: Where can I download that disassembly?
A: Here: http://forum.princed.org/viewtopic.php?f=68&t=3423
Scroll down to the newest zip files.
The exact version is PoP 1.0, i.e. pop1_ida.zip .
(But I also added some features from later versions.)

Sources that helped in making the disassembly:
* Modifications to prince.exe (hex editing) topic in the POPUW forum.
	- That forum is down, you can find some saved posts here: http://forum.princed.org/viewtopic.php?f=73&t=661
	- HTamas posted the dungeon wall drawing algorithm in C-style pseudocode here, along with many hex-edit hacks.
* PoP1 Technical Information by Mechner: http://www.popot.org/documentation.php?doc=OldDocuments
* PoP1 Apple II source code by Mechner: https://github.com/jmechner/Prince-of-Persia-Apple-II

LICENSE
=======

This program is open source under the GNU General Public License terms, see gpl-3.0.txt.

USAGE
=====

Q: How do I run it?
A:
Windows:
	Double-click on the prince.exe file.
	If you want to pass command line parameters, you need to open a command line.
GNU/Linux:
	First you have to compile the game. (See the DEVELOPING section.)
	Then you can start the game with the
		./prince
	command.
	(Or just double-click it in a file-manager.)

Q: What command-line options are there?
A:
* megahit -- Enable cheats.
* a number from 1 to 14 -- Start the given level.
* draw -- Draw directly to the screen, skipping the offscreen buffer.
* full -- Run in full screen mode.
* demo -- Run in demo mode: only the first two levels will be playable, and quotes from magazine reviews will be displayed.
* record -- Start recording immediately. (See the Replays section.)
* replay or a *.P1R filename -- Start replaying immediately. (See the Replays section.)

Q: What keys can I use?
A:
Controlling the kid:
* left: turn or run left
* right: turn or run right
* up: jump or climb up
* down: crouch or climb down
* shift: pick up things
* shift+left/right: careful step
* home or up+left: jump left
* page up or up+right: jump right
You can also use the numeric keypad.

Gamepad equivalents:
* left/right = left/right
* A = down
* B = quit
* X = shift
* Y = up

Controlling the game:
* Esc: pause game
* Space: show time left
* Ctrl-A: restart level
* Ctrl-G: save game (on levels 3..13)
* Ctrl-J: joystick mode (implemented by segrax) / gamepad mode (implemented by Norbert)
* Ctrl-K: keyboard mode
* Ctrl-R: return to intro
* Ctrl-S: sound on/off
* Ctrl-V: show version
* Ctrl-Q: quit game
* Ctrl-L: load game (when in the intro)
* Alt-Enter: toggle fullscreen
* F6: quicksave
* F9: quickload

Viewing or recording replays:
* Ctrl+Tab (in game): start or stop recording
* Tab (on title screen): view/cycle through the saved replays in the SDLPoP directory

Cheats:
* Shift-L: go to next level
* c: show numbers of current and adjacent rooms
* Shift-C: show numbers of diagonally adjacent rooms
* -: less remaining time
* +: more remaining time
* r: resurrect kid
* k: kill guard
* Shift-I: flip screen upside-down
* Shift-W: slow falling
* h: look at room to the left
* j: look at room to the right
* u: look at room above
* n: look at room below
* Shift-B: toggle hiding of non-animated objects
* Shift-S: Restore lost hit-point. (Like a small red potion.)
* Shift-T: Give more hit-points. (Like a big red potion.)

Q: Where is the music?
A:
Since version 1.13, the game supports loading music from the data/music folder.
The music is not included in releases because it is very big, and it does not change between versions.
You need to get the music from here: (38 MB)
	http://www.popot.org/get_the_games/various/PoP1_DOS_music.zip
It's the last link here: http://www.popot.org/get_the_games.php?game=1
Copy the OGG files to the data/music folder.

Since version 1.15, music is included.

MODS
====

Q: Can I play mods?
A:
Since version 1.02, the game supports LEVELS.DAT, and since version 1.03, the game can use all .DAT files.
You can either copy the modified .DAT files to the folder of the game, or the game to the mod's folder.

Another way is to start the game while the current directory is the mod's directory.
You can do this from the command line, or with batch files / shell scripts.
This is useful if you want to compare the behavior of this port and the original DOS version (to find bugs).
	Especially if you're editing the level and don't want to copy LEVELS.DAT from one place to the other.
/!\ Note that as of 1.03, the data/font folder and its contents must exist in the current directory!
	Since 1.11, the data/font folder is no longer required.

Note that this port does not recognize if the PRINCE.EXE of the mod was changed.
Since version 1.16, you can configure some options in SDLPoP.ini: starting time, level types, etc.

Beware, some mods (especially the harder ones) might rely on bugs that are fixed in SDLPoP.
Since version 1.16, SDLPoP will ask you whether gameplay quirks should be fixed or not.
You can set your choice permanently in the file 'SDLPoP.ini':
- Set the option 'use_fixes_and_enhancements' to 'false' to get the exact behavior of the original game.
- Alternatively, set the option 'use_fixes_and_enhancements' to 'true'. You can then also enable or disable 
  individual fixes and enhancements, depending on your preference.

Furthermore, SDLPoP opens up new possibilities for mod making.
For example:
Falcury released a mod, called "Secrets of the Citadel" that "has been designed to be played using a modified version of SDLPoP".
Description and download: http://forum.princed.org/viewtopic.php?f=73&t=3664

Since version 1.16, there is support for fake tiles, for example walls that the prince can go through.
The Apoplexy level editor supports these additional tiles since v3.0: http://www.popot.org/level_editors.php?editor=apoplexy
(Just don't overuse them, please!)

REPLAYS
=======

Q: How do replays work?
A:
Starting from version 1.16, you can capture or view replays in SDLPoP.
To start recording, press Ctrl+Tab while in game. To stop recording, press Ctrl+Tab again.
Your replays get saved in the SDLPoP folder as files with a .P1R extension (REPLAY_001.P1R, REPLAY_002.P1R, and so on).

To view a replay, you can press Tab while on the title screen. 
The game then looks for replays with the REPLAY_XXX.P1R pattern and plays those in order (you can cycle by pressing Tab again).
You can also double-click on a replay file (and tell the OS that the file needs to be opened with the SDLPoP executable).
SDLPoP will then immediately play that replay. Dragging and dropping onto the executable also works.

Your settings specified in SDLPoP.ini (including whether you are playing with bugfixes on or off) are remembered in the replay.
It shouldn't matter how SDLPoP.ini is set up when you are viewing the replay later.
Note that any cheats you use do not get saved as part of the replay.

If you want to start recording on a specific level, you can use the command "prince.exe record <lvl_number>",
where <lvl_number> is the level on which you want to start.

Also beware that the format of the replay files is not yet final and may change in the future!
So it is possible that replays you record now will not work well in future versions.

DEVELOPING
==========

Q: How do I (re)compile it?
A:
Prerequisites for all platforms:
	Make sure that you have the development versions of the "SDL", "SDL_image" and "SDL_mixer" (since 1.13) libraries installed.
	(These in turn require the "libjpeg", "libpng" and "zlib" libraries.)

Windows:
	If you are using Dev-C++:
		I use Dev-C++ version 4.9.9.2 from here: http://sourceforge.net/projects/dev-cpp/files/Binaries/
		You can download the libraries (except SDL2) from: http://sourceforge.net/projects/devpaks/files/
		I used these files:
			libjpeg-6b_4-1spec.DevPak
			libpng-1.2.7-1spec.DevPak
			zlib-1.2.3-1cm.DevPak
		You can install the libraries at Tools -> Package Manager.
		For Dev-C++ you need the MinGW Development Libraries of SDL2:
			http://libsdl.org/download-2.0.php
			http://libsdl.org/projects/SDL_image/
			http://libsdl.org/projects/SDL_mixer/
		To install these, just extract the contents of the i686-w64-mingw32 folder from each archive to the Dev-Cpp folder.
		To compile, open one of the .dev files and click the compile icon.

GNU/Linux:
	The libraries can be installed with apt-get or a package manager.
		sudo apt-get install libsdl2-image-dev libsdl2-mixer-dev
	Just type the command:
		make all
	and the game should compile.

Mac OS X:
	Get SDL2 and dependencies
		a) Install "port" from http://www.macports.org/
		b) sudo port install libsdl2 libsdl2_image libsdl2_mixer
	or
		a) Install "homebrew"
		b) sudo brew install libsdl2 libsdl2_image
		c) sudo brew install sdl2_mixer --with-libvorbis

	Get development tools:
		a) Install Xcode.
		b) Install the "command line developer tools" by typing 'xcode-select --install' at the prompt.
		c) Using terminal, in the root directory of SDLPOP, type: make

	PLAY!
		a) Type './prince' or './prince full'.
		b) Hit Control-Q to quit.

	Tested on OSX 10.9.5 with Xcode 6.0.1


