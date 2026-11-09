# SDLPoP — Game Boy Advance port (devkitPro/devkitARM)

This is a port of [SDLPoP](https://github.com/NagyD/SDLPoP) (Prince of Persia)
to the Nintendo Game Boy Advance, using **devkitARM** + **libgba** +
**maxmod**.

It is built from the upstream sources with:

- a **SDL2 compatibility shim** (`gba/include/SDL2/SDL.h`,
  `gba/src/sdl_shim.c`) covering the surface, palette, rwops, event, timing
  and audio APIs that SDLPoP touches;
- **direct hardware drivers** for Mode-4 video (`gba_video.c`), DirectSound A
  PCM audio (`gba_audio.c`), buttons (`gba_input.c`), 60 Hz VBlank ticking
  (`gba_time.c`), 32 KB SRAM saves (`gba_save.c`), and ROM-resident data
  blobs (`gba_file.c`);
- **`#ifndef __GBA__`** guards around a handful of subsystems that won't fit
  on a 16.78 MHz ARM7 with 256 KB EWRAM (OPL3, MIDI, stb_vorbis, lighting,
  screenshot, menu, replay, quicksave, fade, fast-forward, debug cheats,
  dark transition, teleports).

The unmodified upstream source files in `../src` compile as-is once
`__GBA__` is defined and the SDL include path is the shim.

## Limitations vs. desktop SDLPoP

- **Resolution**: 240×160 (GBA hardware) vs. 320×200 (original). A camera
  scroll keeps the prince in view. The original game world still runs at
  320×192 internally.
- **Audio**: digitized SFX only (DIGISND*.DAT), played as 8-bit PCM via
  DirectSound A on DMA1 at 18 157 Hz. Adlib music, PC-speaker beeps, MIDI
  and OGG music are **dropped**. You can optionally prerender music as
  raw 8-bit signed PCM and feed it via `gba_music_play()` from a mod.
- **Save**: the original `PRINCE.SAV` file is redirected to cartridge SRAM
  (32 KB). Emulators persist this as `SDLPoP_gba.sav`.
- **No menu / replay / screenshot / lighting / fade / fast-forward / debug
  cheats**: dropped to save code size and RAM.
- **Hall of Fame**: writes to SRAM (same blob namespace as save).

## Build prerequisites

1. **devkitPro** with the `gba-dev` group:
   ```bash
   # macOS (with Homebrew):
   brew install --cask devkitpro/devkitpro/devkitpro-pacman
   sudo dkp-pacman -S gba-dev
   # Linux: see https://devkitpro.org/wiki/Getting_Started
   ```
2. Source the devkitARM environment:
   ```bash
   source /opt/devkitpro/devkitarm.sh   # or wherever you installed it
   ```
3. Have a copy of the **Prince of Persia v1.0/1.1 DOS data files**
   (`KID`, `PRINCE`, `TITLE`, `LEVELS`, `GUARD*`, `V*`, `DIGISND*.DAT`,
   etc.). The SDLPoP source ships them in `../data/`.

## Build

```bash
cd gba
./tools/prep_data.sh     # copies ../data/* -> data/*.bin
make
```

Produces `SDLPoP_gba.gba`, ready to run in any GBA emulator (mGBA, no$gba,
VBA-M) or to flash to a flashcart.

## Layout

```
gba/
├── Makefile               # devkitARM build
├── README.md              # this file
├── include/
│   ├── SDL2/SDL.h         # minimal SDL2 shim header (used in place of real SDL2)
│   ├── SDL2/SDL_image.h   # IMG_Load returns NULL
│   └── gba_port.h         # cross-module bindings
├── src/
│   ├── sdl_shim.c         # SDL_Surface, blits, palette, RWops, audio, timing
│   ├── gba_main.c         # entry point (main): init libgba, call pop_main()
│   ├── gba_video.c        # Mode-4 framebuffer + viewport panning
│   ├── gba_audio.c        # DMA1 8-bit PCM mixer (6 voices)
│   ├── gba_sound_bridge.c # overrides SDLPoP's play_sound_from_buffer
│   ├── gba_input.c        # KEYINPUT -> SDL scancodes
│   ├── gba_time.c         # VBlank tick counter + SDLPoP frame timer
│   ├── gba_file.c         # fopen/fread over linked DAT blobs, save -> SRAM
│   ├── gba_save.c         # SRAM read/write + Fletcher-32 checksum
│   └── gba_viewport.c     # camera follows the prince
├── tools/
│   └── prep_data.sh       # copies ../data/* to data/*.bin
└── data/                  # populated by prep_data.sh
```

## Source modifications

Three upstream files have **small** `__GBA__`-gated additions:

- `src/common.h` — preprocessor macros for `fopen`/`fread`/`stat` (re-route
  to `gba_fopen` etc).
- `src/config.h` — disable `USE_LIGHTING`, `USE_SCREENSHOT`, `USE_MENU`,
  `USE_REPLAY`, `USE_QUICKSAVE`, `USE_FADE`, `USE_DARK_TRANSITION`,
  `USE_TELEPORTS`, `USE_AUTO_INPUT_MODE`, `USE_FAST_FORWARD`,
  `USE_DEBUG_CHEATS` on `__GBA__`. The fixes are also gated by
  `DISABLE_ALL_FIXES`.
- `src/seg009.c` — wraps the SDL audio implementation (`#ifndef __GBA__`)
  and the SDL_Renderer-based `set_gr_mode`/`update_screen` with an `#else`
  branch that creates an 8 bpp paletted surface and feeds it to
  `gba_present_surface()`.

The other ~33 000 lines of upstream source compile unmodified.

## Controls

| GBA button | SDL scancode | Action                |
|------------|--------------|-----------------------|
| D-Pad      | Arrow keys   | Move / jump / crouch  |
| A          | Shift        | Grab / draw sword     |
| B          | Space        | (unused / pause)      |
| L          | Ctrl         | Chop / fight strike   |
| R          | Alt          | Modifier              |
| Start      | Enter        | Advance dialog        |
| Select     | Backspace    | Cancel                |

## What is NOT yet wired

- **Music**: only SFX is fed to the mixer. To add music, place a raw 8-bit
  signed PCM stream as `data/music.bin` and call
  `gba_music_play(music_bin, music_bin_size, 1)` at the start of each
  level (see `gba_audio.c`). A music conversion pipeline isn't included.
- **Sound resampling**: digitized SFX is played at the rate stored in the
  DIGISND header; the mixer plays at a fixed 18 157 Hz, so pitch is
  approximate. To improve, add a nearest-neighbour resampler in
  `play_digi_sound()` in `gba_sound_bridge.c`.
- **The `chtab` sprite cache** is loaded into EWRAM on demand by upstream
  code. With KID + PRINCE + GUARD + VPALACE + VDUNGEON simultaneously
  active you can exceed 256 KB; if you hit OOM crashes, see
  `load_chtab_from_file` in `seg009.c` and consider releasing earlier
  chtabs or moving them into IWRAM/ROM.
- **Lighting**, **fade**, **dark room transition** — disabled. If you want
  fade-in/out, re-enable `USE_FADE` and live with a slower CPU budget.

## Troubleshooting

- `gba.h: file not found` from your IDE — your editor is finding the host
  SDL2/clang headers, not devkitARM's. Build with `make`, not the IDE.
- `undefined reference to '_kid_bin_size'` at link time — you didn't run
  `./tools/prep_data.sh`, so the linker has no blobs to attach. Run it
  and rebuild.
- The ROM boots to a black screen — typical first-run hang because some
  sprite chtab failed to load. Try with the original `data/` files only,
  no mods.
- Saves don't persist in your emulator — make sure the emulator is
  configured to write `.sav` files alongside the ROM (mGBA does by
  default; some emulators need `--sram`).
