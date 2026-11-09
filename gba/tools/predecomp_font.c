/* Pre-decompress a PoP raw font (rawfont_type) at build time.
 *
 * The GBA runtime can blit pre-decoded sprites straight from ROM, but the
 * built-in fonts (hc_font_data / hc_small_font_data) ship compressed, so
 * decode_image() used to flag them gba_rom_compressed and the per-blit
 * decoder ran on every glyph — and a malformed/edge glyph spun that decoder
 * forever, freezing the game the moment any text was drawn (e.g. the first
 * move). This tool decodes every glyph to depth=8 / cmeth=0 once, so at
 * runtime decode_image() takes its no-decode fast path.
 *
 * Input/output are raw rawfont_type blobs:
 *   byte  first_char;
 *   byte  last_char;
 *   short height_above_baseline, height_below_baseline;
 *   short space_between_lines,   space_between_chars;
 *   word  offsets[n_chars];      // byte offset (from blob start) of each glyph
 *   ... glyph image records (image_data_type: {word h; word w; word flags; data[]})
 *
 * Usage: predecomp_font in.bin out.bin
 * Build: cc -O2 -o predecomp_font predecomp_font.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef uint8_t  byte;
typedef int8_t   sbyte;
typedef uint16_t word;

/* --- decoders: identical to tools/preproc_dat.c (size-tracking) --------- */
static int rle_lr(byte* dst, const byte* src, int src_avail, int dst_len) {
    int s = 0, d = 0;
    while (d < dst_len) {
        if (s >= src_avail) return -1;
        sbyte count = (sbyte)src[s++];
        if (count >= 0) {
            int n = count + 1;
            while (n-- > 0 && d < dst_len) { if (s >= src_avail) return -1; dst[d++] = src[s++]; }
        } else {
            int n = -count;
            if (s >= src_avail) return -1;
            byte v = src[s++];
            while (n-- > 0 && d < dst_len) dst[d++] = v;
        }
    }
    return s;
}
static int rle_ud(byte* dst, const byte* src, int src_avail, int dst_len, int width, int height) {
    int s = 0, rem_height = height, rem_length = dst_len, dest_end = dst_len - 1, w_step = width - 1;
    byte* dest_pos = dst;
    if (rem_length <= 0) return s;
    while (rem_length > 0) {
        if (s >= src_avail) return -1;
        sbyte count = (sbyte)src[s++];
        int n; byte v = 0; int rep = 0;
        if (count >= 0) n = count + 1; else { n = -count; if (s >= src_avail) return -1; v = src[s++]; rep = 1; }
        while (n > 0 && rem_length > 0) {
            if (!rep) { if (s >= src_avail) return -1; }
            if (dest_pos < dst || dest_pos >= dst + dst_len) return -1;
            *dest_pos = rep ? v : src[s++];
            dest_pos++; dest_pos += w_step;
            if (--rem_height == 0) { dest_pos -= dest_end; rem_height = height; }
            rem_length--; n--;
        }
    }
    return s;
}
static int lzg_lr(byte* dst, const byte* src, int src_avail, int dst_len) {
    byte window[0x400]; memset(window, 0, sizeof(window));
    int wp = 0x400 - 0x42, rem = dst_len, s = 0, d = 0; word mask = 0;
    while (rem > 0) {
        mask >>= 1;
        if ((mask & 0xFF00) == 0) { if (s >= src_avail) return -1; mask = (word)src[s++] | 0xFF00; }
        if (mask & 1) {
            if (s >= src_avail) return -1;
            byte v = src[s++]; window[wp++] = v; if (d >= dst_len) return -1; dst[d++] = v;
            if (wp >= 0x400) wp = 0; rem--;
        } else {
            if (s + 1 >= src_avail) return -1;
            word ci = src[s++]; ci = (ci << 8) | src[s++];
            int cs = ci & 0x3FF, cl = (ci >> 10) + 3;
            while (rem > 0 && cl > 0) {
                byte v = window[cs]; window[wp++] = v; if (d >= dst_len) return -1; dst[d++] = v;
                if (++cs >= 0x400) cs = 0; if (wp >= 0x400) wp = 0; rem--; cl--;
            }
        }
    }
    return s;
}
static int lzg_ud(byte* dst, const byte* src, int src_avail, int dst_len, int stride, int height) {
    byte window[0x400]; memset(window, 0, sizeof(window));
    int wp = 0x400 - 0x42, rem_h = height, rem_len = dst_len, dest_end = dst_len - 1, s = 0; word mask = 0;
    byte* dest_pos = dst;
    while (rem_len > 0) {
        mask >>= 1;
        if ((mask & 0xFF00) == 0) { if (s >= src_avail) return -1; mask = (word)src[s++] | 0xFF00; }
        if (mask & 1) {
            if (s >= src_avail) return -1;
            byte v = src[s++]; window[wp++] = v;
            if (dest_pos < dst || dest_pos >= dst + dst_len) return -1;
            *dest_pos = v; if (wp >= 0x400) wp = 0;
            dest_pos += stride; if (--rem_h == 0) { dest_pos -= dest_end; rem_h = height; } rem_len--;
        } else {
            if (s + 1 >= src_avail) return -1;
            word ci = src[s++]; ci = (ci << 8) | src[s++];
            int cs = ci & 0x3FF, cl = (ci >> 10) + 3;
            while (rem_len > 0 && cl > 0) {
                byte v = window[cs]; window[wp++] = v;
                if (dest_pos < dst || dest_pos >= dst + dst_len) return -1;
                *dest_pos = v; if (++cs >= 0x400) cs = 0; if (wp >= 0x400) wp = 0;
                dest_pos += stride; if (--rem_h == 0) { dest_pos -= dest_end; rem_h = height; } rem_len--; cl--;
            }
        }
    }
    return s;
}
static void unpack_to_8bpp(byte* dst, const byte* src, int width, int height, int stride, int depth) {
    int ppb = 8 / depth, mask = (1 << depth) - 1;
    for (int y = 0; y < height; ++y) {
        const byte* in = src + (size_t)y * stride;
        byte* out = dst + (size_t)y * width;
        int x = 0;
        for (int xb = 0; xb < stride; ++xb) {
            byte v = *in++; int shift = 8;
            for (int p = 0; p < ppb && x < width; ++p, ++x) { shift -= depth; *out++ = (byte)((v >> shift) & mask); }
        }
    }
}

static byte* slurp(const char* path, int* n) {
    FILE* f = fopen(path, "rb"); if (!f) return NULL;
    fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
    byte* b = malloc((size_t)sz); if (!b) { fclose(f); return NULL; }
    if (fread(b, 1, (size_t)sz, f) != (size_t)sz) { free(b); fclose(f); return NULL; }
    fclose(f); *n = (int)sz; return b;
}
static word rd16(const byte* p) { return (word)(p[0] | (p[1] << 8)); }
static void wr16(byte* p, word v) { p[0] = v & 0xFF; p[1] = (v >> 8) & 0xFF; }

int main(int argc, char** argv) {
    if (argc != 3) { fprintf(stderr, "usage: %s in.bin out.bin\n", argv[0]); return 2; }
    int in_size = 0;
    byte* in = slurp(argv[1], &in_size);
    if (!in) { perror(argv[1]); return 1; }
    if (in_size < 10) { fprintf(stderr, "%s: too small for a rawfont\n", argv[1]); return 1; }

    int first = in[0], last = in[1];
    int n = last - first + 1;
    if (n < 1 || n > 256) { fprintf(stderr, "bad char range %d..%d\n", first, last); return 1; }
    int header = 2 + 8;                 /* first,last + 4 shorts */
    if (header + n * 2 > in_size) { fprintf(stderr, "offsets truncated\n"); return 1; }

    /* Output: same 10-byte header, fresh offset table, then decoded glyphs. */
    int cap = header + n * 2 + n * (6 + 400 * 256) ;  /* generous upper bound */
    byte* out = calloc(1, (size_t)cap);
    if (!out) { perror("calloc"); return 1; }
    memcpy(out, in, header);            /* copy first,last + spacing shorts */
    int cur = header + n * 2;           /* glyph records start after offsets */
    int decoded = 0, passthrough = 0;

    for (int i = 0; i < n; ++i) {
        word off = rd16(in + header + i * 2);
        wr16(out + header + i * 2, (word)cur);   /* record this glyph's new offset */
        if (off + 6 > (word)in_size) {           /* missing glyph: emit 1x1 blank */
            wr16(out + cur + 0, 1); wr16(out + cur + 2, 1);
            wr16(out + cur + 4, (word)(7 << 12)); out[cur + 6] = 0; cur += 7; continue;
        }
        const byte* g = in + off;
        int gh = rd16(g + 0), gw = rd16(g + 2), flags = rd16(g + 4);
        int depth = ((flags >> 12) & 7) + 1;
        int cmeth = (flags >> 8) & 0x0F;
        int h = gh ? gh : 1;                     /* decode_image treats h==0 as 1 */
        const byte* cdata = g + 6;
        int cdata_avail = in_size - (int)(off + 6);
        if (gw < 1 || gw > 400 || h > 256 || depth < 1 || depth > 8 || cmeth > 4 || cdata_avail < 0) {
            /* Unclassifiable: keep the original record verbatim. */
            int rest = in_size - (int)off;
            if (rest < 6) rest = 6;
            memcpy(out + cur, g, (size_t)rest);
            cur += rest; passthrough++; continue;
        }
        int stride = (depth * gw + 7) / 8;
        int dsize = stride * h;
        byte* dec = calloc(1, (size_t)(dsize > 0 ? dsize : 1));
        int used = -1;
        switch (cmeth) {
            case 0: if (cdata_avail >= dsize) { memcpy(dec, cdata, (size_t)dsize); used = dsize; } break;
            case 1: used = rle_lr(dec, cdata, cdata_avail, dsize); break;
            case 2: used = rle_ud(dec, cdata, cdata_avail, dsize, stride, h); break;
            case 3: used = lzg_lr(dec, cdata, cdata_avail, dsize); break;
            case 4: used = lzg_ud(dec, cdata, cdata_avail, dsize, stride, h); break;
        }
        /* Emit a depth=8 cmeth=0 record (blank-filled if decode failed). */
        wr16(out + cur + 0, (word)h);
        wr16(out + cur + 2, (word)gw);
        wr16(out + cur + 4, (word)((flags & 0x00FF) | (7 << 12)));
        byte* px = out + cur + 6;
        if (used >= 0) {
            if (depth == 8) memcpy(px, dec, (size_t)gw * h);
            else            unpack_to_8bpp(px, dec, gw, h, stride, depth);
            decoded++;
        } else {
            memset(px, 0, (size_t)gw * h);   /* couldn't decode: transparent glyph */
        }
        cur += 6 + gw * h;
        free(dec);
    }

    FILE* f = fopen(argv[2], "wb");
    if (!f) { perror(argv[2]); return 1; }
    fwrite(out, 1, (size_t)cur, f);
    fclose(f);
    fprintf(stderr, "%s: %d glyphs decoded, %d passed through, %d bytes\n",
            argv[2], decoded, passthrough, cur);
    return 0;
}
