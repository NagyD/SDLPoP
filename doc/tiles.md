# Tile+modifier combinations

In Prince of Persia, each tile is defined by a tile type (`level.fg[...]`) and a modifier byte (`level.bg[...]`).

SDLPoP supports the tile+modifier combinations listed below, in addition to what the original game supports.

(The headings show the value of the `fg` byte, the list items show the value of the `bg` byte.)

See also: https://www.princed.org/wiki/Legacy_combos \
That page includes the combinations supported by the original game and also the fake tiles.

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

## Teleports
(See `USE_TELEPORTS`.)

### 0x17: balcony left
* any non-zero modifier: teleport left half, takes to another balcony left with the same modifier.

### 0x18: balcony right
* 1 = 0x01: teleport right half
