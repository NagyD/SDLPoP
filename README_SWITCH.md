# SDLPoP on Nintendo Switch

This branch adds an experimental Nintendo Switch build of SDLPoP, using devkitA64, libnx and the SDL2 portlibs. The changes are small and are guarded by `__SWITCH__`, so desktop builds are not affected.

## What is included

- `Makefile.switch`: libnx build file (produces `SDLPoP.nro`).
- `src/switch_main.c`: Switch entry point. It checks the applet mode and the game data, then starts the game from `sdmc:/switch/SDLPoP/`.
- `src/main.c`: on Switch the game entry point is `sdlpop_main()`, called from `switch_main.c`.

The game data is NOT part of this repository and is NOT distributed with it.

## Requirements

- devkitPro with devkitA64 and libnx (`DEVKITPRO` must be set).
- SDL2, SDL2_image (PNG support), zlib and libpng built for Switch, in the devkitPro portlibs (`PORTLIBS`).
- A copy of the game data that you obtained legally (for example, the original MS-DOS release or a copy you own). Get the data from your own legal copy; this project does not provide it.

## Build

    export DEVKITPRO=/opt/devkitpro
    make -f Makefile.switch

The output is `SDLPoP.nro`.

If SDL2_image is not in the devkitPro portlibs, build it (PNG support is enough) into a local prefix and pass it with `POP_PREFIX=/path/to/prefix`. The path must not contain spaces because the libnx makefiles do not quote it.

## Install on the console

Put these in `sdmc:/switch/SDLPoP/` on the microSD card:

- `SDLPoP.nro`
- `SDLPoP.ini` (optional; the game uses its defaults without it)
- `data/` with the game data files (`PRINCE.DAT`, `KID.DAT`, `GUARD.DAT`, `FAT.DAT`, `SKEL.DAT`, `SHADOW.DAT`, `VIZIER.DAT`, `PV.DAT`, `VDUNGEON.DAT`, `VPALACE.DAT`, `TITLE.DAT`, `light.png`) and the `font/` folder.

Start the `.nro` from hbmenu. If the data is missing, the launcher lists the missing files and waits for the + button.

## Controls

The game uses the default SDLPoP controller mapping. Pad buttons are read through SDL's game controller API. Check the controller mapping in the source before relying on it on a specific device; no remapping is provided in this branch.

## Known limits

- Tested on hardware: not yet on this branch. It was developed and tested by the author on a Nintendo Switch OLED (HOS 22.1.0, Atmosphère 1.11.1) with a build that includes extra local changes not present in this branch.
- Audio and saves have not been verified on Switch in this branch.
- Quitting from the pause menu follows the normal SDL path; a hang on exit was seen in one local test, not fixed here.
- Applet mode (Album) is not supported: start the game from hbmenu.
