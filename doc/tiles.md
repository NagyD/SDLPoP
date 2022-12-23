# Tile+modifier combinations

In Prince of Persia, each tile is defined by a tile type (`level.fg[...]`) and a modifier byte (`level.bg[...]`).

SDLPoP supports the tile+modifier combinations listed below, in addition to what the original game supports.

(The headings show the value of the `fg` byte, the list items show the value of the `bg` byte.)

See also: https://www.princed.org/wiki/Legacy_combos \
That page includes the combinations supported by the original game and also the native tiles added by SDLPoP and MININIM.

*(TODO: Maybe document the original combinations in this file as well?)*

## Fake tiles
(See `USE_FAKE_TILES` in `seg008.c`.)

### 0x00: empty
*  4 = 0x04: display a fake floor with modifier = 0
*  5 = 0x05: display a fake wall with modifier = 0
* 12 = 0x0C: display a fake floor with modifier = 1
* 13 = 0x0D: display a fake wall with modifier = 1
* 50 = 0x32: display a fake wall (pattern: no walls left or right)
* 51 = 0x33: display a fake wall (pattern: wall only to the right)
* 52 = 0x34: display a fake wall (pattern: wall only to the left)
* 53 = 0x35: display a fake wall (pattern: wall on both sides)

### 0x01: floor
*  5 = 0x05: display a fake wall with modifier = 0
*  6 = 0x06: display nothing (invisible floor) with modifier = 0
* 13 = 0x0D: display a fake wall with modifier = 1
* 14 = 0x0E: display nothing (invisible floor) with modifier = 1
* 50 = 0x32: display a fake wall (pattern: no walls left or right)
* 51 = 0x33: display a fake wall (pattern: wall only to the right)
* 52 = 0x34: display a fake wall (pattern: wall only to the left)
* 53 = 0x35: display a fake wall (pattern: wall on both sides)

### 0x14: wall
*  4 = 0x04: display a floor (invisible wall) with modifier = 0
*  6 = 0x06: display empty tile (invisible wall) with modifier = 0
* 12 = 0x0C: display a floor (invisible wall) with modifier = 1
* 14 = 0x0E: display empty tile (invisible wall) with modifier = 1

## Colored torches
(See `USE_COLORED_TORCHES`.)

### 0x13: torch and 0x1E: torch with debris

* 0x00: Use the default colors.
* 0x01-0x3F: Use a 6-bit RGB color for the flame.
	* bits 0-1: blue (0-3, respectively +0, +1, +2, +3)
	* bits 2-3: green (0-3, respectively +0, +4, +8, +12)
	* bits 4-5: red (0-3, respectively +0, +16, +32, +48)

## Teleports
(See `USE_TELEPORTS`.)

### 0x17: balcony left
* any non-zero modifier: teleport left half, takes to another balcony left with the same modifier.

### 0x18: balcony right
* 1 = 0x01: teleport right half
