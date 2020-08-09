# SDLPoP
An open-source port of Prince of Persia, based on the disassembly of the DOS version.

More info: doc/Readme.txt


Thanks to (in no special order):
NagyD for SDLPoP port "big-endian" branch (https://github.com/NagyD/SDLPoP/tree/big-endian).
BeWorld for its morphos port, so I knew SDLPoP existed.
Capehill et all for SDL2 AmigaOS4 port.
salass00, kas1e, samo79, guillaume (code and tips).
os4coding.net


Q: Where is the music? (snippet from Readme.txt)
A:
Since version 1.13, the game supports loading music from the data/music folder.
Until 1.15, music was not included in releases because it is very big, and it does not change between SDLPoP versions.
You need to get the music from here: (38 MB)
	https://www.popot.org/get_the_games/various/PoP1_DOS_music.zip
It's the last link here: https://www.popot.org/get_the_games.php?game=1
Copy the OGG files to the data/music folder.

Since version 1.15, music is included.

Since version 1.18, SDLPoP can play music from the MIDISND*.DAT files and OGG files are not included.


HOW TO USE/LOAD CUSTOM LEVELS
*****************************
In this link you can find lot of custom levels for Prince of Persia:
https://www.popot.org/custom_levels.php

Just unpack inside mods/ drawer AND if it has a <FILENAME>.EXE, remove it.
Then in SDLPoP.ini change "levelset = original" with "levelset = _custom_level_drawername_"

