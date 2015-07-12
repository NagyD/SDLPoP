/*
SDLPoP, a port/conversion of the DOS game Prince of Persia.
Copyright (C) 2013-2015  Dávid Nagy

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

#include "common.h"

#define SEQTBL_BASE 0x196E
#define SEQTBL_0 (seqtbl - SEQTBL_BASE)
// data:196E
const byte seqtbl[] = {0xF9,0x01,0xFF,0x81,0x19,0xF9,0x01,0x01,0x02,0x03,0x04,0xFB,0x08,0x05,0xFB,0x03,0x06,0xFB,0x03,0x07,0xFB,0x05,0x08,0xFB,0x01,0xF2,0x01,0x09,0xFB,0x02,0x0A,0xFB,0x04,0x0B,0xFB,0x05,0x0C,0xFB,0x02,0xF2,0x01,0x0D,0xFB,0x03,0x0E,0xFB,0x04,0xFF,0x81,0x19,0xF9,0x00,0x0F,0xFF,0xA0,0x19,0xF9,0x01,0xA6,0xFF,0xA8,0x19,0xF9,0x05,0xFB,0x0A,0xB1,0xB1,0xFB,0xF9,0xFA,0xFE,0xB2,0xFB,0x05,0xFA,0x02,0xA6,0xFB,0xFF,0xFF,0xD2,0x19,0xFF,0xD2,0x19,0xF9,0x01,0xFB,0x02,0xCF,0xD0,0xFB,0x02,0xD1,0xFB,0x02,0xD2,0xFB,0x03,0xF9,0x01,0xF2,0x00,0x9E,0xAA,0xAB,0xFF,0xD8,0x19,0xF9,0x05,0xF8,0xFF,0x00,0xAC,0xFB,0xFF,0xFA,0x01,0xAD,0xFB,0xFF,0xAE,0xFB,0xFF,0xFA,0x02,0xFB,0xFE,0xFA,0x01,0xFB,0xFB,0xFA,0xFC,0xFF,0x4D,0x1A,0xF9,0x01,0xF8,0x01,0x00,0x9B,0xFB,0x02,0xA5,0xFB,0xFE,0xFF,0xD2,0x19,0xF9,0x01,0xF8,0xFF,0x00,0x9B,0x9C,0x9D,0x9E,0xFF,0x2E,0x1A,0xF9,0x01,0xF8,0x01,0x00,0xFB,0x02,0xA3,0xFB,0x04,0xA4,0xA5,0xFF,0xD2,0x19,0xF9,0x01,0xF8,0x01,0x00,0xFB,0x06,0xA4,0xA5,0xFF,0xD2,0x19,0xF9,0x01,0xF8,0xFF,0x00,0xFB,0xFD,0xA0,0xFB,0xFE,0x9D,0xFF,0xD2,0x19,0xF9,0x01,0xF8,0xFF,0x00,0xA8,0xF9,0x01,0x97,0xF9,0x01,0x98,0x99,0x9A,0xF9,0x05,0x9B,0xF9,0x01,0x9C,0x9D,0xFF,0xD2,0x19,0xF9,0x01,0xA7,0xFF,0x4A,0x1A,0xA2,0xFF,0x45,0x1A,0xA9,0x96,0xFF,0xD2,0x19,0x9F,0xA0,0xFF,0x5F,0x1A,0xF9,0x01,0xF4,0xFF,0xD2,0x19,0xF9,0x05,0xFB,0xF8,0xFF,0xD2,0x19,0xF9,0x05,0xA0,0x9D,0xFF,0xD2,0x19,0xF9,0x07,0xFB,0xF8,0xFF,0x39,0x1B,0xF9,0x05,0xFE,0xFB,0x05,0xFF,0x2E,0x1A,0xF9,0x05,0xFE,0xFB,0x12,0xFF,0xA6,0x19,0xF9,0x01,0x10,0x11,0xFB,0x02,0x12,0xFB,0x02,0x13,0xFB,0x02,0x14,0xFB,0x02,0x15,0xFB,0x02,0x16,0xFB,0x07,0x17,0xFB,0x09,0x18,0xFB,0x05,0xFA,0xFA,0x19,0xFB,0x01,0xFA,0x06,0x1A,0xFB,0x04,0xF4,0xF2,0x01,0x1B,0xFB,0xFD,0x1C,0xFB,0x05,0x1D,0xF2,0x01,0x1E,0x1F,0x20,0x21,0xFB,0x01,0xFF,0xA0,0x19,0xF9,0x01,0xF2,0x01,0x22,0xFB,0x05,0x23,0xFB,0x06,0x24,0xFB,0x03,0x25,0xFB,0x05,0xF2,0x01,0x26,0xFB,0x07,0x27,0xFB,0x0C,0xFA,0xFD,0x28,0xFB,0x08,0xFA,0xF7,0x29,0xFB,0x08,0xFA,0xFE,0x2A,0xFB,0x04,0xFA,0x0B,0x2B,0xFB,0x04,0xFA,0x03,0x2C,0xFB,0x05,0xF4,0xF2,0x01,0xFF,0x81,0x19,0xF9,0x01,0xFB,0x01,0x6B,0xFB,0x02,0xFB,0x02,0x6C,0xFB,0x02,0x6D,0xFB,0x02,0x6D,0xFB,0x02,0x6D,0xFF,0x16,0x1B,0x00,0xF9,0x01,0xFB,0x01,0x6E,0x6F,0xFB,0x02,0x70,0xFB,0x02,0x6C,0xFB,0x02,0x6D,0xFF,0x29,0x1B,0xF9,0x07,0xFE,0xFB,0x06,0x2D,0xFB,0x01,0x2E,0xFF,0xC4,0x19,0xF9,0x07,0xFE,0xFB,0x06,0x2D,0xFB,0x01,0x2E,0xFB,0x02,0x2F,0xFB,0xFF,0x30,0xFB,0x01,0x31,0xFB,0xFE,0x32,0x33,0x34,0xFF,0xA0,0x19,0xF9,0x01,0xFB,0xFF,0xFF,0x75,0x19,0xF9,0x01,0xFB,0x01,0x35,0xFB,0x01,0xF2,0x01,0x36,0xFB,0x08,0x37,0xF2,0x01,0x38,0xFB,0x07,0x39,0xFB,0x03,0x3A,0xFB,0x01,0x3B,0x3C,0xFB,0x02,0x3D,0xFB,0xFF,0x3E,0x3F,0x40,0xFB,0xFF,0x41,0xFB,0xF2,0xFE,0xFF,0x95,0x19,0xF9,0x03,0xFA,0xFF,0x66,0xFB,0xFE,0xFA,0x06,0x67,0xFB,0xFE,0xFA,0x09,0x68,0xFB,0xFF,0xFA,0x0C,0x69,0xFB,0xFD,0xF8,0x00,0x0F,0xFF,0x49,0x1D,0xF9,0x03,0xFA,0xFF,0xFB,0xFE,0x66,0xFB,0xFD,0xFA,0x06,0x67,0xFB,0xFD,0xFA,0x09,0x68,0xFB,0xFE,0xFA,0x0C,0x69,0xFB,0xFD,0xF8,0x00,0x0F,0xFF,0x49,0x1D,0xF9,0x03,0xFB,0x01,0xFA,0xFF,0x66,0xFB,0x02,0xFA,0x06,0x67,0xFB,0xFF,0xFA,0x09,0x68,0xFA,0x0C,0x69,0xFB,0xFE,0xF8,0x01,0x0F,0xFF,0x49,0x1D,0xF9,0x03,0xFB,0x01,0xFA,0x03,0xF7,0x06,0x1C,0x66,0xFB,0x02,0xFA,0x06,0x67,0xFB,0xFF,0xFA,0x09,0x68,0xFA,0x0C,0x69,0xFB,0xFE,0xF8,0x01,0x0F,0xFF,0x49,0x1D,0xFB,0xFF,0xFA,0xFD,0xFF,0xE4,0x1B,0xFB,0x01,0xFF,0xDB,0x1B,0x66,0xFB,0x02,0xFA,0x03,0x67,0xFB,0xFF,0xFA,0x04,0x68,0xFA,0x05,0x69,0xFB,0xFE,0xF8,0x01,0x06,0xFF,0x49,0x1D,0xF9,0x03,0xFB,0x01,0xFA,0x03,0x66,0xFB,0x02,0xFA,0x06,0x67,0xFB,0x01,0xFA,0x09,0x68,0xFB,0x02,0xFA,0x0C,0x69,0xF8,0x02,0x0F,0xFF,0x49,0x1D,0xF9,0x03,0xFB,0x01,0xFA,0x03,0x66,0xFB,0x03,0xFA,0x06,0x67,0xFB,0x02,0xFA,0x09,0x68,0xFB,0x03,0xFA,0x0C,0x69,0xF8,0x03,0x0F,0xFF,0x49,0x1D,0xF9,0x01,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0xF9,0x02,0x4E,0x4F,0x50,0xFF,0xA1,0x1C,0xF9,0x01,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0xF9,0x02,0xFB,0x01,0x4E,0xFB,0x02,0x4F,0xFB,0x01,0x50,0xFF,0xA1,0x1C,0xF9,0x01,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0xFB,0xFF,0x4D,0xF9,0x02,0xFB,0xFE,0x4E,0xFB,0xFF,0x4F,0xFB,0xFF,0x50,0xFF,0xA1,0x1C,0xF9,0x02,0x5B,0x5A,0x59,0x58,0x57,0x57,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,0x60,0x61,0x62,0x63,0x61,0x60,0x5F,0x5E,0x5D,0x5C,0x5B,0x5A,0x59,0x58,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,0x60,0x5F,0x5E,0x5D,0x5C,0xFF,0x25,0x1D,0xF9,0x06,0x5C,0x5D,0x5D,0x5C,0x5C,0x5B,0xFF,0xD8,0x1C,0x87,0x88,0x89,0x89,0x8A,0x8A,0x8A,0x8A,0x89,0x88,0x87,0xFB,0xF9,0xFF,0x25,0x1D,0xF9,0x01,0x94,0x91,0x90,0x8F,0x8E,0x8D,0xFB,0xFB,0xFA,0x3F,0xFC,0xF9,0x03,0x8C,0x8A,0x88,0x5B,0xF9,0x02,0xFF,0xA4,0x1C,0xF9,0x01,0x87,0x88,0x89,0x8A,0x8B,0x8C,0xFB,0x05,0xFA,0xC1,0xFD,0x8D,0x8E,0x8F,0x90,0x91,0x92,0x93,0x94,0xF9,0x05,0x95,0xF9,0x01,0x76,0x77,0xFB,0x01,0xFF,0xA0,0x19,0x51,0x52,0xF9,0x05,0x53,0xF9,0x01,0xF4,0xF2,0x00,0x54,0x55,0xFB,0x03,0xFF,0xA0,0x19,0xF9,0x03,0x51,0xFA,0x06,0x51,0xFA,0x09,0x51,0xFA,0x0C,0xFB,0x02,0xF8,0x00,0x0C,0xFF,0x49,0x1D,0xF9,0x04,0x6A,0xFF,0x4B,0x1D,0xF9,0x01,0x35,0xFB,0x02,0xF2,0x01,0x36,0xFB,0x07,0x37,0xF2,0x01,0x38,0xFB,0x02,0x31,0xFB,0xFE,0x32,0x33,0x34,0xFF,0xA0,0x19,0xF9,0x01,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0xF9,0x00,0xF5,0x4F,0xFF,0x25,0x1D,0xF9,0x01,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,0xFA,0xFC,0x4F,0xFA,0xFE,0x4F,0x4F,0xFA,0x02,0x4F,0xFA,0x04,0xFF,0x25,0x1D,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0xFA,0xFF,0x4D,0xFA,0xFD,0x4E,0xFA,0xFC,0x4F,0xFA,0xF6,0x4F,0xFA,0xF7,0x4F,0xFA,0xF8,0x4F,0xFA,0xF9,0x4F,0xFA,0xFA,0x4F,0xFA,0xFB,0x4F,0xFA,0xFC,0x4F,0xFA,0xFD,0x4F,0xFA,0xFE,0x4F,0xFA,0xFE,0x4F,0xFA,0xFF,0x4F,0xFA,0xFF,0x4F,0xFA,0xFF,0x4F,0x4F,0x4F,0x4F,0xFA,0x01,0x4F,0xFA,0x01,0x4F,0xFA,0x02,0x4F,0xFA,0x02,0x4F,0xFA,0x03,0x4F,0xFA,0x04,0x4F,0xFA,0x05,0x4F,0xFA,0x06,0x4F,0xF8,0x00,0x06,0xFF,0x49,0x1D,0xF9,0x03,0x50,0xFF,0xA1,0x1C,0xF9,0x05,0xFB,0xFC,0x32,0x33,0x34,0xFF,0xA0,0x19,0xF9,0x05,0xFB,0x01,0xFA,0x03,0xF7,0x25,0x1E,0x66,0xFB,0x02,0xFA,0x06,0x67,0xFB,0xFF,0xFA,0x09,0x68,0xFA,0x0C,0x69,0xFB,0xFE,0xF8,0x00,0x0F,0xFF,0x49,0x1D,0x66,0xFB,0x02,0xFA,0x03,0x67,0xFB,0xFF,0xFA,0x04,0x68,0xFA,0x05,0x69,0xFB,0xFE,0xF8,0x00,0x06,0xFF,0x49,0x1D,0xF9,0x05,0xFB,0xFF,0xFA,0xFC,0x66,0xFB,0xFF,0xFA,0x03,0xFB,0xFD,0xFA,0x01,0xF4,0xFB,0x01,0xF2,0x01,0x6B,0xFB,0x02,0x6C,0xF2,0x01,0x6D,0xFF,0xB3,0x1F,0x79,0xFB,0x01,0x7A,0x7B,0xFB,0x02,0x7C,0xFB,0x04,0x7D,0xFB,0x03,0x7E,0xFB,0xFC,0x56,0xF2,0x01,0xF4,0xFB,0xFC,0x74,0xFB,0xFE,0x75,0x76,0x77,0xFF,0xA0,0x19,0xFB,0xFB,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFB,0x01,0x7A,0xFB,0x01,0x7B,0xFB,0x03,0x7C,0xFB,0x04,0x7D,0xFB,0x03,0x7E,0xFB,0xFF,0xFB,0x03,0x7F,0x80,0x81,0x82,0x83,0x84,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFB,0x01,0x7A,0xFB,0x01,0x7B,0xFB,0x03,0x7C,0xFB,0x04,0x7D,0xFB,0x03,0x7E,0xFB,0xFF,0xFB,0x02,0x7F,0x80,0x81,0x82,0x83,0x84,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFB,0x01,0x7A,0xFB,0x01,0x7B,0xFB,0x03,0x7C,0xFB,0x04,0x7D,0xFB,0x03,0x7E,0xFB,0xFF,0xFB,0x01,0x7F,0x80,0x81,0x82,0x83,0x84,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFB,0x01,0x7A,0xFB,0x01,0x7B,0xFB,0x03,0x7C,0xFB,0x04,0x7D,0xFB,0x03,0x7E,0xFB,0xFF,0x7F,0x80,0x81,0x82,0x83,0x84,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFB,0x01,0x7A,0xFB,0x01,0x7B,0xFB,0x03,0x7C,0xFB,0x04,0x7D,0xFB,0x03,0x7E,0xFB,0xFE,0x80,0x81,0x82,0x83,0x84,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFF,0xFC,0x1E,0xF9,0x01,0x79,0xFB,0x01,0x7A,0xFB,0x01,0x7B,0xFB,0x03,0x7C,0xFB,0x04,0x7D,0xFB,0xFF,0x7F,0x80,0x81,0x82,0x83,0x84,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFB,0x01,0x7A,0xFB,0x01,0x7B,0xFB,0x03,0x7C,0xFB,0x02,0x81,0x82,0x83,0x84,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFB,0x01,0x7A,0xFB,0x01,0x7B,0xFB,0x02,0x7C,0xFB,0x02,0x81,0x82,0x83,0x84,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFB,0x01,0x7A,0xFB,0x01,0x7B,0xFB,0x02,0x7C,0xFB,0x01,0x81,0x82,0x83,0x84,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFB,0x01,0x7A,0xFB,0x01,0x7B,0xFB,0x02,0x83,0x84,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFB,0x01,0x7A,0xFB,0x01,0x7B,0xFB,0x01,0x83,0x84,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFB,0x01,0x7A,0xFB,0x01,0x84,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFB,0x01,0x84,0xFF,0xA0,0x19,0xF9,0x01,0xFB,0x01,0x6B,0xFB,0x02,0x6C,0x6D,0xFF,0xAF,0x1F,0xF9,0x05,0xFB,0x01,0x6E,0x6F,0xFB,0x02,0x70,0x71,0xFB,0x01,0x72,0x73,0x74,0xFB,0xFC,0x75,0x76,0x77,0xFF,0xA0,0x19,0xF9,0x01,0xF3,0x01,0xE5,0xE5,0xE5,0xE5,0xE5,0xE5,0xE6,0xE7,0xE8,0xFF,0xDA,0x1F,0xF9,0x01,0xFB,0xFB,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,0xF0,0x85,0x85,0x86,0x86,0x86,0x30,0xFB,0x01,0x31,0xFB,0xFE,0xF9,0x05,0x32,0xF9,0x01,0x33,0x34,0xFF,0xA0,0x19,0xF9,0x01,0xFB,0xFB,0xEA,0xEC,0xEE,0xF0,0x86,0xFB,0xFF,0xFF,0xA0,0x19,0xF9,0x01,0xFB,0x04,0xBF,0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xF2,0x03,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCD,0xCD,0xF3,0x01,0xCD,0xCD,0xC9,0xC6,0xFB,0xFC,0xFF,0xA0,0x19,0xF9,0x05,0xF4,0xFB,0x01,0x6B,0xFB,0x02,0x6C,0xF9,0x01,0x6D,0xFF,0x36,0x20,0xF9,0x01,0xFA,0xFE,0xFB,0x01,0x6B,0xFB,0x02,0x6C,0x6D,0xFB,0x01,0x6E,0x6F,0xFB,0x02,0x70,0x71,0xFB,0x01,0xFA,0x01,0x72,0xFA,0x01,0x73,0xFB,0xFE,0xFF,0x78,0x19,0xF9,0x05,0xF4,0xFA,0xFE,0xFB,0x01,0xFB,0x02,0x6C,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0xFB,0x01,0x6E,0x6E,0x6E,0x6F,0xFB,0x02,0x70,0x71,0xFB,0x01,0xFA,0x01,0x72,0xFA,0x01,0x73,0x74,0xFB,0xFC,0x75,0x76,0x77,0xFF,0xA0,0x19,0xF9,0x05,0xF4,0xFA,0xFE,0xFB,0x03,0xB9,0xF6,0xB9,0xFF,0xA5,0x20,0xF9,0x05,0xFF,0xAE,0x20,0xF9,0x01,0xF6,0xB3,0xB4,0xB5,0xB6,0xFB,0x01,0xB7,0xFB,0xFC,0xB9,0xFF,0xBA,0x20,0xF9,0x01,0xF4,0xFB,0x04,0xB1,0xF6,0xB1,0xFF,0xC5,0x20,0xF9,0x01,0xB2,0xF6,0xB2,0xFF,0xCD,0x20,0xFF,0x5A,0x20,0xF8,0x00,0x00,0xF9,0x04,0xB9,0xFF,0xD9,0x20,0xF9,0x05,0xFB,0xFB,0xFA,0xFF,0xF2,0x01,0xD9,0xDA,0xDB,0xFB,0x01,0xDC,0xFB,0xFC,0xFA,0xFD,0xF2,0x01,0xDD,0xFB,0xFC,0xFA,0xFE,0xDE,0xFB,0xFE,0xFA,0xFD,0xDF,0xFB,0xFD,0xFA,0xF8,0xF2,0x04,0xF2,0x01,0xE0,0xFB,0xFF,0xFA,0xFF,0xE1,0xFB,0xFD,0xFA,0xFC,0xE2,0xFB,0xFF,0xFA,0xFB,0xF2,0x01,0xE3,0xFB,0xFE,0xFA,0xFF,0xE4,0x00,0xF2,0x01,0x00,0x00,0x00,0xF2,0x01,0x00,0x00,0x00,0xF2,0x01,0x00,0x00,0x00,0xF2,0x01,0xF1,0x00,0xFF,0x2E,0x21,0x36,0xFF,0x32,0x21,0x55,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x53,0x54,0x4C,0xFF,0x4B,0x21,0xFB,0x01,0x30,0xFB,0x02,0x31,0xFB,0x06,0x32,0xFB,0x01,0x33,0xFB,0xFF,0x34,0xFB,0x01,0x35,0xFB,0x01,0xFF,0x51,0x21,0xFB,0x01,0x37,0x38,0xFF,0x32,0x21,0x4D,0x4E,0x4F,0x50,0x51,0x52,0xFB,0x01,0x36,0x36,0x36,0x36,0x36,0x36,0x39,0x3A,0x3B,0x3C,0x3D,0xFB,0x02,0x3E,0xFB,0xFF,0x3F,0xFB,0xFD,0x40,0x41,0xFB,0xFF,0x42,0xFE,0xFB,0x10,0xFB,0x03,0xFF,0x54,0x21,0x0B,0xFF,0x95,0x21,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0xFE,0xFB,0x08,0x0B,0xFF,0x95,0x21,0xFE,0xFB,0x0B,0x0C,0xFB,0x01,0x0D,0xFB,0x01,0x0E,0xFB,0x03,0x0F,0xFB,0x01,0x10,0x11,0xFF,0xB8,0x21,0x13,0xFF,0xBC,0x21,0x14,0xFF,0xC0,0x21,0x15,0xFB,0x01,0x16,0x17,0x18,0xFB,0x01,0x19,0xFB,0xFD,0x1A,0xFB,0xFE,0x1B,0xFB,0xFC,0x1C,0xFB,0xFD,0x1D,0xFB,0xFE,0x1E,0xFB,0xFD,0x1F,0xFB,0xFF,0x20,0x21,0xFF,0xE2,0x21,0x25,0xFF,0xE6,0x21,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,0xFE,0xFB,0x0C,0x0B,0xFF,0xF8,0x21,0x0B,0x0B,0xFE,0xFB,0x0D,0x2F,0x2E,0x2D,0x2C,0x2B,0x2A,0x29,0x28,0x27,0x26,0x25,0x24,0x24,0x24,0x23,0x23,0x23,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x23,0x23,0x24,0x24,0x24,0x23,0x23,0x23,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x23,0x23,0x24,0x24,0x24,0x23,0x23,0x23,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x23,0x23,0x23,0x24,0xFF,0x3C,0x22,0x01,0x12,0xFF,0x41,0x22,0xF9,0x01,0xBA,0xFB,0x05,0xBA,0xFB,0x03,0xBB,0xFB,0x04,0xFF,0x47,0x22,0xBA,0xFF,0x53,0x22,0xBC,0xFF,0x57,0x22,0xF9,0x00,0xBA,0xBA,0xBA,0xBC,0xBC,0xBC,0xBC,0xBC,0xBC,0xBC,0xBC,0xFE,0xFB,0x08,0xFF,0x47,0x22,0xBA,0xBA,0xBC,0xFF,0x70,0x22};


// seg006:0006
int __pascal far get_tile(int room,int col,int row) {
	curr_room = room;
	tile_col = col;
	tile_row = row;
	curr_room = find_room_of_tile();
	// bugfix: check_chomped_kid may call with room = -1
	if (curr_room > 0) {
		get_room_address(curr_room);
		curr_tilepos = tbl_line[tile_row] + tile_col;
		curr_tile2 = curr_room_tiles[curr_tilepos] & 0x1F;
	} else {
		// wall in room 0
		curr_tile2 = tiles_20_wall;
	}
	return curr_tile2;
}

// seg006:005D
int __pascal far find_room_of_tile() {
	again:
	if (tile_col < 0) {
		tile_col += 10;
		if (curr_room) {
			curr_room = level.roomlinks[curr_room - 1].left;
		}
		//find_room_of_tile();
		goto again;
	} else if (tile_col >= 10) {
		tile_col -= 10;
		if (curr_room) {
			curr_room = level.roomlinks[curr_room - 1].right;
		}
		//find_room_of_tile();
		goto again;
	} else if (tile_row < 0) {
		tile_row += 3;
		if (curr_room) {
			curr_room = level.roomlinks[curr_room - 1].up;
		}
		//find_room_of_tile();
		goto again;
	} else if (tile_row >= 3) {
		tile_row -= 3;
		if (curr_room) {
			curr_room = level.roomlinks[curr_room - 1].down;
		}
		//find_room_of_tile();
		goto again;
	}
	return curr_room;
}

// seg006:00EC
int __pascal far get_tilepos(int tile_col,int tile_row) {
	if (tile_row < 0) {
		return -(tile_col + 1);
	} else if (tile_row >= 3 || tile_col >= 10 || tile_col < 0) {
		return 30;
	} else {
		return tbl_line[tile_row] + tile_col;
	}
}

// seg006:0124
int __pascal far get_tilepos_nominus(int tile_col,int tile_row) {
	short var_2;
	var_2 = get_tilepos(tile_col, tile_row);
	if (var_2 < 0) return 30; else return var_2;
}

// seg006:0144
void __pascal far load_fram_det_col() {
	load_frame();
	determine_col();
}

// seg006:014D
void __pascal far determine_col() {
	Char.curr_col = get_tile_div_mod_m7(dx_weight());
}

// data:0FE0
const frame_type frame_table_kid[] = {
{ 255, 0x00| 0,   0,   0, 0x00| 0},
{   0, 0x00| 0,   1,   0, 0xC0| 4},
{   1, 0x00| 0,   1,   0, 0x40| 4},
{   2, 0x00| 0,   3,   0, 0x40| 7},
{   3, 0x00| 0,   4,   0, 0x40| 8},
{   4, 0x00| 0,   0,   0, 0xE0| 6},
{   5, 0x00| 0,   0,   0, 0x40| 9},
{   6, 0x00| 0,   0,   0, 0x40|10},
{   7, 0x00| 0,   0,   0, 0xC0| 5},
{   8, 0x00| 0,   0,   0, 0x40| 4},
{   9, 0x00| 0,   0,   0, 0x40| 7},
{  10, 0x00| 0,   0,   0, 0x40|11},
{  11, 0x00| 0,   0,   0, 0x40| 3},
{  12, 0x00| 0,   0,   0, 0xC0| 3},
{  13, 0x00| 0,   0,   0, 0x40| 7},
{  14, 0x00| 9,   0,   0, 0x40| 3},
{  15, 0x00| 0,   0,   0, 0xC0| 3},
{  16, 0x00| 0,   0,   0, 0x40| 4},
{  17, 0x00| 0,   0,   0, 0x40| 6},
{  18, 0x00| 0,   0,   0, 0x40| 8},
{  19, 0x00| 0,   0,   0, 0x80| 9},
{  20, 0x00| 0,   0,   0, 0x00|11},
{  21, 0x00| 0,   0,   0, 0x80|11},
{  22, 0x00| 0,   0,   0, 0x00|17},
{  23, 0x00| 0,   0,   0, 0x00| 7},
{  24, 0x00| 0,   0,   0, 0x00| 5},
{  25, 0x00| 0,   0,   0, 0xC0| 1},
{  26, 0x00| 0,   0,   0, 0xC0| 6},
{  27, 0x00| 0,   0,   0, 0x40| 3},
{  28, 0x00| 0,   0,   0, 0x40| 8},
{  29, 0x00| 0,   0,   0, 0x40| 2},
{  30, 0x00| 0,   0,   0, 0x40| 2},
{  31, 0x00| 0,   0,   0, 0xC0| 2},
{  32, 0x00| 0,   0,   0, 0xC0| 2},
{  33, 0x00| 0,   0,   0, 0x40| 3},
{  34, 0x00| 0,   0,   0, 0x40| 8},
{  35, 0x00| 0,   0,   0, 0xC0|14},
{  36, 0x00| 0,   0,   0, 0xC0| 1},
{  37, 0x00| 0,   0,   0, 0x40| 5},
{  38, 0x00| 0,   0,   0, 0x80|14},
{  39, 0x00| 0,   0,   0, 0x00|11},
{  40, 0x00| 0,   0,   0, 0x80|11},
{  41, 0x00| 0,   0,   0, 0x80|10},
{  42, 0x00| 0,   0,   0, 0x00| 1},
{  43, 0x00| 0,   0,   0, 0xC0| 4},
{  44, 0x00| 0,   0,   0, 0xC0| 3},
{  45, 0x00| 0,   0,   0, 0xC0| 3},
{  46, 0x00| 0,   0,   0, 0xA0| 5},
{  47, 0x00| 0,   0,   0, 0xA0| 4},
{  48, 0x00| 0,   0,   0, 0x60| 6},
{  49, 0x00| 0,   4,   0, 0x60| 7},
{  50, 0x00| 0,   3,   0, 0x60| 6},
{  51, 0x00| 0,   1,   0, 0x40| 4},
{  64, 0x00| 0,   0,   0, 0xC0| 2},
{  65, 0x00| 0,   0,   0, 0x40| 1},
{  66, 0x00| 0,   0,   0, 0x40| 2},
{  67, 0x00| 0,   0,   0, 0x00| 0},
{  68, 0x00| 0,   0,   0, 0x00| 0},
{  69, 0x00| 0,   0,   0, 0x80| 0},
{  70, 0x00| 0,   0,   0, 0x00| 0},
{  71, 0x00| 0,   0,   0, 0x80| 0},
{  72, 0x00| 0,   0,   0, 0x00| 0},
{  73, 0x00| 0,   0,   0, 0x80| 0},
{  74, 0x00| 0,   0,   0, 0x00| 0},
{  75, 0x00| 0,   0,   0, 0x00| 0},
{  76, 0x00| 0,   0,   0, 0x80| 0},
{ 255, 0x00| 0,   0,   0, 0x00| 0},
{  80, 0x00| 0,  -2,   0, 0x40| 1},
{  81, 0x00| 0,  -2,   0, 0x40| 1},
{  82, 0x00| 0,  -1,   0, 0xC0| 2},
{  83, 0x00| 0,  -2,   0, 0x40| 2},
{  84, 0x00| 0,  -2,   0, 0x40| 1},
{  85, 0x00| 0,  -2,   0, 0x40| 1},
{  86, 0x00| 0,  -2,   0, 0x40| 1},
{  87, 0x00| 0,  -1,   0, 0x00| 7},
{  88, 0x00| 0,  -1,   0, 0x00| 5},
{  89, 0x00| 0,   2,   0, 0x00| 7},
{  90, 0x00| 0,   2,   0, 0x00| 7},
{  91, 0x00| 0,   2,  -3, 0x00| 0},
{  92, 0x00| 0,   2, -10, 0x00| 0},
{  93, 0x00| 0,   2, -11, 0x80| 0},
{  94, 0x00| 0,   3,  -2, 0x40| 3},
{  95, 0x00| 0,   3,   0, 0xC0| 3},
{  96, 0x00| 0,   3,   0, 0xC0| 3},
{  97, 0x00| 0,   3,   0, 0x60| 3},
{  98, 0x00| 0,   4,   0, 0xE0| 3},
{  28, 0x00| 0,   0,   0, 0x00| 0},
{  99, 0x00| 0,   7, -14, 0x80| 0},
{ 100, 0x00| 0,   7, -12, 0x80| 0},
{ 101, 0x00| 0,   4, -12, 0x00| 0},
{ 102, 0x00| 0,   3, -10, 0x80| 0},
{ 103, 0x00| 0,   2, -10, 0x80| 0},
{ 104, 0x00| 0,   1, -10, 0x80| 0},
{ 105, 0x00| 0,   0, -11, 0x00| 0},
{ 106, 0x00| 0,  -1, -12, 0x00| 0},
{ 107, 0x00| 0,  -1, -14, 0x00| 0},
{ 108, 0x00| 0,  -1, -14, 0x00| 0},
{ 109, 0x00| 0,  -1, -15, 0x80| 0},
{ 110, 0x00| 0,  -1, -15, 0x80| 0},
{ 111, 0x00| 0,   0, -15, 0x00| 0},
{ 255, 0x00| 0,   0,   0, 0x00| 0},
{ 255, 0x00| 0,   0,   0, 0x00| 0},
{ 112, 0x00| 0,   0,   0, 0xC0| 6},
{ 113, 0x00| 0,   0,   0, 0x40| 6},
{ 114, 0x00| 0,   0,   0, 0xC0| 5},
{ 115, 0x00| 0,   0,   0, 0x40| 5},
{ 116, 0x00| 0,   0,   0, 0xC0| 2},
{ 117, 0x00| 0,   0,   0, 0xC0| 4},
{ 118, 0x00| 0,   0,   0, 0xC0| 5},
{ 119, 0x00| 0,   0,   0, 0x40| 6},
{ 120, 0x00| 0,   0,   0, 0x40| 7},
{ 121, 0x00| 0,   0,   0, 0x40| 7},
{ 122, 0x00| 0,   0,   0, 0x40| 9},
{ 123, 0x00| 0,   0,   0, 0xC0| 8},
{ 124, 0x00| 0,   0,   0, 0xC0| 9},
{ 125, 0x00| 0,   0,   0, 0x40| 9},
{ 126, 0x00| 0,   0,   0, 0x40| 5},
{ 127, 0x00| 0,   2,   0, 0x40| 5},
{ 128, 0x00| 0,   2,   0, 0xC0| 5},
{ 129, 0x00| 0,   0,   0, 0xC0| 3},
{ 255, 0x00| 0,   0,   0, 0x00| 0},
{ 133, 0x00| 0,   0,   0, 0x40| 3},
{ 134, 0x00| 0,   0,   0, 0xC0| 4},
{ 135, 0x00| 0,   0,   0, 0xC0| 5},
{ 136, 0x00| 0,   0,   0, 0x40| 8},
{ 137, 0x00| 0,   0,   0, 0x60|12},
{ 138, 0x00| 0,   0,   0, 0xE0|15},
{ 139, 0x00| 0,   0,   0, 0x60| 3},
{ 140, 0x00| 0,   0,   0, 0xC0| 3},
{ 141, 0x00| 0,   0,   0, 0x40| 3},
{ 142, 0x00| 0,   0,   0, 0x40| 3},
{ 143, 0x00| 0,   0,   0, 0x40| 4},
{ 144, 0x00| 0,   0,   0, 0x40| 4},
{ 172, 0x00| 0,   0,   1, 0xC0| 1},
{ 173, 0x00| 0,   0,   1, 0xC0| 7},
{ 145, 0x00| 0,   0, -12, 0x00| 1},
{ 146, 0x00| 0,   0, -21, 0x00| 0},
{ 147, 0x00| 0,   1, -26, 0x80| 0},
{ 148, 0x00| 0,   4, -32, 0x80| 0},
{ 149, 0x00| 0,   6, -36, 0x80| 1},
{ 150, 0x00| 0,   7, -41, 0x80| 2},
{ 151, 0x00| 0,   2,  17, 0x40| 2},
{ 152, 0x00| 0,   4,   9, 0xC0| 4},
{ 153, 0x00| 0,   4,   5, 0xC0| 9},
{ 154, 0x00| 0,   4,   4, 0xC0| 8},
{ 155, 0x00| 0,   5,   0, 0x60| 9},
{ 156, 0x00| 0,   5,   0, 0xE0| 9},
{ 157, 0x00| 0,   5,   0, 0xE0| 8},
{ 158, 0x00| 0,   5,   0, 0x60| 9},
{ 159, 0x00| 0,   5,   0, 0x60| 9},
{ 184, 0x00|16,   0,   2, 0x80| 0},
{ 174, 0x00|26,   0,   2, 0x80| 0},
{ 175, 0x00|18,   3,   2, 0x00| 0},
{ 176, 0x00|22,   7,   2, 0xC0| 4},
{ 177, 0x00|21,  10,   2, 0x00| 0},
{ 178, 0x00|23,   7,   2, 0x80| 0},
{ 179, 0x00|25,   4,   2, 0x80| 0},
{ 180, 0x00|24,   0,   2, 0xC0|14},
{ 181, 0x00|15,   0,   2, 0xC0|13},
{ 182, 0x00|20,   3,   2, 0x00| 0},
{ 183, 0x00|31,   3,   2, 0x00| 0},
{ 184, 0x00|16,   0,   2, 0x80| 0},
{ 185, 0x00|17,   0,   2, 0x80| 0},
{ 186, 0x00|32,   0,   2, 0x00| 0},
{ 187, 0x00|33,   0,   2, 0x80| 0},
{ 188, 0x00|34,   2,   2, 0xC0| 3},
{  14, 0x00| 0,   0,   0, 0x40| 3},
{ 189, 0x00|19,   7,   2, 0x80| 0},
{ 190, 0x00|14,   1,   2, 0x80| 0},
{ 191, 0x00|27,   0,   2, 0x80| 0},
{ 181, 0x00|15,   0,   2, 0xC0|13},
{ 181, 0x00|15,   0,   2, 0xC0|13},
{ 112, 0x00|43,   0,   0, 0xC0| 6},
{ 113, 0x00|44,   0,   0, 0x40| 6},
{ 114, 0x00|45,   0,   0, 0xC0| 5},
{ 115, 0x00|46,   0,   0, 0x40| 5},
{ 114, 0x00| 0,   0,   0, 0xC0| 5},
{  78, 0x00| 0,   0,   3, 0x80|10},
{  77, 0x00| 0,   4,   3, 0x80| 7},
{ 211, 0x00| 0,   0,   1, 0x40| 4},
{ 212, 0x00| 0,   0,   1, 0x40| 4},
{ 213, 0x00| 0,   0,   1, 0x40| 4},
{ 214, 0x00| 0,   0,   1, 0x40| 7},
{ 215, 0x00| 0,   0,   7, 0x40|11},
{ 255, 0x00| 0,   0,   0, 0x00| 0},
{  79, 0x00| 0,   4,   7, 0x40| 9},
{ 130, 0x00| 0,   0,   0, 0x40| 4},
{ 131, 0x00| 0,   0,   0, 0x40| 4},
{ 132, 0x00| 0,   0,   2, 0x40| 4},
{ 255, 0x00| 0,   0,   0, 0x00| 0},
{ 255, 0x00| 0,   0,   0, 0x00| 0},
{ 192, 0x00| 0,   0,   0, 0x00| 0},
{ 193, 0x00| 0,   0,   1, 0x00| 0},
{ 194, 0x00| 0,   0,   0, 0x80| 0},
{ 195, 0x00| 0,   0,   0, 0x00| 0},
{ 196, 0x00| 0,  -1,   0, 0x00| 0},
{ 197, 0x00| 0,  -1,   0, 0x00| 0},
{ 198, 0x00| 0,  -1,   0, 0x00| 0},
{ 199, 0x00| 0,  -4,   0, 0x00| 0},
{ 200, 0x00| 0,  -4,   0, 0x80| 0},
{ 201, 0x00| 0,  -4,   0, 0x00| 0},
{ 202, 0x00| 0,  -4,   0, 0x00| 0},
{ 203, 0x00| 0,  -4,   0, 0x00| 0},
{ 204, 0x00| 0,  -4,   0, 0x00| 0},
{ 205, 0x00| 0,  -5,   0, 0x00| 0},
{ 206, 0x00| 0,  -5,   0, 0x00| 0},
{ 255, 0x00| 0,   0,   0, 0x00| 0},
{ 207, 0x00| 0,   0,   1, 0x40| 6},
{ 208, 0x00| 0,   0,   1, 0xC0| 6},
{ 209, 0x00| 0,   0,   1, 0xC0| 8},
{ 210, 0x00| 0,   0,   1, 0x40|10},
{ 255, 0x00| 0,   0,   0, 0x00| 0},
{ 255, 0x00| 0,   0,   0, 0x00| 0},
{ 255, 0x00| 0,   0,   0, 0x00| 0},
{ 255, 0x00| 0,   0,   0, 0x00| 0},
{ 255, 0x00| 0,   0,   0, 0x00| 0},
{ 255, 0x00| 0,   0,   0, 0x00| 0},
{  52, 0x00| 0,   0,   0, 0x80| 0},
{  53, 0x00| 0,   0,   0, 0x00| 0},
{  54, 0x00| 0,   0,   0, 0x00| 0},
{  55, 0x00| 0,   0,   0, 0x00| 0},
{  56, 0x00| 0,   0,   0, 0x80| 0},
{  57, 0x00| 0,   0,   0, 0x00| 0},
{  58, 0x00| 0,   0,   0, 0x00| 0},
{  59, 0x00| 0,   0,   0, 0x00| 0},
{  60, 0x00| 0,   0,   0, 0x80| 0},
{  61, 0x00| 0,   0,   0, 0x00| 0},
{  62, 0x00| 0,   0,   0, 0x80| 0},
{  63, 0x00| 0,   0,   0, 0x00| 0},
{ 160, 0x00|35,   1,   1, 0xC0| 3},
{ 161, 0x00|36,   0,   1, 0x40| 9},
{ 162, 0x00|37,   0,   1, 0xC0| 3},
{ 163, 0x00|38,   0,   1, 0x40| 9},
{ 164, 0x00|39,   0,   1, 0xC0| 3},
{ 165, 0x00|40,   1,   1, 0x40| 9},
{ 166, 0x00|41,   1,   1, 0x40| 3},
{ 167, 0x00|42,   1,   1, 0xC0| 9},
{ 168, 0x00| 0,   4,   1, 0xC0| 6},
{ 169, 0x00| 0,   3,   1, 0xC0|10},
{ 170, 0x00| 0,   1,   1, 0x40| 3},
{ 171, 0x00| 0,   1,   1, 0xC0| 8},
};

// data:1496
const frame_type frame_tbl_guard[] = {
{ 255, 0x00| 0,   0,   0, 0x00| 0},
{  12, 0xC0|13,   2,   1, 0x00| 0},
{   2, 0xC0| 1,   3,   1, 0x00| 0},
{   3, 0xC0| 2,   4,   1, 0x00| 0},
{   4, 0xC0| 3,   7,   1, 0x40| 4},
{   5, 0xC0| 4,  10,   1, 0x00| 0},
{   6, 0xC0| 5,   7,   1, 0x80| 0},
{   7, 0xC0| 6,   4,   1, 0x80| 0},
{   8, 0xC0| 7,   0,   1, 0x80| 0},
{   9, 0xC0| 8,   0,   1, 0xC0|13},
{  10, 0xC0|11,   7,   1, 0x80| 0},
{  11, 0xC0|12,   3,   1, 0x00| 0},
{  12, 0xC0|13,   2,   1, 0x00| 0},
{  13, 0xC0| 0,   2,   1, 0x00| 0},
{  14, 0xC0|28,   0,   1, 0x00| 0},
{  15, 0xC0|29,   0,   1, 0x80| 0},
{  16, 0xC0|30,   2,   1, 0xC0| 3},
{  17, 0xC0| 9,  -1,   1, 0x40| 8},
{  18, 0xC0|10,   7,   1, 0x80| 0},
{  19, 0xC0|14,   3,   1, 0x80| 0},
{   9, 0xC0| 8,   0,   1, 0x80| 0},
{  20, 0xC0| 8,   0,   1, 0xC0|13},
{  21, 0xC0| 8,   0,   1, 0xC0|13},
{  22, 0xC0|47,   0,   0, 0xC0| 6},
{  23, 0xC0|48,   0,   0, 0x40| 6},
{  24, 0xC0|49,   0,   0, 0xC0| 5},
{  24, 0xC0|49,   0,   0, 0xC0| 5},
{  24, 0xC0|49,   0,   0, 0xC0| 5},
{  26, 0xC0| 0,   0,   3, 0x80|10},
{  27, 0xC0| 0,   4,   4, 0x80| 7},
{  28, 0xC0| 0,  -2,   1, 0x40| 4},
{  29, 0xC0| 0,  -2,   1, 0x40| 4},
{  30, 0xC0| 0,  -2,   1, 0x40| 4},
{  31, 0xC0| 0,  -2,   2, 0x40| 7},
{  32, 0xC0| 0,  -2,   2, 0x40|10},
{ 255, 0x00| 0,   0,   0, 0x00| 0},
{  33, 0xC0| 0,   3,   4, 0xC0| 9},
{ 255, 0x00| 0,   0,   0, 0x00| 0},
{ 255, 0x00| 0,   0,   0, 0x00| 0},
{ 255, 0x00| 0,   0,   0, 0x00| 0},
{ 255, 0x00| 0,   0,   0, 0x00| 0},
};

// data:1564
const frame_type frame_tbl_cuts[] = {
{ 255, 0x00| 0,   0,   0, 0x00| 0},
{  15, 0x40| 0,   0,   0, 0x00| 0},
{   1, 0x40| 0,   0,   0, 0x80| 0},
{   2, 0x40| 0,   0,   0, 0x80| 0},
{   3, 0x40| 0,   0,   0, 0x80| 0},
{   4, 0x40| 0,  -1,   0, 0x00| 0},
{   5, 0x40| 0,   2,   0, 0x80| 0},
{   6, 0x40| 0,   2,   0, 0x00| 0},
{   7, 0x40| 0,   0,   0, 0x80| 0},
{   8, 0x40| 0,   1,   0, 0x80| 0},
{ 255, 0x00| 0,   0,   0, 0x00| 0},
{   0, 0x40| 0,   0,   0, 0x80| 0},
{   9, 0x40| 0,   0,   0, 0x80| 0},
{  10, 0x40| 0,   0,   0, 0x00| 0},
{  11, 0x40| 0,   0,   0, 0x80| 0},
{  12, 0x40| 0,   0,   0, 0x80| 0},
{  13, 0x40| 0,   0,   0, 0x80| 0},
{  14, 0x40| 0,   0,   0, 0x00| 0},
{  16, 0x40| 0,   0,   0, 0x00| 0},
{   0, 0x80| 0,   0,   0, 0x00| 0},
{   2, 0x80| 0,   0,   0, 0x00| 0},
{   3, 0x80| 0,   0,   0, 0x00| 0},
{   4, 0x80| 0,   0,   0, 0x80| 0},
{   5, 0x80| 0,   0,   0, 0x00| 0},
{   6, 0x80| 0,   0,   0, 0x80| 0},
{   7, 0x80| 0,   0,   0, 0x80| 0},
{   8, 0x80| 0,   0,   0, 0x00| 0},
{   9, 0x80| 0,   0,   0, 0x00| 0},
{  10, 0x80| 0,   0,   0, 0x00| 0},
{  11, 0x80| 0,   0,   0, 0x00| 0},
{  12, 0x80| 0,   0,   0, 0x00| 0},
{  13, 0x80| 0,   0,   0, 0x00| 0},
{  14, 0x80| 0,   0,   0, 0x00| 0},
{  15, 0x80| 0,   0,   0, 0x00| 0},
{  16, 0x80| 0,   0,   0, 0x00| 0},
{  17, 0x80| 0,   0,   0, 0x00| 0},
{  18, 0x80| 0,   0,   0, 0x00| 0},
{  19, 0x80| 0,   0,   0, 0x00| 0},
{  20, 0x80| 0,   0,   0, 0x80| 0},
{  21, 0x80| 0,   0,   0, 0x80| 0},
{  22, 0x80| 0,   1,   0, 0x00| 0},
{  23, 0x80| 0,  -1,   0, 0x00| 0},
{  24, 0x80| 0,   2,   0, 0x00| 0},
{  25, 0x80| 0,   1,   0, 0x80| 0},
{  26, 0x80| 0,   0,   0, 0x80| 0},
{  27, 0x80| 0,   0,   0, 0x80| 0},
{  28, 0x80| 0,   0,   0, 0x80| 0},
{  29, 0x80| 0,  -1,   0, 0x00| 0},
{   0, 0x80| 0,   0,   0, 0x80| 0},
{   1, 0x80| 0,   0,   0, 0x80| 0},
{   2, 0x80| 0,   0,   0, 0x80| 0},
{   3, 0x80| 0,   0,   0, 0x00| 0},
{   4, 0x80| 0,   0,   0, 0x00| 0},
{   5, 0x80| 0,   0,   0, 0x80| 0},
{   6, 0x80| 0,   0,   0, 0x80| 0},
{   7, 0x80| 0,   0,   0, 0x80| 0},
{   8, 0x80| 0,   0,   0, 0x80| 0},
{   9, 0x80| 0,   0,   0, 0x80| 0},
{  10, 0x80| 0,   0,   0, 0x80| 0},
{  11, 0x80| 0,   0,   0, 0x80| 0},
{  12, 0x80| 0,   0,   0, 0x80| 0},
{  13, 0x80| 0,   0,   0, 0x00| 0},
{  14, 0x80| 0,   0,   0, 0x80| 0},
{  15, 0x80| 0,   0,   0, 0x00| 0},
{  16, 0x80| 0,   0,   0, 0x00| 0},
{  17, 0x80| 0,   0,   0, 0x80| 0},
{  18, 0x80| 0,   0,   0, 0x00| 0},
{  19, 0x80| 0,   3,   0, 0x00| 0},
{  20, 0x80| 0,   3,   0, 0x00| 0},
{  21, 0x80| 0,   3,   0, 0x00| 0},
{  22, 0x80| 0,   2,   0, 0x00| 0},
{  23, 0x80| 0,   3,   0, 0x80| 0},
{  24, 0x80| 0,   5,   0, 0x00| 0},
{  25, 0x80| 0,   5,   0, 0x00| 0},
{  26, 0x80| 0,   1,   0, 0x80| 0},
{  27, 0x80| 0,   2,   0, 0x80| 0},
{  28, 0x80| 0,   2,   0, 0x80| 0},
{  29, 0x80| 0,   1,   0, 0x80| 0},
{  30, 0x80| 0,   1,   0, 0x00| 0},
{  31, 0x80| 0,   2,   0, 0x00| 0},
{  32, 0x80| 0,   3,   0, 0x00| 0},
{  33, 0x80| 0,   3,   0, 0x00| 0},
{  34, 0x80| 0,   0,   0, 0x80| 0},
{  35, 0x80| 0,   2,   0, 0x80| 0},
{  36, 0x80| 0,   2,   0, 0x80| 0},
{  37, 0x80| 0,   1,   0, 0x00| 0},
};

// seg006:015A
void __pascal far load_frame() {
	short frame;
	short add_frame;
	frame = Char.frame;
	add_frame = 0;
	switch (Char.charid) {
		case charid_0_kid:
		case charid_24_mouse:
		use_table_kid:
			cur_frame = frame_table_kid[frame];
		break;
		case charid_2_guard:
		case charid_4_skeleton:
			if (frame >= 102 && frame < 107) add_frame = 70;
			goto use_table_guard;
		case charid_1_shadow:
			if (frame < 150 || frame >= 190) goto use_table_kid;
		use_table_guard:
			cur_frame = frame_tbl_guard[frame + add_frame - 149];
		break;
		case charid_5_princess:
		case charid_6_vizier:
//		use_table_cutscene:
			cur_frame = frame_tbl_cuts[frame];
		break;
	}
}

// seg006:01F5
short __pascal far dx_weight() {
	sbyte var_2;
	var_2 = cur_frame.dx - (cur_frame.flags & 0x1F);
	return char_dx_forward(var_2);
}

// seg006:0213
int __pascal far char_dx_forward(int delta_x) {
	if (Char.direction < dir_0_right) {
		delta_x = -delta_x;
	}
	return delta_x + Char.x;
}

// seg006:0234
int __pascal far obj_dx_forward(int delta_x) {
	if (obj_direction < dir_0_right) {
		delta_x = -delta_x;
	}
	obj_x += delta_x;
	return obj_x;
}

// seg006:0254
void __pascal far play_seq() {
	for (;;) {
		byte item = *(SEQTBL_0 + Char.curr_seq++);
		switch (item) {
			case 0xFB: // dx
				Char.x = char_dx_forward(*(SEQTBL_0 + Char.curr_seq++));
				break;
			case 0xFA: // dy
				Char.y += *(SEQTBL_0 + Char.curr_seq++);
				break;
			case 0xFE: // flip
				Char.direction = ~Char.direction;
				break;
			case 0xF7: // jump if feather
				if (!is_feather_fall) {
					++Char.curr_seq;
					++Char.curr_seq;
					break;
				}
				// fallthrough!
			case 0xFF: // jump
				Char.curr_seq = *(const word*)(SEQTBL_0 + Char.curr_seq);
				break;
			case 0xFD: // up
				--Char.curr_row;
				start_chompers();
				break;
			case 0xFC: // down
				inc_curr_row();
				start_chompers();
				break;
			case 0xF9: // action
				Char.action = *(SEQTBL_0 + Char.curr_seq++);
				break;
			case 0xF8: // set fall
				Char.fall_x = *(SEQTBL_0 + Char.curr_seq++);
				Char.fall_y = *(SEQTBL_0 + Char.curr_seq++);
				break;
			case 0xF5: // knock up
				knock = 1;
				break;
			case 0xF4: // knock down
				knock = -1;
				break;
			case 0xF2: // sound
				switch (*(SEQTBL_0 + Char.curr_seq++)) {
					case 0: // sword
						is_guard_notice = 1;
						break;
					case 1: // feet
						play_sound(sound_23_footstep); // footstep
						is_guard_notice = 1;
						break;
					case 2: // bump
						play_sound(sound_8_bumped); // touching a wall
						is_guard_notice = 1;
						break;
					case 3: // drink
						play_sound(sound_18_drink); // drink
						break;
					case 4: // level
						if (is_sound_on) {
							if (current_level == 4) {
								play_sound(sound_32_shadow_music); // end level with shadow (level 4)
							} else if (current_level != 13 && current_level != 15) {
								play_sound(sound_41_end_level_music); // end level
							}
						}
						break;
				}
				break;
			case 0xF1: // end level
				++next_level;
				break;
			case 0xF3: // get item
				if (*(SEQTBL_0 + Char.curr_seq++) == 1) {
					proc_get_object();
				}
				break;
			case 0xF6: // nop
				break;
			default:
				Char.frame = item;
				//if (Char.frame == 185) Char.frame = 185;
				return;
		}
	}
}

// seg006:03DE
int __pascal far get_tile_div_mod_m7(int xpos) {
	return get_tile_div_mod(xpos - 7);
}

// data:22A6
const sbyte tile_div_tbl[256] = {
                                    -5,-5,
-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,
-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,
-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
10,10,10,10,10,10,10,10,10,10,10,10,10,10,
11,11,11,11,11,11,11,11,11,11,11,11,11,11,
12,12,12,12,12,12,12,12,12,12,12,12,12,12,
13,13,13,13,13,13,13,13,13,13,13,13,13,13,
14,14
};

// data:23A6
const byte tile_mod_tbl[256] = {
                                      12, 13,
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
0, 1
};

// seg006:03F0
int __pascal far get_tile_div_mod(int xpos) {
	// xpos might be negative if the kid is far off left.
	// In this case, the array index overflows.
/*	if (xpos < 0 || xpos >= 256) {
		printf("get_tile_div_mod(): xpos = %d\n", xpos);
	}*/
//	obj_xl = tile_mod_tbl[xpos];
//	return tile_div_tbl[xpos];
	int x = xpos - 58;
	int xl = x % 14;
	int xh = x / 14;
	if (xl < 0) {
		// Integer division rounds towards zero, but we want to round down.
		--xh;
		// Modulo returns a negative number if x is negative, but we want 0 <= xl < 14.
		xl += 14;
	}
	obj_xl = xl;
	return xh;
}

// seg006:0433
int __pascal far y_to_row_mod4(int ypos) {
	return (ypos + 60) / 63 % 4 - 1;
}

// seg006:044F
void __pascal far loadkid() {
	Char = Kid;
}

// seg006:0464
void __pascal far savekid() {
	Kid = Char;
}

// seg006:0479
void __pascal far loadshad() {
	Char = Guard;
}

// seg006:048E
void __pascal far saveshad() {
	Guard = Char;
}

// seg006:04A3
void __pascal far loadkid_and_opp() {
	loadkid();
	Opp = Guard;
}

// seg006:04BC
void __pascal far savekid_and_opp() {
	savekid();
	Guard = Opp;
}

// seg006:04D5
void __pascal far loadshad_and_opp() {
	loadshad();
	Opp = Kid;
}

// seg006:04EE
void __pascal far saveshad_and_opp() {
	saveshad();
	Kid = Opp;
}

// seg006:0507
void __pascal far reset_obj_clip() {
	obj_clip_left = 0;
	obj_clip_top = 0;
	obj_clip_right = 320;
	obj_clip_bottom = 192;
}

// seg006:051C
void __pascal far x_to_xh_and_xl(int xpos, sbyte *xh_addr, sbyte *xl_addr) {
	if (xpos < 0) {
		*xh_addr = -((ABS(-xpos) >> 3) + 1);
		*xl_addr = - ((-xpos - 1) % 8 - 7);
	} else {
		*xh_addr = ABS(xpos) >> 3;
		*xl_addr = xpos % 8;
	}
}

// seg006:057C
void __pascal far fall_accel() {
	if (Char.action == actions_4_in_freefall) {
		if (is_feather_fall) {
			++Char.fall_y;
			if (Char.fall_y > 4) Char.fall_y = 4;
		} else {
			Char.fall_y += 3;
			if (Char.fall_y > 33) Char.fall_y = 33;
		}
	}
}

// seg006:05AE
void __pascal far fall_speed() {
	Char.y += Char.fall_y;
	if (Char.action == actions_4_in_freefall) {
		Char.x = char_dx_forward(Char.fall_x);
		load_fram_det_col();
	}
}

// seg006:05CD
void __pascal far check_action() {
	short frame;
	short action;
	action = Char.action;
	frame = Char.frame;
	// frame 109: crouching
	if (action == actions_6_hang_straight ||
		action == actions_5_bumped
	) {
		if (frame == frame_109_crouch) {
			check_on_floor();
		}
	} else if (action == actions_4_in_freefall) {
		do_fall();
	} else if (action == actions_3_in_midair) {
		// frame 102..106: start fall + fall
		if (frame >= frame_102_start_fall_1 && frame < frame_106_fall) {
			check_grab();
		}
	} else if (action != actions_2_hang_climb) {
		check_on_floor();
	}
}

// seg006:0628
int __pascal far tile_is_floor(int tiletype) {
	switch (tiletype) {
		case tiles_0_empty:
		case tiles_9_bigpillar_top:
		case tiles_12_doortop:
		case tiles_20_wall:
		case tiles_26_lattice_down:
		case tiles_27_lattice_small:
		case tiles_28_lattice_left:
		case tiles_29_lattice_right:
			return 0;
		default:
			return 1;
	}
}

// seg006:0658
void __pascal far check_spiked() {
	short harmful;
	short frame;
	frame = Char.frame;
	if (get_tile(Char.room, Char.curr_col, Char.curr_row) == tiles_2_spike) {
		harmful = is_spike_harmful();
		// frames 7..14: running
		// frames 34..39: start run-jump
		// frame 43: land from run-jump
		// frame 26: lang from standing jump
		if (
			(harmful >= 2 && ((frame>= frame_7_run && frame<15) || (frame>=frame_34_start_run_jump_1 && frame<40))) ||
			((frame == frame_43_running_jump_4 || frame == frame_26_land_after_standing_jump) && harmful != 0)
		) {
			spiked();
		}
	}
}

// seg006:06BD
int __pascal far take_hp(int count) {
	word dead;
	dead = 0;
	if (Char.charid == charid_0_kid) {
		if (count >= hitp_curr) {
			hitp_delta = -hitp_curr;
			dead = 1;
		} else {
			hitp_delta = -count;
		}
	} else {
		if (count >= guardhp_curr) {
			guardhp_delta = -guardhp_curr;
			dead = 1;
		} else {
			guardhp_delta = -count;
		}
	}
	return dead;
}

// seg006:070D
int __pascal far get_tile_at_char() {
	return get_tile(Char.room, Char.curr_col, Char.curr_row);
}

// seg006:0723
void __pascal far set_char_collision() {
	image_type* image;
	if (obj_id < 0 || obj_id >= chtab_addrs[obj_chtab]->n_images) {
		//printf("set_char_collision: from chtab %d (%d images) trying to use out-of-range image index %d\n", obj_chtab, chtab_addrs[obj_chtab]->n_images, obj_id);
		//quit(1);
		// After going up the level door, obj_id=255.
		//return;
		image = NULL;
	} else {
		image = chtab_addrs[obj_chtab]->images[obj_id];
	}
	if (image == NULL) {
		//printf("set_char_collision: from chtab %d (%d images) trying to use NULL image %d\n", obj_chtab, chtab_addrs[obj_chtab]->n_images, obj_id);
		//quit(1);
		char_width_half = 0;
		char_height = 0;
	} else {
		char_width_half = (image->/*width*/w + 1) / 2;
		char_height = image->/*height*/h;
	}
	char_x_left = obj_x / 2 + 58;
	if (Char.direction >= dir_0_right) {
		char_x_left -= char_width_half;
	}
	char_x_left_coll = char_x_left;
	char_x_right_coll = char_x_right = char_x_left + char_width_half;
	char_top_y = obj_y - char_height + 1;
	if (char_top_y >= 192) {
		char_top_y = 0;
	}
	char_top_row = y_to_row_mod4(char_top_y);
	char_bottom_row = y_to_row_mod4(obj_y);
	if (char_bottom_row == -1) {
		char_bottom_row = 3;
	}
	char_col_left = MAX(get_tile_div_mod(char_x_left), 0);
	char_col_right = MIN(get_tile_div_mod(char_x_right), 9);
	if (cur_frame.flags & 0x20) {
		// "thin" this frame for collision detection
		char_x_left_coll += 4;
		char_x_right_coll -= 4;
	}
}

// seg006:0815
void __pascal far check_on_floor() {
	if (cur_frame.flags & 0x40) {
		if (get_tile_at_char() == tiles_20_wall) {
			in_wall();
		}
		if (! tile_is_floor(curr_tile2)) {
			// Special event: floors appear
			if (current_level == 12 &&
				united_with_shadow < 0 &&
				Char.curr_row == 0 &&
				(Char.room == 2 || (Char.room == 13 && tile_col >= 6))
			) {
				curr_room_tiles[curr_tilepos] = tiles_1_floor;
				set_wipe(curr_tilepos, 1);
				set_redraw_full(curr_tilepos, 1);
				++curr_tilepos;
				set_wipe(curr_tilepos, 1);
				set_redraw_full(curr_tilepos, 1);
			} else {
				start_fall();
			}
		}
	}
}

// seg006:08B9
void __pascal far start_fall() {
	short frame;
	word seq_id;
	frame = Char.frame;
	Char.sword = sword_0_sheathed;
	inc_curr_row();
	start_chompers();
	fall_frame = frame;
	if (frame == frame_9_run_land) {
		// frame 9: run
		seq_id = seq_7_fall; // fall (when?)
	} else if (frame == frame_13_run_land) {
		// frame 13: run
		seq_id = seq_19_fall; // fall (when?)
	} else if (frame == frame_26_land_after_standing_jump) {
		// frame 26: land after standing jump
		seq_id = seq_18_fall_after_standing_jump; // fall after standing jump
	} else if (frame == frame_44_land_after_running_jump) {
		// frame 44: land after running jump
		seq_id = seq_21_fall_after_running_jump; // fall after running jump
	} else if (frame >= frame_81_jump_up_3 && frame < 86) {
		// frame 81..85: land after jump up
		seq_id = seq_19_fall; // fall after jumping up
		Char.x = char_dx_forward(5);
		load_fram_det_col();
	} else if (frame >= 150 && frame < 180) {
		// frame 150..179: with sword + fall + dead
		if (Char.charid == charid_2_guard) {
			if (Char.curr_row == 3 && Char.curr_col == 10) {
				clear_char();
				return;
			}
			if (Char.fall_x < 0) {
				seq_id = seq_82_guard_pushed_off_ledge; // Guard is pushed off the ledge
				if (Char.direction < dir_0_right && distance_to_edge_weight() <= 7) {
					Char.x = char_dx_forward(-5);
				}
			} else {
				word_1EA12 = 0;
				seq_id = seq_83_guard_fall; // fall after forwarding with sword
			}
		} else {
			word_1EA12 = 1;
			if (Char.direction < dir_0_right && distance_to_edge_weight() <= 7) {
				Char.x = char_dx_forward(-5);
			}
			seq_id = seq_81_kid_pushed_off_ledge; // fall after backing with sword / Kid is pushed off the ledge
		}
	} else {
		seq_id = seq_7_fall; // fall after stand, run, step, crouch
	}
	seqtbl_offset_char(seq_id);
	play_seq();
	load_fram_det_col();
	if (get_tile_at_char() == tiles_20_wall) {
		in_wall();
		return;
	}
	if (get_tile_infrontof_char() == tiles_20_wall) {
		if (fall_frame != 44 || distance_to_edge_weight() >= 6) {
			Char.x = char_dx_forward(-1);
		} else {
			seqtbl_offset_char(seq_104_start_fall_in_front_of_wall); // start fall (when?)
			play_seq();
		}
		load_fram_det_col();
	}
}

// seg006:0A19
void __pascal far check_grab() {
	word old_x;
	if (control_shift < 0 && // press shift to grab
		Char.fall_y < 32 && // you can't grab if you're falling too fast ...
		Char.alive < 0 && // ... or dead
		(word)y_land[Char.curr_row + 1] <= (word)(Char.y + 25)
	) {
		old_x = Char.x;
		Char.x = char_dx_forward(-8);
		load_fram_det_col();
		if ( ! can_grab_front_above()) {
			Char.x = old_x;
		} else {
			Char.x = char_dx_forward(distance_to_edge_weight());
			Char.y = y_land[Char.curr_row + 1];
			Char.fall_y = 0;
			seqtbl_offset_char(seq_15_grab_ledge_midair); // grab a ledge (after falling)
			play_seq();
			grab_timer = 12;
			play_sound(sound_9_grab); // grab
			is_screaming = 0;
		}
	}
}

// seg006:0ABD
int __pascal far can_grab_front_above() {
	through_tile = get_tile_above_char();
	get_tile_front_above_char();
	return can_grab();
}

// seg006:0ACD
void __pascal far in_wall() {
	short delta_x;
	delta_x = distance_to_edge_weight();
	if (delta_x >= 8 || get_tile_infrontof_char() == tiles_20_wall) {
		delta_x = 6 - delta_x;
	} else {
		delta_x += 4;
	}
	Char.x = char_dx_forward(delta_x);
	load_fram_det_col();
	get_tile_at_char();
}

// seg006:0B0C
int __pascal far get_tile_infrontof_char() {
	return get_tile(Char.room, byte_20C62 = dir_front[Char.direction + 1] + Char.curr_col, Char.curr_row);
}

// seg006:0B30
int __pascal far get_tile_infrontof2_char() {
	short var_2;
	var_2 = dir_front[Char.direction + 1];
	return get_tile(Char.room, byte_20C62 = (var_2 << 1) + Char.curr_col, Char.curr_row);
}

// seg006:0B66
int __pascal far get_tile_behind_char() {
	return get_tile(Char.room, dir_behind[Char.direction + 1] + Char.curr_col, Char.curr_row);
}

// seg006:0B8A
int __pascal far distance_to_edge_weight() {
	return distance_to_edge(dx_weight());
}

// seg006:0B94
int __pascal far distance_to_edge(int xpos) {
	short var_2;
	get_tile_div_mod_m7(xpos);
	var_2 = obj_xl;
	if (Char.direction == dir_0_right) {
		var_2 = 13 - var_2;
	}
	return var_2;
}

// seg006:0BC4
void __pascal far fell_out() {
	if (Char.alive < 0 && Char.room == 0) {
		take_hp(100);
		Char.alive = 0;
		erase_bottom_text(1);
		Char.frame = frame_185_dead; // dead
	}
}

// seg006:0BEE
void __pascal far play_kid() {
	fell_out();
	control_kid();
	if (Char.alive >= 0 && is_dead()) {
		if (resurrect_time) {
			stop_sounds();
			loadkid();
			hitp_delta = hitp_max;
			seqtbl_offset_char(seq_2_stand); // stand
			Char.x += 8;
			play_seq();
			load_fram_det_col();
			set_start_pos();
		}
		if (check_sound_playing() && current_sound != 5) { // gate opening
			return;
		}
		is_show_time = 0;
		if (Char.alive < 0 || Char.alive >= 6) {
			if (Char.alive == 6) {
				if (is_sound_on &&
					current_level != 0 && // no death music on demo level
					current_level != 15 // no death music on potions level
				) {
					play_death_music();
				}
			} else {
				if (Char.alive != 7 || check_sound_playing()) return;
				if (rem_min == 0) {
					expired();
				}
				if (current_level != 0 && // no message if died on demo level
					current_level != 15 // no message if died on potions level
				) {
					text_time_remaining = text_time_total = 288;
					display_text_bottom("Press Button to Continue");
				} else {
					text_time_remaining = text_time_total = 36;
				}
			}
		}
		++Char.alive;
	}
}

// seg006:0CD1
void __pascal far control_kid() {
	word key;
	if (Char.alive < 0 && hitp_curr == 0) {
		Char.alive = 0;
	}
	if (grab_timer != 0) {
		--grab_timer;
	}
	if (current_level == 0) {
		do_demo();
		control();
		// we can start the game or load a game while the demo
		key = key_test_quit();
		if (key == 0x0C) { // ctrl-L
			if (load_game()) {
				start_game();
			}
		} else {
			if (key) {
				start_level = 1;
				start_game();
			}
		}
	} else {
		rest_ctrl_1();
		do_paused();
		read_user_control();
		user_control();
		save_ctrl_1();
	}
}

const auto_move_type demo_moves[] = {
{0x00, 0},
{0x01, 1},
{0x0D, 0},
{0x1E, 1},
{0x25, 5},
{0x2F, 0},
{0x30, 1},
{0x41, 0},
{0x49, 2},
{0x4B, 0},
{0x63, 2},
{0x64, 0},
{0x73, 5},
{0x80, 6},
{0x88, 3},
{0x9D, 7},
{0x9E, 0},
{0x9F, 1},
{0xAB, 4},
{0xB1, 0},
{0xB2, 1},
{0xBC, 0},
{0xC1, 1},
{0xCD, 0},
{0xE9,-1},
};

// seg006:0D49
void __pascal far do_demo() {
	if (checkpoint) {
		control_shift2 = release_arrows();
		control_forward = control_x = -1;
	} else if (Char.sword) {
		guard_skill = 10;
		autocontrol_opponent();
		guard_skill = 11;
	} else {
		do_auto_moves(demo_moves);
	}
}

// seg006:0D85
void __pascal far play_guard() {
	if (Char.charid == charid_24_mouse) {
		autocontrol_opponent();
	} else {
		if (Char.alive < 0) {
			if (guardhp_curr == 0) {
				Char.alive = 0;
				on_guard_killed();
			} else {
				goto loc_7A65;
			}
		}
		if (Char.charid == charid_1_shadow) {
			clear_char();
		}
		loc_7A65:
		autocontrol_opponent();
		control();
	}
}

// seg006:0DC0
void __pascal far user_control() {
	if (Char.direction >= dir_0_right) {
		flip_control_x();
		control();
		flip_control_x();
	} else {
		control();
	}
}

// seg006:0DDC
void __pascal far flip_control_x() {
	byte temp;
	control_x = -control_x;
	temp = control_forward;
	control_forward = control_backward;
	control_backward = temp;
}

// seg006:0E00
int __pascal far release_arrows() {
	control_backward = control_forward = control_up = control_down = 0;
	return 1;
}

// seg006:0E12
void __pascal far save_ctrl_1() {
	ctrl1_forward = control_forward;
	ctrl1_backward = control_backward;
	ctrl1_up = control_up;
	ctrl1_down = control_down;
	ctrl1_shift2 = control_shift2;
}

// seg006:0E31
void __pascal far rest_ctrl_1() {
	control_forward = ctrl1_forward;
	control_backward = ctrl1_backward;
	control_up = ctrl1_up;
	control_down = ctrl1_down;
	control_shift2 = ctrl1_shift2;
}

// seg006:0E8E
void __pascal far clear_saved_ctrl() {
	ctrl1_forward = ctrl1_backward = ctrl1_up = ctrl1_down = ctrl1_shift2 = 0;
}

// seg006:0EAF
void __pascal far read_user_control() {
	if (control_forward >= 0) {
		if (control_x < 0) {
			if (control_forward == 0) {
				control_forward = -1;
			}
		} else {
			control_forward = 0;
		}
	}
	if (control_backward >= 0) {
		if (control_x == 1) {
			if (control_backward == 0) {
				control_backward = -1;
			}
		} else {
			control_backward = 0;
		}
	}
	if (control_up >= 0) {
		if (control_y < 0) {
			if (control_up == 0) {
				control_up = -1;
			}
		} else {
			control_up = 0;
		}
	}
	if (control_down >= 0) {
		if (control_y == 1) {
			if (control_down == 0) {
				control_down = -1;
			}
		} else {
			control_down = 0;
		}
	}
	if (control_shift2 >= 0) {
		if (control_shift < 0) {
			if (control_shift2 == 0) {
				control_shift2 = -1;
			}
		} else {
			control_shift2 = 0;
		}
	}
}

// seg006:0F55
int __pascal far can_grab() {
	// Can char grab curr_tile2 through through_tile?
	byte modifier;
	modifier = curr_room_modif[curr_tilepos];
	// can't grab through wall
	if (through_tile == tiles_20_wall) return 0;
	// can't grab through a door top if looking right
	if (through_tile == tiles_12_doortop && Char.direction >= dir_0_right) return 0;
	// can't grab through floor
	if (tile_is_floor(through_tile)) return 0;
	// can't grab a shaking loose floor
	if (curr_tile2 == tiles_11_loose && modifier != 0) return 0;
	// a doortop with floor can be grabbed only from the left (looking right)
	if (curr_tile2 == tiles_7_doortop_with_floor && Char.direction < dir_0_right) return 0;
	// can't grab something that has no floor
	if ( ! tile_is_floor(curr_tile2)) return 0;
	return 1;
}

// seg006:0FC3
int __pascal far wall_type(byte tiletype) {
	switch (tiletype) {
		case tiles_4_gate:
		case tiles_7_doortop_with_floor:
		case tiles_12_doortop:
			return 1; // wall at right
		case tiles_13_mirror:
			return 2; // wall at left
		case tiles_18_chomper:
			return 3; // chomper at left
		case tiles_20_wall:
			return 4; // wall at both sides
		default:
			return 0; // no wall
	}
}

// seg006:1005
int __pascal far get_tile_above_char() {
	return get_tile(Char.room, Char.curr_col, Char.curr_row - 1);
}

// seg006:1020
int __pascal far get_tile_behind_above_char() {
	return get_tile(Char.room, dir_behind[Char.direction + 1] + Char.curr_col, Char.curr_row - 1);
}

// seg006:1049
int __pascal far get_tile_front_above_char() {
	return get_tile(Char.room, byte_20C62 = dir_front[Char.direction + 1] + Char.curr_col, Char.curr_row - 1);
}

// seg006:1072
int __pascal far back_delta_x(int delta_x) {
	if (Char.direction < dir_0_right) {
		// direction = left
		return delta_x;
	} else {
		// direction = right
		return -delta_x;
	}
}

// seg006:108A
void __pascal far do_pickup(int obj_type) {
	pickup_obj_type = obj_type;
	control_shift2 = 1;
	// erase picked up item
	curr_room_tiles[curr_tilepos] = tiles_1_floor;
	curr_room_modif[curr_tilepos] = 0;
	redraw_height = 35;
	set_wipe(curr_tilepos, 1);
	set_redraw_full(curr_tilepos, 1);
}

// seg006:10E6
void __pascal far check_press() {
	short frame;
	short action;
	frame = Char.frame;
	action = Char.action;
	// frames 87..99: hanging
	// frames 135..140: start climb up
	if ((frame >= frame_87_hanging_1 && frame < 100) || (frame >= frame_135_climbing_1 && frame < frame_141_climbing_7)) {
		// the pressed tile is the one that the char is grabbing
		get_tile_above_char();
	} else if (action == actions_7_turn || action == actions_5_bumped || action < actions_2_hang_climb) {
		// frame 79: jumping up
		if (frame == frame_79_grab_2 && get_tile_above_char() == tiles_11_loose) {
			// break a loose floor from above
			make_loose_fall(1);
		} else {
			// the pressed tile is the one that the char is standing on
			if (! (cur_frame.flags & 0x40)) return;
			get_tile_at_char();
		}
	} else {
		return;
	}
	if (curr_tile2 == tiles_15_opener || curr_tile2 == tiles_6_closer) {
		if (Char.alive < 0) {
			trigger(1, 0, -1);
		} else {
			died_on_button();
		}
	} else if (curr_tile2 == tiles_11_loose) {
		is_guard_notice = 1;
		if (!is_shadow_effect)
			make_loose_fall(1);
		else make_loose_fall(0x80);
	}
}

// seg006:1199
void __pascal far check_spike_below() {
	short not_finished;
	short room;
	short row;
	short col;
	short right_col;
	right_col = get_tile_div_mod_m7(char_x_right);
	if (right_col < 0) return;
	row = Char.curr_row;
	room = Char.room;
	for (col = get_tile_div_mod_m7(char_x_left); col <= right_col; ++col) {
		row = Char.curr_row;
		do {
			not_finished = 0;
			if (get_tile(room, col, row) == tiles_2_spike) {
				start_anim_spike(curr_room, curr_tilepos);
			} else if (
				! tile_is_floor(curr_tile2) &&
				curr_room != 0 &&
#ifdef FIX_INFINITE_DOWN_BUG
				row <= 2
#else
				room == curr_room
#endif
			) {
				++row;
				not_finished = 1;
			}
		} while(not_finished);
	}
}

// seg006:1231
void __pascal far clip_char() {
	short frame;
	short room;
	short action;
	short col;
	short var_A;
	short row;
	short var_E;
	frame = Char.frame;
	action = Char.action;
	room = Char.room;
	row = Char.curr_row;
	reset_obj_clip();
	// frames 217..228: going up the level door
	if (frame >= frame_224_exit_stairs_8 && frame < 229) {
		obj_clip_top = leveldoor_ybottom + 1;
		obj_clip_right = leveldoor_right;
	} else {
		if (
			get_tile(room, char_col_left, char_top_row) == tiles_20_wall ||
			tile_is_floor(curr_tile2)
		) {
			// frame 79: jump up, frame 81: grab
			if ((action == actions_0_stand && (frame == frame_79_grab_2 || frame == frame_81_jump_up_3)) ||
				get_tile(room, char_col_right, char_top_row) == tiles_20_wall ||
				tile_is_floor(curr_tile2)
			) {
				var_E = row + 1;
				if (var_E == 1 ||
					((var_A = y_clip[var_E]) < obj_y && var_A - 15 < char_top_y)
				) {
					obj_clip_top = char_top_y = y_clip[var_E];
				}
			}
		}
		col = get_tile_div_mod(char_x_left_coll - 4);
		if (get_tile(room, col + 1, row) == tiles_7_doortop_with_floor ||
			curr_tile2 == tiles_12_doortop
		) {
			obj_clip_right = (tile_col << 5) + 32;
		} else {
			if ((get_tile(room, col, row) != tiles_7_doortop_with_floor &&
				curr_tile2 != tiles_12_doortop) ||
				action == actions_3_in_midair ||
				(action == actions_4_in_freefall && frame == frame_106_fall) ||
				(action == actions_5_bumped && frame == frame_107_fall_land_1) ||
				(Char.direction < dir_0_right && (
					action == actions_2_hang_climb ||
					action == actions_6_hang_straight ||
					(action == actions_1_run_jump &&
					frame >= frame_137_climbing_3 && frame < frame_140_climbing_6)
				))
			) {
				if (
					(get_tile(room, col = get_tile_div_mod(char_x_right_coll), row) == tiles_20_wall ||
					(curr_tile2 == tiles_13_mirror && Char.direction == dir_0_right)) &&
					(get_tile(room, col, char_top_row) == tiles_20_wall ||
					curr_tile2 == tiles_13_mirror) &&
					room == curr_room
				) {
					obj_clip_right = tile_col << 5;
				}
			} else {
				obj_clip_right = (tile_col << 5) + 32;
			}
		}
	}
}

// seg006:13E6
void __pascal far stuck_lower() {
	if (get_tile_at_char() == tiles_5_stuck) {
		++Char.y;
	}
}

// seg006:13F3
void __pascal far set_objtile_at_char() {
	short char_frame;
	short char_action;
	char_frame = Char.frame;
	char_action = Char.action;
	if (char_action == actions_1_run_jump) {
		tile_row = char_bottom_row;
		tile_col = char_col_left;
	} else {
		tile_row = Char.curr_row;
		tile_col = Char.curr_col;
	}
	// frame 135..148: climbing
	if ((char_frame >= frame_135_climbing_1 && char_frame < 149) ||
		char_action == actions_2_hang_climb ||
		char_action == actions_3_in_midair ||
		char_action == actions_4_in_freefall ||
		char_action == actions_6_hang_straight
	) {
		--tile_col;
	}
	obj_tilepos = get_tilepos_nominus(tile_col, tile_row);
	//printf("set_objtile_at_char: obj_tile = %d\n", obj_tile); // debug
}

// seg006:1463
void __pascal far proc_get_object() {
	if (Char.charid != charid_0_kid || pickup_obj_type == 0) return;
	if (pickup_obj_type == -1) {
		have_sword = -1;
		play_sound(sound_37_victory); // get sword
		flash_color = color_14_yellow;
		flash_time = 8;
	} else {
		switch (--pickup_obj_type) {
			case 0: // health
				if (hitp_curr != hitp_max) {
					stop_sounds();
					play_sound(sound_33_small_potion); // small potion
					hitp_delta = 1;
					flash_color = color_4_red;
					flash_time = 2;
				}
			break;
			case 1: // life
				stop_sounds();
				play_sound(sound_30_big_potion); // big potion
				flash_color = color_4_red;
				flash_time = 4;
				add_life();
			break;
			case 2: // feather
				feather_fall();
			break;
			case 3: // invert
				toggle_upside();
			break;
			case 5: // open
				get_tile(8, 0, 0);
				trigger(0, 0, -1);
			break;
			case 4: // hurt
				stop_sounds();
				play_sound(sound_13_kid_hurt); // Kid hurt (by potion)
				// Special event: blue potions on potions level take half of HP
				if (current_level == 15) {
					hitp_delta = - ((hitp_max + 1) >> 1);
				} else {
					hitp_delta = -1;
				}
			break;
			case 6: // shadow potion
				stop_sounds();
				united_with_shadow = 10; // blink
				is_shadow_effect = 220;
				flash_color = 7; // grey
				flash_time = 4;
				hitp_max--;
				hitp_delta = 1 - hitp_curr; // remove all but 1 hp
				draw_kid_hp(hitp_curr-1, hitp_max+1); // erase one hp box
				play_sound(sound_39_low_weight); // low weight
				break;
			case 7: // extra time potion
				stop_sounds();
				play_sound(sound_37_victory);
				flash_color = 7; // grey
				flash_time = 4;
				extra_minutes_to_be_added = 15;
				if (hitp_curr >= hitp_max) hitp_curr--;
				hitp_max--;
				draw_kid_hp(0, hitp_max+1); // erase one hp box
				draw_kid_hp(hitp_curr, hitp_max); // erase one hp box
				is_show_time = 1;
				break;
		}
	}
}

// seg006:1599
int __pascal far is_dead() {
	// 177: spiked, 178: chomped, 185: dead
	// or maybe this was a switch-case?
	return Char.frame >= frame_177_spiked && (Char.frame <= frame_178_chomped || Char.frame == frame_185_dead);
}

// seg006:15B5
void __pascal far play_death_music() {
	word sound_id;
	if (Guard.charid == charid_1_shadow) {
		sound_id = sound_32_shadow_music; // killed by shadow
	} else if (holding_sword) {
		sound_id = sound_28_death_in_fight; // death in fight
	} else {
		sound_id = sound_24_death_regular; // death not in fight
	}
	play_sound(sound_id);
}

// seg006:15E8
void __pascal far on_guard_killed() {
	if (current_level == 0) {
		// demo level: after killing Guard, run out of room
		checkpoint = 1;
		demo_index = demo_time = 0;
	} else if (current_level == 13) {
		// Jaffar's level: flash
		flash_color = color_15_white; // white
		flash_time = 18;
		is_show_time = 1;
		leveldoor_open = 2;
		play_sound(sound_43_victory_Jaffar); // Jaffar's death
	} else if (Char.charid != charid_1_shadow) {
		play_sound(sound_37_victory); // Guard's death
	}
}

// seg006:1634
void __pascal far clear_char() {
	Char.direction = dir_56_none;
	Char.alive = 0;
	Char.action = 0;
	draw_guard_hp(0, guardhp_curr);
	guardhp_curr = 0;
}

// data:42EC
byte obj2_tilepos;
// data:34A6
word obj2_x;
// data:34A8
byte obj2_y;
// data:599E
sbyte obj2_direction;
// data:5948
byte obj2_id;
// data:42BE
byte obj2_chtab;
// data:4D90
short obj2_clip_top;
// data:460C
short obj2_clip_bottom;
// data:4C94
short obj2_clip_left;
// data:4CDE
short obj2_clip_right;

// seg006:1654
void __pascal far save_obj() {
	obj2_tilepos = obj_tilepos;
	obj2_x = obj_x;
	obj2_y = obj_y;
	obj2_direction = obj_direction;
	obj2_id = obj_id;
	obj2_chtab = obj_chtab;
	obj2_clip_top = obj_clip_top;
	obj2_clip_bottom = obj_clip_bottom;
	obj2_clip_left = obj_clip_left;
	obj2_clip_right = obj_clip_right;
}

// seg006:1691
void __pascal far load_obj() {
	obj_tilepos = obj2_tilepos;
	obj_x = obj2_x;
	obj_y = obj2_y;
	obj_direction = obj2_direction;
	obj_id = obj2_id;
	obj_chtab = obj2_chtab;
	obj_clip_top = obj2_clip_top;
	obj_clip_bottom = obj2_clip_bottom;
	obj_clip_left = obj2_clip_left;
	obj_clip_right = obj2_clip_right;
}

// seg006:16CE
void __pascal far draw_hurt_splash() {
	short frame;
	frame = Char.frame;
	if (frame != frame_178_chomped) { // chomped
		save_obj();
		obj_tilepos = -1;
		// frame 185: dead
		// frame 106..110: fall + land
		if (frame == frame_185_dead || (frame>= frame_106_fall && frame<111)) {
			obj_y += 4;
			obj_dx_forward(5);
		} else if (frame == frame_177_spiked) { // spiked
			obj_dx_forward(-5);
		} else {
			obj_y -= ((Char.charid == charid_0_kid) << 2) + 11;
			obj_dx_forward(5);
		}
		if (Char.charid == charid_0_kid) {
			obj_chtab = id_chtab_2_kid;
			obj_id = 218; // splash!
		} else {
			obj_chtab = id_chtab_5_guard;
			obj_id = 1; // splash!
		}
		reset_obj_clip();
		add_objtable(5); // hurt splash
		load_obj();
	}
}

// seg006:175D
void __pascal far check_killed_shadow() {
	// Special event: killed the shadow
	if (current_level == 12) {
		if ((Char.charid | Opp.charid) == charid_1_shadow &&
			Char.alive < 0 && Opp.alive >= 0
		) {
			flash_color = color_15_white; // white
			flash_time = 5;
			take_hp(100);
		}
	}
}

// data:1712
const sword_table_type sword_tbl[] = {
{ 255,   0,   0},
{   0,   0,  -9},
{   5,  -9, -29},
{   1,   7, -25},
{   2,  17, -26},
{   6,   7, -14},
{   7,   0,  -5},
{   3,  17, -16},
{   4,  16, -19},
{  30,  12,  -9},
{   8,  13, -34},
{   9,   7, -25},
{  10,  10, -16},
{  11,  10, -11},
{  12,  22, -21},
{  13,  28, -23},
{  14,  13, -35},
{  15,   0, -38},
{  16,   0, -29},
{  17,  21, -19},
{  18,  14, -23},
{  19,  21, -22},
{  19,  22, -23},
{  17,   7, -13},
{  17,  15, -18},
{   7,   0,  -8},
{   1,   7, -27},
{  28,  14, -28},
{   8,   7, -27},
{   4,   6, -23},
{   4,   9, -21},
{  10,  11, -18},
{  13,  24, -23},
{  13,  19, -23},
{  13,  21, -23},
{  20,   7, -32},
{  21,  14, -32},
{  22,  14, -31},
{  23,  14, -29},
{  24,  28, -28},
{  25,  28, -28},
{  26,  21, -25},
{  27,  14, -22},
{ 255,  14, -25},
{ 255,  21, -25},
{  29,   0, -16},
{   8,   8, -37},
{  31,  14, -24},
{  32,  14, -24},
{  33,   7, -14},
{   8,   8, -37},
};

// seg006:1798
void __pascal far add_sword_to_objtable() {
	short frame;
	short sword_frame;
	frame = Char.frame;
	if ((frame >= frame_229_found_sword && frame < 238) || // found sword + put sword away
		Char.sword != sword_0_sheathed ||
		(Char.charid == charid_2_guard && Char.alive < 0)
	) {
		sword_frame = cur_frame.sword & 0x3F;
		if (sword_frame) {
			obj_id = sword_tbl[sword_frame].id;
			if (obj_id != 0xFF) {
				obj_x = calc_screen_x_coord(obj_x);
				obj_dx_forward(sword_tbl[sword_frame].x);
				obj_y += sword_tbl[sword_frame].y;
				obj_chtab = id_chtab_0_sword;
				add_objtable(3); // sword
			}
		}
	}
}

// seg006:1827
void __pascal far control_guard_inactive() {
	if (Char.frame == frame_166_stand_inactive && control_down < 0) {
		if (control_forward < 0) {
			draw_sword();
		} else {
			control_down = 1;
			seqtbl_offset_char(seq_80_stand_flipped); // stand flipped
		}
	}
}

// seg006:1852
int __pascal far char_opp_dist() {
	// >0 if Opp is in front of char
	// <0 if Opp is behind char
	short distance;
	if (Char.room != Opp.room) {
		return 999;
	}
	distance = Opp.x - Char.x;
	if (Char.direction < dir_0_right) {
		distance = -distance;
	}
	if (distance >= 0 && Char.direction != Opp.direction) {
		distance += 13;
	}
	return distance;
}

// seg006:189B
void __pascal far inc_curr_row() {
	++Char.curr_row;
}
