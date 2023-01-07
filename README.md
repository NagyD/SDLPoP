# SDLPoP

An open-source port of Prince of Persia, based on the disassembly of the DOS version, extended with new features.

## Links

* Forum board: https://forum.princed.org/viewforum.php?f=126
* GitHub: https://github.com/NagyD/SDLPoP
* Compiled versions: https://www.popot.org/get_the_games.php?game=SDLPoP

## Authors

Author:

* NagyD (David from `forum.princed.org`)

Contributors:

* Andrew (bug reports)
* htamas (inspiration, dungeon wall drawing algorithm, bug reports)
* Norbert (EndeavourAccuracy on GitHub) (bug reports, suggestions, improved gamepad support)
* musa (bug reports)
* Eugene (bug reports)
* StaticReturn (Mac OS X: Makefile (for older SDL1 version), bug reports)
* Poirot (ecalot on GitHub) (Mac OS X: Now compatible with Falcury SDL2 port)
* kees (bugfixes)
* Falcury
    * porting to SDL2
    * quicksave improvements
    * replay files
    * SDLPoP.ini: added basic support and constantly adding new options
    * mod folders
    * fake tiles
    * readable sequence table
    * CMake support
    * pause menu
    * MIDI support
    * and various other bugfixes, improvements, additions
* segra (segrax on GitHub) (Joystick support, resizable window)
* DarkPrince (bug reports)
* Andrey Vasilkin / digi@os2.snc.ru (eComStation (OS/2) support)
* mfn (fixed a small bug when `USE_MIXER` is undefined)
* diddledani (Visual C++/NMake support)
* zaps166 (small Makefile fixes)
* usineur (faster music loading)
* yaqxsw (icon)
* SergioMartin86
* dstarosta
* Ryzee119
* berikv
* xyproto
* BarrOff
* icculus
* thrimbor
* vanfanel
* asppsa
* akien-mga
* mbeijen
* alebcay
* ckorn
* Rondom
* yuv422
* dive

Usernames on the list above refer to either GitHub users or users from `forum.princed.org`.

## GENERAL

### What is this?

This is an open-source port/conversion of the DOS game Prince of Persia.
It is based on the disassembly of the original PoP1 for DOS.

Note, however, that SDLPoP has many new features not found in the original game.
These are marked as such in the command-line and the keys sections below.

### Where can I download that disassembly?

Here: https://forum.princed.org/viewtopic.php?f=68&t=3423

* Scroll down to the newest zip files.
* The exact version is PoP 1.0, i.e. `pop1_ida.zip` (but with features added from later versions).

Sources which helped in making the disassembly:

* Modifications to `prince.exe` (hex editing) topic in the PoPUW forum.
  - That forum is down, you can find some saved posts here: https://forum.princed.org/viewtopic.php?f=73&t=661
  - HTamas posted the dungeon wall drawing algorithm in C-style pseudocode here, along with many hex-edit hacks.
  - It was his work that prompted me to start the disassembly and later SDLPoP. Thank you!
* PoP1 Technical Information by Mechner: https://www.popot.org/documentation.php?doc=OldDocuments
* PoP1 Apple II source code by Mechner: https://github.com/jmechner/Prince-of-Persia-Apple-II

## LICENSE

This program is open source under the GNU General Public License terms, see `COPYING` and `src/GPLv3.h`.

The following files are external libraries with their own licenses: `src/opl3.c`, `src/opl3.h`, `src/stb_vorbis.c`.
See their contents for license information.

## USAGE

### Running the game

#### Windows

* Double-click on the `prince.exe` file.
* If you want to pass command line parameters, you need to open a command line.

#### GNU/Linux

##### Arch Linux

* Use your favorite AUR helper and install the `sdlpop` package.
* Start the game with `prince` or from the menu of your desktop environment.

##### Debian/Ubuntu

* Compile the game manually (see instructions below).
* Start the game with `./prince`.

#### macOS

* See the compilation instructions below. Thanks to StaticReturn and Poirot.

#### Compiled versions

* You can find compiled versions for these three platforms here: https://www.popot.org/get_the_games.php?game=SDLPoP
* Unofficial ports for other systems can be found here: https://forum.princed.org/viewtopic.php?f=126&t=4728. These were not made by the authors of SDLPoP.

### What command-line options are there?

* `megahit` -- Enable cheats.
* a number from `0` to `15` -- Start the given level. (Works only together with `megahit` or `record`.)
* `draw` -- Draw directly to the screen, skipping the offscreen buffer.
* `demo` -- Run in demo mode: only the first two levels will be playable, and quotes from magazine reviews will be displayed.
* `stdsnd` -- Use PC speaker sounds.

**The following don't exist in the original game:**

* `full` -- Run in full screen mode.
* `record` -- Start recording immediately. (See the Replays section.)
* `replay` or a `*.P1R` filename -- Start replaying immediately. (See the Replays section.)
* `validate "replays/replay.p1r"` -- Print out information about a replay file and quit. (See the Replays section.)
* `mod "Mod Name"` -- Run with custom data files from the folder "mods/Mod Name/"
* `debug` -- Enable debug cheats.
* `--version`, `-v` -- Display SDLPoP version and quit.
* `--help`, `-h`, `-?` -- Display help and quit. (Currently it only points to this Readme...)
* `seed=number` -- Set initial random seed, for testing.
* `--screenshot` -- Must be used with megahit and a level number. When the level starts, a screenshot is saved to the screenshots folder and the game quits.
* `--screenshot-level` -- Similar to the above, except the whole level is screenshotted, thus creating a level map.
* `--screenshot-level-extras` -- Similar to the above, except lots of additional info is displayed on the picture. You can find the meaning of each symbol in `Map_Symbols.txt`.
* `mute` -- Start the game with sound off. (You can still enable sound with Ctrl+S.)
* `playdemo` -- Make the demo level playable. You may want to use it together with options which start the demo level immediately, such as `megahit 0 playdemo` or `record 0 playdemo`.

### Which keys can I use?

#### Controlling the kid

* Left: turn or run left
* Right: turn or run right
* Up: jump or climb up
* Down: crouch or climb down
* Shift: pick up things
* Shift+Left/Right: careful step
* Home or Up+Left: jump left
* Page Up or Up+Right: jump right
* Shift while falling: grab onto ledge

You can also use the numeric keypad.

#### Gamepad equivalents

* D-Pad: arrows
* Joystick: left/right (for all-directional joystick movement, set joystick_only_horizontal to false in SDLPoP.ini)
* A: down
* Y: up
* X or triggers: Shift
* Start or Back: Display in-game menu.

If SDLPoP does not work correctly with your gamepad, it might help if you download gamecontrollerdb.txt and configure SDLPoP to use it. See `SDLPoP.ini` for details.

Since version 1.23 SDLPoP now detects controllers plugged in while the game is already running. (hotplug)

#### Controlling the game

* Esc: Pause game.
* Space: Show how much time is left.
* Ctrl+A: Restart level.
* Ctrl+G: Save game (on levels 3..13).
    * This saves only the level number, the remaining time, and the number of hit points.
* Ctrl+L: Load game (press in the intro).
    * The game will continue from the beginning of the level where you saved.
* Ctrl+J: Joystick/gamepad mode.
* Ctrl+K: Keyboard mode.
    * The initial mode is joystick/gamepad if such a device is detected, otherwise keyboard mode.
    * Since version 1.18, SDLPoP automatically changes the input mode when there is input from either device. As a result, Ctrl+K and Ctrl+J are now redundant.
* Ctrl+R: Return to intro.
* Ctrl+S: Sound on/off.
* Ctrl+V: Show version of SDLPoP.
* Ctrl+Q: Quit game.

**The following don't exist in the original game:**

* Ctrl+C: Show versions of SDL:
    * COMP: the SDL version SDLPoP was compiled against, i.e. the version of the SDL headers.
    * LINK: the SDL version SDLPoP was linked against, i.e. the version of SDL2.dll (or its equivalent on other platforms).
* Alt+Enter: Toggle full-screen mode.
* F6: Quicksave: Save the exact state of the game.
* F9: Quickload: Load what the last quicksave saved.
* F12: Save a screenshot to the screenshots folder.
* Backspace: Display the in-game menu. (Esc will also display the menu by default, but you can turn that off.)
* `: Fast forward. (It's the key above Tab. It might have a different label depending on your keyboard layout.)

#### Viewing or recording replays:

**Replays don't exist in the original game.**

* Ctrl+Tab (in game, or on title screen): Start or stop recording.
* Tab (on title screen): View/cycle through the saved replays in the SDLPoP directory.
* F (while viewing a replay): Skip forward to the next room.
* Shift+F (while viewing a replay): Skip forward to the next level.

### Cheats:

* Shift+L: Go to next level.
* C: Show numbers of current and adjacent rooms.
* Shift+C: Show numbers of diagonally adjacent rooms.
* -: Decrease remaining time by one minute.
* +: Increase remaining time by one minute.
* R: Resurrect kid.
* K: Kill guard.
* Shift+I: Flip the screen upside down.
* Shift+W: Slow falling.
* H: Look at the room to the left.
* J: Look at the room to the right.
* U: Look at the room above.
* N: Look at the room below.
* Shift+B: Toggle hiding of non-animated objects. (Also known as "blind mode".)
* Shift+S: Restore a lost hit-point. (Like a small red potion.)
* Shift+T: Give more hit-points. (Like a big red potion.)

**The following don't exist in the original game:**

* Ctrl+B: Go back to the room where the prince is. (Undo H,J,U,N.)
* Shift+F12: Save a screenshot of the whole level to the screenshots folder, thus creating a level map.
* Ctrl+Shift+F12: Save a screenshot of the whole level with extras to the screenshots folder. You can find the meaning of each symbol in Map_Symbols.txt.

#### Debug cheats

**These don't exist in the original game.**

* [: Shift kid 1 pixel to the left.
* ]: Shift kid 1 pixel to the right.
* T: Toggle display of timer (remaining minutes:seconds:ticks). Also shows the total elapsed ticks during playback.
* F: Toggle display of the remaining feather fall time. (Only if "Fix quick save in feather mode" is enabled.)
* Shift+F9: Quickload but keep the currently loaded level.

Intended use of Shift+F9:

* Suppose you made a quicksave after you got the prince or a guard into a specific position needed for a trick.
* Then you try to do the trick, but you realize that you need to change the level slightly to make the trick work. So you edit the level.
* But you can't use a (regular) quickload to get back to the saved position, because that would load the previous version of the level from the quicksave file.
* In this situation, press Ctrl+A to load the new version of the level, then press Shift+F9 to load the quicksave onto this new level.
* Motivation: https://forum.princed.org/viewtopic.php?p=32556#p32556

#### Where is the music?

Since version 1.13, the game supports loading music from the data/music folder.

Until 1.15, music was not included in releases because it is very big, and it does not change between SDLPoP versions.
You need to get the music from here: (38 MB)
https://www.popot.org/get_the_games/various/PoP1_DOS_music.zip
It's the last link here: https://www.popot.org/get_the_games.php?game=1
Copy the OGG files to the data/music folder.

Since version 1.15, music is included.

Since version 1.18, SDLPoP can play music from the MIDISND*.DAT files and OGG files are not included.

#### Using custom music

You can still copy OGG files to `data/music` or `mods/<modname>/music` to override music for the original game or a mod, respectively.
To use the music in `data/music` for all mods as well, enable the `always_use_original_music` option in `SDLPoP.ini`.

The possible file names can be found in `data/music/names.txt`.
The file name has to be the part after the equals sign, followed by `.ogg`.

You can find an example alternate music set in the ZIP mentioned above, in the `ogg_MT-32` folder.

## MODS

### Can I play mods?

Since version 1.02, the game supports `LEVELS.DAT`, and since version 1.03, the game can use all .DAT files.
You can either copy the modified .DAT files to the folder of the game, or the game to the mod's folder.

Since version 1.17, the game can also load from mod folders that have been placed in the "mods/" directory.
If you use this method, only the files different from the original V1.0 data are required in the mod's folder.

To choose which mod to play from the `mods/` folder, do one of the following:

* Open SDLPoP.ini and change the 'levelset' option to the name of the mod's folder.
* Use the command line option "mod", like so: `prince mod "Mod Name"`

Hall-of-Fame and saved game files will also be placed in the mod's folder.

Another way to play a mod is to start the game while the current directory is the mod's directory.
You can do this from the command line, or with batch files / shell scripts.
This is useful if you want to compare the behavior of this port and the original DOS version (to find bugs).
Especially if you're editing the level and don't want to copy LEVELS.DAT from one place to the other.

* As of 1.03, the data/font folder and its contents must exist in the current directory!
* Since 1.11, the data/font folder is no longer required.

Since version 1.19, SDLPoP can recognize most changes made with CusPoP in a DOS mod's PRINCE.EXE.
Since version 1.16, you can configure some options in SDLPoP.ini: starting time, level types, etc.
In addition, since version 1.17, mods in the "mods/" folder can use a custom configuration file "mod.ini".
Options in this file can override (most of) the gameplay-related options in SDLPoP.ini.

Beware, some mods (especially the harder ones) might rely on bugs that are fixed in SDLPoP.

* You can choose whether gameplay quirks should be fixed or not in the file 'SDLPoP.ini':
    - Set the option 'use_fixes_and_enhancements' to 'false' to get the exact behavior of the original game.
    - Alternatively, set the option 'use_fixes_and_enhancements' to 'true'. You can then also enable or disable
      individual fixes and enhancements, depending on your preference.
* You can also enable or disable gameplay fixes through the in-game menu.
    - In the settings menu, look for the option "Enhanced mode (allow bug fixes)" in the GAMEPLAY section.

Furthermore, SDLPoP opens up new possibilities for mod making. For example:

* Falcury released a mod, called "Secrets of the Citadel" that "has been designed to be played using a modified version of SDLPoP".
* Description and download: https://forum.princed.org/viewtopic.php?f=73&t=3664
* Alternate link: https://www.popot.org/custom_levels.php?mod=0000153

### Additional tiles

See [doc/tiles.md](doc/tiles.md) for documentation on tiles added by SDLPoP.

* Since version 1.16, SDLPoP supports fake tiles, for example walls that the prince can go through.
The [Apoplexy level editor](https://www.popot.org/level_editors.php?editor=apoplexy) supports these additional tiles since v3.0.

* Since version 1.19, SDLPoP supports colored torches.
(Supported in Apoplexy since v3.5.)

* Since version 1.23, SDLPoP supports teleports.
(Supported in Apoplexy since v3.16.)

## REPLAYS

**Replays don't exist in the original game.**

#### How do replays work?

Starting from version 1.16, you can capture or view replays in SDLPoP.
To start recording, press Ctrl+Tab on the title screen or while in game. To stop recording, press Ctrl+Tab again.
Your replays get saved in the "replays/" directory as files with a .P1R extension.
You can change the location where replays are kept using the setting 'replays_folder' in SDLPoP.ini.

If you want to start recording on a specific level, you can use the command `prince record <lvl_number>`,
where <lvl_number> is the level on which you want to start.

To view a replay, you can press Tab while on the title screen.
To cycle to the next replay (in reverse creation order), press Tab again.
You can also double-click on a replay file (and tell the OS that the file needs to be opened with the SDLPoP executable).
SDLPoP will then immediately play that replay. Dragging and dropping onto the executable also works.

While viewing a replay, you can press F to skip forward to the next room, or Shift+F to skip to the next level.

Your settings specified in SDLPoP.ini (including whether you are playing with bugfixes on or off) are remembered in the replay.
It shouldn't matter how SDLPoP.ini is set up when you are viewing the replay later.
Note that any cheats you use do not get saved as part of the replay.

To print out information about the replay from the command-line, you can use the 'validate' command-line parameter.
Example usage: `prince validate "replays/replay.p1r"`

Since version 1.21 you can re-record if you make a mistake:
While recording, make a quicksave to mark your place, and press quickload to return to that place.

## COMPILING

### Prerequisites for all platforms

* Make sure that you have the development versions of the `SDL2` and `SDL2_image` libraries installed.
* See per-platform installation instructions below.

### Windows

#### Dev-C++

* Use this version of Dev-C++: https://sourceforge.net/projects/orwelldevcpp/

You need the MinGW Development Libraries of SDL2:

* https://github.com/libsdl-org/SDL/releases/latest
* https://github.com/libsdl-org/SDL_image/releases/latest
* Download the `*-mingw.tar.gz` files.

To install these, just extract the contents of the `i686-w64-mingw32` folder from each archive to:

* on 64-bit Windows: `c:\Program Files (x86)\Dev-Cpp\MinGW64\`
* on 32-bit Windows: `c:\Program Files\Dev-Cpp\MinGW64\`

You need to "merge" the contents of the `bin`, `include`, etc. folders in the archives into the existing folders with the same name in the `MinGW64` folder.

To compile, open one of the .dev files and click the compile icon.

#### Visual Studio

* Run `build.bat` in the `src/` directory.
* For this to work, you first need to do two other things:
    1. Run vsvarsall.bat from the command line, with either 'x86' or 'x64' as a parameter.
       This batch file is included with all installations of MS Visual Studio, but its exact location may vary.
       For VS2017, the command you should run might look like this:
       ```
       call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x86
       ```
       This step sets up various environment variables, to enable running the compiler from the command line.
    2. Set up the environment variable 'SDL2' to point to the SDL2 development library files.
       To do this, you can use a command like so:
       ```
       set "SDL2=C:\libraries\SDL2-2.0.8"
       ```
       You can get the SDL2 library files from here:
       * https://github.com/libsdl-org/SDL/releases/latest
       * https://github.com/libsdl-org/SDL_image/releases/latest
       * Download the Visual C++ 32/64-bit development package, the `*-VC.zip` files.
    * (You could create a small batch file to automate the above steps on your system.)
* Alternatively, you can also build SDLPoP using MSVC with NMake (use the makefile src/NMakefile).

#### CMake

* You can also use CMake, in conjunction with the MinGW-w64 toolchain.
    * You could either invoke CMake from the command line yourself, or use an IDE that uses CMake internally.
    * As an example, CLion uses CMake as its project model.
    * If you are using CLion as your IDE, you can simply load the src/ directory as a project.

### GNU/Linux

#### Install SDL2 + SDL2 image

##### Arch Linux

    pacman -S sdl2_image

##### Debian/Ubuntu

    apt-get install libsdl2-image-dev

##### Manually

I recommend this if your distro does not have the newest SDL version, because sound may be garbled in SDL versions older than 2.0.4 if the sound output is not 8-bit.

Download the SDL2 and SDL2 image sources:

* SDL2: https://libsdl.org/download-2.0.php
* SDL2 image: https://libsdl.org/projects/SDL_image/

Build and install these libraries according to their documentation.

Then build the game:

    make all

Create a desktop/menu icon with:

    sudo make install

Remove it with:

    sudo make uninstall

### macOS

#### Install SDL2 + SDL2 image

##### With Homebrew

    brew install sdl2 sdl2_image pkg-config

##### With Port

Install "port" from https://www.macports.org/, then:

    sudo port install libsdl2 libsdl2_image pkgconfig

#### Install development tools

1. Install Xcode.
2. Install the "command line developer tools" by typing `xcode-select --install` at the prompt.
3. Using terminal, in the '/src' directory of SDLPoP, type: `make`

#### Start the game

Tested on OSX 10.9.5, OSX 10.11.2, macOS 10.13 and 10.14.

1. In the project root directory. Type `./prince` or `./prince full`.
2. Hit Ctrl+Q to quit.
