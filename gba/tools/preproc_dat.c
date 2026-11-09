/* Pre-decompress sprite resources in a PoP DAT file.
 *
 * For each resource we can confidently identify as an image_data_type
 * (compression method 0..4, depth 1..8, sane height/width, decoder
 * produces exactly the expected output size from at most the payload
 * bytes available), rewrite it as a depth=8 cmeth=0 record holding the
 * already-decoded 8bpp pixels.  All other resources pass through
 * unchanged.  The output DAT shares the input's table layout but with
 * new offsets/sizes; the runtime can fast-path the pre-decoded ones
 * and skip the per-blit decompression scratch buffers.
 *
 * Usage: preproc_dat input.dat output.dat
 *
 * Builds with any host C compiler:
 *   cc -O2 -o preproc_dat preproc_dat.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

typedef uint8_t  byte;
typedef int8_t   sbyte;
typedef uint16_t word;
typedef uint32_t dword;

/* DAT header / table entry layout (pragma packed in upstream types.h). */
#pragma pack(push, 1)
typedef struct {
    uint32_t table_offset;
    uint16_t table_size;
} dat_header_t;
typedef struct {
    uint16_t id;
    uint32_t offset;
    uint16_t size;
} dat_res_t;
typedef struct {
    uint16_t height;
    uint16_t width;
    uint16_t flags;
    /* uint8_t data[]; */
} image_hdr_t;
#pragma pack(pop)

/* --- Decompressors, ported verbatim from src/seg009.c (size-tracking
 * versions: each returns -1 on overrun so the caller can reject inputs
 * that aren't really compressed images). ------------------------------ */

static int decompress_rle_lr(byte* dst, const byte* src, int src_avail, int dst_len) {
    int s = 0, d = 0;
    while (d < dst_len) {
        if (s >= src_avail) return -1;
        sbyte count = (sbyte)src[s++];
        if (count >= 0) {
            int n = count + 1;
            while (n-- > 0 && d < dst_len) {
                if (s >= src_avail) return -1;
                dst[d++] = src[s++];
            }
        } else {
            int n = -count;
            if (s >= src_avail) return -1;
            byte v = src[s++];
            while (n-- > 0 && d < dst_len) dst[d++] = v;
        }
    }
    return s;
}

static int decompress_rle_ud(byte* dst, const byte* src, int src_avail,
                              int dst_len, int width, int height) {
    /* Same as in seg009.c: the destination is filled column-by-column;
     * rem_length counts physical writes. */
    int s = 0;
    int rem_height = height;
    int rem_length = dst_len;
    byte* dest_pos = dst;
    /* per upstream: dest_end_offset = dst_len - 1, used to rewind to next col */
    int dest_end = dst_len - 1;
    int w_step = width - 1;
    if (rem_length <= 0) return s;
    while (rem_length > 0) {
        if (s >= src_avail) return -1;
        sbyte count = (sbyte)src[s++];
        if (count >= 0) {
            int n = count + 1;
            while (n > 0 && rem_length > 0) {
                if (s >= src_avail) return -1;
                if (dest_pos < dst || dest_pos >= dst + dst_len) return -1;
                *dest_pos = src[s++];
                dest_pos++;
                dest_pos += w_step;
                rem_height--;
                if (rem_height == 0) {
                    dest_pos -= dest_end;
                    rem_height = height;
                }
                rem_length--;
                n--;
            }
        } else {
            int n = -count;
            if (s >= src_avail) return -1;
            byte v = src[s++];
            while (n > 0 && rem_length > 0) {
                if (dest_pos < dst || dest_pos >= dst + dst_len) return -1;
                *dest_pos = v;
                dest_pos++;
                dest_pos += w_step;
                rem_height--;
                if (rem_height == 0) {
                    dest_pos -= dest_end;
                    rem_height = height;
                }
                rem_length--;
                n--;
            }
        }
    }
    return s;
}

static int decompress_lzg_lr(byte* dst, const byte* src, int src_avail, int dst_len) {
    byte window[0x400];
    memset(window, 0, sizeof(window));
    int wp = 0x400 - 0x42;
    int rem = dst_len;
    int s = 0, d = 0;
    word mask = 0;
    while (rem > 0) {
        mask >>= 1;
        if ((mask & 0xFF00) == 0) {
            if (s >= src_avail) return -1;
            mask = (word)(src[s++]) | 0xFF00;
        }
        if (mask & 1) {
            if (s >= src_avail) return -1;
            byte v = src[s++];
            window[wp++] = v;
            if (d >= dst_len) return -1;
            dst[d++] = v;
            if (wp >= 0x400) wp = 0;
            rem--;
        } else {
            if (s + 1 >= src_avail) return -1;
            word ci = src[s++];
            ci = (ci << 8) | src[s++];
            int cs = ci & 0x3FF;
            int cl = (ci >> 10) + 3;
            while (rem > 0 && cl > 0) {
                byte v = window[cs];
                window[wp++] = v;
                if (d >= dst_len) return -1;
                dst[d++] = v;
                cs++;
                if (cs >= 0x400) cs = 0;
                if (wp >= 0x400) wp = 0;
                rem--;
                cl--;
            }
        }
    }
    return s;
}

static int decompress_lzg_ud(byte* dst, const byte* src, int src_avail,
                              int dst_len, int stride, int height) {
    byte window[0x400];
    memset(window, 0, sizeof(window));
    int wp = 0x400 - 0x42;
    int rem_h = height;
    int rem_len = dst_len;
    int dest_end = dst_len - 1;
    int s = 0;
    byte* dest_pos = dst;
    word mask = 0;
    while (rem_len > 0) {
        mask >>= 1;
        if ((mask & 0xFF00) == 0) {
            if (s >= src_avail) return -1;
            mask = (word)(src[s++]) | 0xFF00;
        }
        if (mask & 1) {
            if (s >= src_avail) return -1;
            byte v = src[s++];
            window[wp++] = v;
            if (dest_pos < dst || dest_pos >= dst + dst_len) return -1;
            *dest_pos = v;
            if (wp >= 0x400) wp = 0;
            dest_pos += stride;
            rem_h--;
            if (rem_h == 0) {
                dest_pos -= dest_end;
                rem_h = height;
            }
            rem_len--;
        } else {
            if (s + 1 >= src_avail) return -1;
            word ci = src[s++];
            ci = (ci << 8) | src[s++];
            int cs = ci & 0x3FF;
            int cl = (ci >> 10) + 3;
            while (rem_len > 0 && cl > 0) {
                byte v = window[cs];
                window[wp++] = v;
                if (dest_pos < dst || dest_pos >= dst + dst_len) return -1;
                *dest_pos = v;
                cs++;
                if (cs >= 0x400) cs = 0;
                if (wp >= 0x400) wp = 0;
                dest_pos += stride;
                rem_h--;
                if (rem_h == 0) {
                    dest_pos -= dest_end;
                    rem_h = height;
                }
                rem_len--;
                cl--;
            }
        }
    }
    return s;
}

/* Expand bitpacked depth-bpp source into 8bpp output (one byte per pixel). */
static void unpack_to_8bpp(byte* dst, const byte* src, int width, int height,
                            int stride, int depth) {
    int ppb = 8 / depth;
    int mask = (1 << depth) - 1;
    for (int y = 0; y < height; ++y) {
        const byte* in = src + (size_t)y * stride;
        byte* out = dst + (size_t)y * width;
        int x = 0;
        for (int xb = 0; xb < stride; ++xb) {
            byte v = *in++;
            int shift = 8;
            for (int p = 0; p < ppb && x < width; ++p, ++x) {
                shift -= depth;
                *out++ = (byte)((v >> shift) & mask);
            }
        }
    }
}

/* Try to recognize a sound_buffer_type wrapping a digi_new_type (the PoP
 * 1.3 wave format used in DIGISND*.DAT) and convert its samples in
 * place from unsigned 8-bit to signed 8-bit.  Returns a newly-allocated
 * payload + size on success, NULL otherwise.
 *
 * Layout (pack(1)):
 *   byte type;              // == 1 (sound_digi)
 *   word sample_rate;       // 4000..48000 in practice
 *   byte sample_size;       // == 8
 *   word sample_count;
 *   word unknown;
 *   word unknown2;
 *   byte samples[sample_count];
 * Total payload = 10 + sample_count.
 *
 * A sentinel is left in unknown2's high bit so the runtime can tell
 * pre-converted samples from raw ones. */
static byte* try_convert_digi_sound(const byte* entry_buf, int entry_size,
                                     int* out_size) {
    if (entry_size < 1 + 10 + 1) return NULL;
    const byte* payload = entry_buf + 1;
    int payload_size = entry_size - 1;
    if (payload_size < 11) return NULL;
    if (payload[0] != 1) return NULL;  /* not sound_digi */
    uint16_t sample_rate  = (uint16_t)(payload[1] | (payload[2] << 8));
    uint8_t  sample_size  = payload[3];
    uint16_t sample_count = (uint16_t)(payload[4] | (payload[5] << 8));
    if (sample_rate < 4000 || sample_rate > 48000) return NULL;
    if (sample_size != 8) return NULL;
    if (sample_count == 0) return NULL;
    if (payload_size != 10 + (int)sample_count) return NULL;
    byte* out = (byte*)malloc((size_t)payload_size);
    if (!out) return NULL;
    memcpy(out, payload, 10);
    /* Sentinel: set high bit of unknown2 to mark "samples already int8". */
    out[9] = (byte)(payload[9] | 0x80);
    for (int i = 0; i < (int)sample_count; ++i) {
        out[10 + i] = (byte)((int)payload[10 + i] - 128);
    }
    *out_size = payload_size;
    return out;
}

/* --- DAT walking ----------------------------------------------------- */

/* Try to interpret entry as an image; on success allocate and return a
 * new payload (raw 8bpp), write its size to *out_size, and the original
 * checksum byte to *out_chk.  Returns NULL if not classifiable as image
 * (caller should pass entry through unchanged). */
static byte* try_decode_image(const byte* entry_buf, int entry_size,
                               int* out_size) {
    if (entry_size < 1 + (int)sizeof(image_hdr_t)) return NULL;
    /* entry_buf[0] = checksum byte; payload starts at +1 */
    const byte* payload = entry_buf + 1;
    int payload_size = entry_size - 1;
    if (payload_size < (int)sizeof(image_hdr_t)) return NULL;
    image_hdr_t hdr;
    memcpy(&hdr, payload, sizeof(hdr));
    int height = hdr.height;
    int width  = hdr.width;
    int flags  = hdr.flags;
    int depth  = ((flags >> 12) & 7) + 1;
    int cmeth  = (flags >> 8) & 0x0F;
    if (height < 1 || height > 256) return NULL;
    if (width  < 1 || width  > 400) return NULL;
    if (depth  < 1 || depth  > 8)   return NULL;
    if (cmeth  > 4) return NULL;
    int stride = (depth * width + 7) / 8;
    int decoded_size = stride * height;
    if (decoded_size <= 0 || decoded_size > 1024 * 1024) return NULL;
    const byte* cdata = payload + sizeof(image_hdr_t);
    int cdata_size = payload_size - (int)sizeof(image_hdr_t);
    if (cdata_size < 0) return NULL;
    byte* decoded = (byte*)malloc((size_t)decoded_size);
    if (!decoded) return NULL;
    memset(decoded, 0, (size_t)decoded_size);
    int used = -1;
    switch (cmeth) {
        case 0:
            /* RAW: payload must match expected size exactly. */
            if (cdata_size != decoded_size) { free(decoded); return NULL; }
            memcpy(decoded, cdata, (size_t)decoded_size);
            used = decoded_size;
            break;
        case 1:
            used = decompress_rle_lr(decoded, cdata, cdata_size, decoded_size);
            break;
        case 2:
            used = decompress_rle_ud(decoded, cdata, cdata_size, decoded_size, stride, height);
            break;
        case 3:
            used = decompress_lzg_lr(decoded, cdata, cdata_size, decoded_size);
            break;
        case 4:
            used = decompress_lzg_ud(decoded, cdata, cdata_size, decoded_size, stride, height);
            break;
    }
    if (used < 0) { free(decoded); return NULL; }
    /* Decompression succeeded and consumed at most payload_size bytes.
     * Now expand to 8bpp. */
    byte* out = (byte*)malloc(sizeof(image_hdr_t) + (size_t)width * height);
    if (!out) { free(decoded); return NULL; }
    image_hdr_t oh;
    oh.height = (uint16_t)height;
    oh.width  = (uint16_t)width;
    /* Mark as depth=8 (bits 12..14 = 7), cmeth=0 (bits 8..11 = 0). */
    oh.flags  = (uint16_t)((flags & 0x00FF) | (7 << 12));
    memcpy(out, &oh, sizeof(oh));
    if (depth == 8) {
        memcpy(out + sizeof(oh), decoded, (size_t)width * height);
    } else {
        unpack_to_8bpp(out + sizeof(oh), decoded, width, height, stride, depth);
    }
    free(decoded);
    *out_size = (int)sizeof(image_hdr_t) + width * height;
    return out;
}

/* Read whole file. */
static byte* slurp(const char* path, int* out_size) {
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    byte* buf = (byte*)malloc((size_t)n);
    if (!buf) { fclose(f); return NULL; }
    if (fread(buf, 1, (size_t)n, f) != (size_t)n) { free(buf); fclose(f); return NULL; }
    fclose(f);
    *out_size = (int)n;
    return buf;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s input.dat output.dat\n", argv[0]);
        return 2;
    }
    int in_size = 0;
    byte* in_buf = slurp(argv[1], &in_size);
    if (!in_buf) { perror(argv[1]); return 1; }
    if (in_size < (int)sizeof(dat_header_t)) {
        fprintf(stderr, "%s: too small to be a DAT, copying unchanged\n", argv[1]);
        FILE* f = fopen(argv[2], "wb");
        if (!f) { perror(argv[2]); return 1; }
        fwrite(in_buf, 1, (size_t)in_size, f);
        fclose(f);
        return 0;
    }
    dat_header_t hdr;
    memcpy(&hdr, in_buf, sizeof(hdr));
    if ((int)hdr.table_offset > in_size ||
        (int)(hdr.table_offset + 2) > in_size) {
        fprintf(stderr, "%s: malformed (table outside file), copying unchanged\n", argv[1]);
        FILE* f = fopen(argv[2], "wb");
        fwrite(in_buf, 1, (size_t)in_size, f); fclose(f);
        return 0;
    }
    uint16_t res_count;
    memcpy(&res_count, in_buf + hdr.table_offset, 2);
    int table_bytes = 2 + res_count * (int)sizeof(dat_res_t);
    if ((int)(hdr.table_offset + table_bytes) > in_size) {
        fprintf(stderr, "%s: malformed (table truncated), copying unchanged\n", argv[1]);
        FILE* f = fopen(argv[2], "wb");
        fwrite(in_buf, 1, (size_t)in_size, f); fclose(f);
        return 0;
    }
    dat_res_t* entries = (dat_res_t*)malloc((size_t)res_count * sizeof(dat_res_t));
    if (!entries) { perror("malloc"); return 1; }
    memcpy(entries, in_buf + hdr.table_offset + 2,
           (size_t)res_count * sizeof(dat_res_t));

    /* Reserve 6 bytes for the (rewritten) header; resources start there. */
    int cur = 6;
    int cap = in_size * 2 + 64;
    byte* out = (byte*)malloc((size_t)cap);
    if (!out) { perror("malloc"); return 1; }
    memset(out, 0, 6);

    dat_res_t* new_entries = (dat_res_t*)malloc((size_t)res_count * sizeof(dat_res_t));
    int decoded_count = 0;

    for (int i = 0; i < res_count; ++i) {
        dat_res_t e = entries[i];
        new_entries[i].id = e.id;
        /* offset[i] points at the checksum byte. */
        if ((int)(e.offset + 1 + e.size) > in_size) {
            /* malformed entry: copy as-is */
            int sz = 1 + e.size;
            while (cur + sz > cap) { cap *= 2; out = (byte*)realloc(out, (size_t)cap); }
            memcpy(out + cur, in_buf + e.offset, (size_t)sz);
            new_entries[i].offset = (uint32_t)cur;
            new_entries[i].size   = e.size;
            cur += sz;
            continue;
        }
        const byte* eb = in_buf + e.offset;
        int new_size = 0;
        byte* repl = try_decode_image(eb, 1 + e.size, &new_size);
        if (!repl) repl = try_convert_digi_sound(eb, 1 + e.size, &new_size);
        if (repl) {
            /* Write checksum + new payload. */
            int sz = 1 + new_size;
            while (cur + sz > cap) { cap *= 2; out = (byte*)realloc(out, (size_t)cap); }
            out[cur] = eb[0];
            memcpy(out + cur + 1, repl, (size_t)new_size);
            new_entries[i].offset = (uint32_t)cur;
            new_entries[i].size   = (uint16_t)new_size;
            cur += sz;
            free(repl);
            decoded_count++;
        } else {
            int sz = 1 + e.size;
            while (cur + sz > cap) { cap *= 2; out = (byte*)realloc(out, (size_t)cap); }
            memcpy(out + cur, eb, (size_t)sz);
            new_entries[i].offset = (uint32_t)cur;
            new_entries[i].size   = e.size;
            cur += sz;
        }
    }

    /* Write the new table, then patch the header. */
    int new_table_offset = cur;
    int need = cur + 2 + res_count * (int)sizeof(dat_res_t);
    while (need > cap) { cap *= 2; out = (byte*)realloc(out, (size_t)cap); }
    uint16_t rc = res_count;
    memcpy(out + cur, &rc, 2); cur += 2;
    memcpy(out + cur, new_entries, (size_t)res_count * sizeof(dat_res_t));
    cur += res_count * (int)sizeof(dat_res_t);

    dat_header_t newh;
    newh.table_offset = (uint32_t)new_table_offset;
    newh.table_size   = (uint16_t)(2 + res_count * (int)sizeof(dat_res_t));
    memcpy(out, &newh, sizeof(newh));

    FILE* f = fopen(argv[2], "wb");
    if (!f) { perror(argv[2]); return 1; }
    if (fwrite(out, 1, (size_t)cur, f) != (size_t)cur) { perror("fwrite"); return 1; }
    fclose(f);
    fprintf(stderr, "%s -> %s: %d/%d resources pre-decoded, %d -> %d bytes\n",
            argv[1], argv[2], decoded_count, res_count, in_size, cur);

    free(out);
    free(new_entries);
    free(entries);
    free(in_buf);
    return 0;
}
