/* GBA SRAM save layer.
   The cart's SRAM is 32 KB at 0x0E000000, byte-only access. gbafix tags the
   ROM as having SRAM so emulators will create/persist a .sav file.

   Layout:
     [0..15]    gba_sram_header { magic, version, payload_len, fletcher32 }
     [16..N+15] opaque payload

   SDLPoP writes to a file named "PRINCE.SAV" (see save_file in seg000.c).
   We intercept that file name in gba_file.c — for "wb" mode we buffer writes
   to a RAM scratch and commit to SRAM on fclose; for "rb" we populate a RAM
   copy from SRAM at fopen.  All other names route to read-only ROM blobs.
*/
#include <string.h>
#include "gba_port.h"

#define SRAM_BASE ((volatile uint8_t*)0x0E000000)
#define SRAM_CAP  0x8000u

static uint32_t fletcher32(const uint8_t* d, size_t n) {
    uint32_t s1 = 0xFFFF, s2 = 0xFFFF;
    while (n) {
        size_t t = n > 359 ? 359 : n;
        n -= t;
        while (t--) { s1 = (s1 + *d++) % 0xFFFF; s2 = (s2 + s1) % 0xFFFF; }
    }
    return (s2 << 16) | s1;
}

int gba_sram_write(const void* data, size_t len) {
    if (len + sizeof(gba_sram_header_t) > SRAM_CAP) return -1;
    gba_sram_header_t hdr = {
        .magic = GBA_SRAM_MAGIC,
        .version = 1,
        .payload_len = (uint32_t)len,
        .checksum = fletcher32((const uint8_t*)data, len),
    };
    const uint8_t* hb = (const uint8_t*)&hdr;
    for (size_t i = 0; i < sizeof(hdr); ++i) SRAM_BASE[i] = hb[i];
    const uint8_t* p = (const uint8_t*)data;
    for (size_t i = 0; i < len; ++i) SRAM_BASE[sizeof(hdr) + i] = p[i];
    return 0;
}

int gba_sram_read(void* data, size_t cap, size_t* out_len) {
    gba_sram_header_t hdr;
    uint8_t* hb = (uint8_t*)&hdr;
    for (size_t i = 0; i < sizeof(hdr); ++i) hb[i] = SRAM_BASE[i];
    if (hdr.magic != GBA_SRAM_MAGIC) return -1;
    if (hdr.payload_len > cap)       return -2;
    uint8_t* p = (uint8_t*)data;
    for (size_t i = 0; i < hdr.payload_len; ++i) p[i] = SRAM_BASE[sizeof(hdr) + i];
    if (fletcher32(p, hdr.payload_len) != hdr.checksum) return -3;
    if (out_len) *out_len = hdr.payload_len;
    return 0;
}

void gba_sram_erase(void) {
    for (size_t i = 0; i < 16; ++i) SRAM_BASE[i] = 0;
}
