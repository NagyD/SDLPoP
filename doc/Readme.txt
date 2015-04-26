Name of program: SDLPoP
(Earlier name: David's open-source port of PoP)
Author: David from forum.princed.org
Topic in forum: http://forum.princed.org/viewtopic.php?f=69&t=3512

=== GENERAL

Q: What is this?
A: This is an open-source port/conversion of the DOS game Prince of Persia.
It is based on the disassembly of the original PoP1 for DOS.

=== USAGE

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
megahit -- Enable cheats.
a number from 1 to 14 -- Start the given level.
draw -- Draw directly to the screen, skipping the offscreen buffer.
full -- Run in full screen mode.

Q: What keys can I use?
A:
Controlling the kid:
left: turn or go left
right: turn or go right
up: jump or climb up
down: crouch or climb down
shift: pick up things
shift+left/right: careful step
home or up-left: jump left
page up or up-right: jump right
You can also use the numeric keypad.

Controlling the game:
Esc: pause game
space: show time left
Ctrl-A: restart level
Ctrl-G: save game (on levels 3..13)
Ctrl-J: joystick mode (not implemented)
Ctrl-K: keyboard mode
Ctrl-R: return to intro
Ctrl-S: sound on/off
Ctrl-V: show version
Ctrl-Q: quit game

Cheats:
Shift-L: go to next level
c: show numbers of current and adjacent rooms
Shift-C: show numbers of diagonally adjacent rooms
-: less remaining time
+: more remaining time
r: resurrect kid
k: kill guard
Shift-I: flip screen upside-down
Shift-W: slow falling
h: look at room to the left
j: look at room to the right
u: look at room above
n: look at room below
Shift-B: toggle hiding of non-animated objects
Shift-S: Restore lost hit-point. (Like a small red potion.)
Shift-T: Give more hit-points. (Like a big red potion.)

Q: Where is the music?
A:
Since version 1.13, the game supports loading music from the data/music folder.
The music is not included in releases because it is very big, and it does not change between versions.
You need to get the music from here: (38MB)
	http://www.popot.org/get_the_games/various/PoP1_DOS_music.zip
It's the last link here: http://www.popot.org/get_the_games.php?game=1
Copy the ogg files to the data/music folder. (The mp3 and flac files don't seem to work.)

=== MODS

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

=== DEVELOPING

Q: How do I (re)compile it?
A:
Prerequisites for all platforms:
	Make sure that you have the development versions of the "SDL", "SDL_image" and "SDL_mixer" (since 1.13) libraries installed.
	(These in turn require the "libjpeg", "libpng" and "zlib" libraries.)

Windows:
	If you are using Dev-C++:
		I use Dev-C++ version 4.9.9.2 from here: http://sourceforge.net/projects/dev-cpp/files/Binaries/
		You can download the libraries from: http://sourceforge.net/projects/devpaks/files/
		I used these files:
			libjpeg-6b_4-1spec.DevPak
			libpng-1.2.7-1spec.DevPak
			SDL_image-1.2.4notiff.DevPak
			SDL-1.2.8-2spec.DevPak
			zlib-1.2.3-1cm.DevPak
			SDL_mixer-1.2.6-2mol.DevPak
		You can install the libraries at Tools -> Package Manager.
		Open one of the .dev files and click the compile icon.

GNU/Linux:
	The libraries can be installed with apt-get or a package manager.
		sudo apt-get install libsdl-image1.2-dev libsdl-mixer1.2-dev
	Just type the command:
		make all
	and the game should compile.

Mac OS X:
	See the MAC_INSTALL.txt written by StaticReturn.
