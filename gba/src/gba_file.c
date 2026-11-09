/* File I/O over linked data blobs on GBA.
   We never open real files on the cartridge. fopen() is intercepted by the
   common.h preprocessor (#define fopen gba_fopen) so unchanged upstream code
   that does fopen("KID","rb") gets a memory-backed FILE*.

   Each DAT file (KID, PRINCE, LEVELS, ...) is converted by `gbafix bin2s`
   (handled by the Makefile) into _start/_end/_size symbols. We list them in
   the table below; the linker will resolve them from data/<name>.bin.o. */
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "gba_port.h"

/* bin2s exports `<stem>_bin[]` (start) and `<stem>_bin_end[]` for each blob.
   We compute the size as (end - start). */
#define BLOB_DECL(sym)                  \
    extern const uint8_t sym[];         \
    extern const uint8_t sym##_end[];
BLOB_DECL(kid_bin)
BLOB_DECL(prince_bin)
BLOB_DECL(title_bin)
BLOB_DECL(levels_bin)
BLOB_DECL(guard_bin)
BLOB_DECL(guard1_bin)
BLOB_DECL(guard2_bin)
BLOB_DECL(vpalace_bin)
BLOB_DECL(vdungeon_bin)
BLOB_DECL(vizier_bin)
BLOB_DECL(fat_bin)
BLOB_DECL(skel_bin)
BLOB_DECL(shadow_bin)
BLOB_DECL(pv_bin)
BLOB_DECL(digisnd1_bin)
BLOB_DECL(digisnd2_bin)
BLOB_DECL(digisnd3_bin)
BLOB_DECL(ibm_snd1_bin)
BLOB_DECL(ibm_snd2_bin)
BLOB_DECL(midisnd1_bin)
BLOB_DECL(midisnd2_bin)
BLOB_DECL(font_bin)
#undef BLOB_DECL

static const struct {
    const char*    name;
    const uint8_t* data;
    const uint8_t* end;
} s_blobs[] = {
#define ENT(sym, name) { name, sym, sym##_end }
    ENT(kid_bin,      "KID"),       ENT(kid_bin,      "KID.DAT"),
    ENT(prince_bin,   "PRINCE"),    ENT(prince_bin,   "PRINCE.DAT"),
    ENT(title_bin,    "TITLE"),     ENT(title_bin,    "TITLE.DAT"),
    ENT(levels_bin,   "LEVELS"),    ENT(levels_bin,   "LEVELS.DAT"),
    ENT(guard_bin,    "GUARD"),     ENT(guard_bin,    "GUARD.DAT"),
    ENT(guard1_bin,   "GUARD1"),    ENT(guard1_bin,   "GUARD1.DAT"),
    ENT(guard2_bin,   "GUARD2"),    ENT(guard2_bin,   "GUARD2.DAT"),
    ENT(vpalace_bin,  "VPALACE"),   ENT(vpalace_bin,  "VPALACE.DAT"),
    ENT(vdungeon_bin, "VDUNGEON"),  ENT(vdungeon_bin, "VDUNGEON.DAT"),
    ENT(vizier_bin,   "VIZIER"),    ENT(vizier_bin,   "VIZIER.DAT"),
    ENT(fat_bin,      "FAT"),       ENT(fat_bin,      "FAT.DAT"),
    ENT(skel_bin,     "SKEL"),      ENT(skel_bin,     "SKEL.DAT"),
    ENT(shadow_bin,   "SHADOW"),    ENT(shadow_bin,   "SHADOW.DAT"),
    ENT(pv_bin,       "PV"),        ENT(pv_bin,       "PV.DAT"),
    ENT(digisnd1_bin, "DIGISND1.DAT"),
    ENT(digisnd2_bin, "DIGISND2.DAT"),
    ENT(digisnd3_bin, "DIGISND3.DAT"),
    ENT(ibm_snd1_bin, "IBM_SND1"),     ENT(ibm_snd1_bin, "IBM_SND1.DAT"),
    ENT(ibm_snd2_bin, "IBM_SND2"),     ENT(ibm_snd2_bin, "IBM_SND2.DAT"),
    ENT(midisnd1_bin, "MIDISND1"),     ENT(midisnd1_bin, "MIDISND1.DAT"),
    ENT(midisnd2_bin, "MIDISND2"),     ENT(midisnd2_bin, "MIDISND2.DAT"),
    ENT(font_bin,     "font"),
#undef ENT
};

/* Strip any "data/" or "../" prefix and uppercase the filename so case
   variations work. Returns a pointer into a static buffer. */
static const char* canonicalize(const char* path) {
    static char buf[64];
    const char* p = path;
    /* trim leading directories */
    const char* slash = strrchr(p, '/');
    if (slash) p = slash + 1;
    /* dot-extensions stay case-insensitive but exact match is preferred */
    size_t n = strlen(p);
    if (n >= sizeof(buf)) n = sizeof(buf) - 1;
    for (size_t i = 0; i < n; ++i) {
        buf[i] = (char)toupper((unsigned char)p[i]);
    }
    buf[n] = '\0';
    /* Special case: "font" stays lower-case in the table. */
    if (n == 4 && memcmp(buf, "FONT", 4) == 0) memcpy(buf, "font", 4);
    return buf;
}

const gba_blob_t* gba_find_blob(const char* path) {
    static gba_blob_t out;
    const char* key = canonicalize(path);
    for (unsigned i = 0; i < sizeof(s_blobs)/sizeof(s_blobs[0]); ++i) {
        if (strcmp(s_blobs[i].name, key) == 0) {
            out.name  = s_blobs[i].name;
            out.bytes = s_blobs[i].data;
            out.size  = (size_t)(s_blobs[i].end - s_blobs[i].data);
            return &out;
        }
    }
    return NULL;
}

int gba_access(const char* path) {
    return gba_find_blob(path) ? 0 : -1;
}

/* --- Memory-backed FILE* ----------------------------------------------- */
/* We don't have a real libc FILE on devkitARM newlib that we can subclass.
   Instead we maintain a parallel handle table keyed by the FILE* address. */

#define SAVE_FILE_NAME "PRINCE.SAV"
#define SAVE_BUF_CAP   4096

typedef struct {
    const uint8_t* base;    /* read-only blob backing, NULL for save buffer */
    uint8_t*       wbuf;    /* write-mode scratch (points into s_save_buf) */
    size_t         wcap;
    size_t         size;
    size_t         pos;
    int            in_use;
    int            is_write;
    int            is_save;
    FILE*          public_handle;
} gba_file_t;

#define MAX_FILES 8

extern int gba_sram_read(void* data, size_t cap, size_t* out_len);
extern int gba_sram_write(const void* data, size_t len);
static gba_file_t s_files[MAX_FILES];

/* Single static save scratch buffer. Only one PRINCE.SAV handle is ever open
   at a time (open -> read/write -> close), so a single static buffer suffices
   and we avoid a heap allocation. s_save_in_use guards against the (never
   reached in practice) case of two concurrent save handles. */
static uint8_t s_save_buf[SAVE_BUF_CAP] __attribute__((aligned(4)));
static int     s_save_in_use;

/* Per-slot dummy byte: gives each FILE* a unique, stable address without
   allocating. find_handle() keys on this pointer. */
static uint8_t s_handle_tokens[MAX_FILES];

static gba_file_t* find_handle(FILE* fp) {
    for (int i = 0; i < MAX_FILES; ++i)
        if (s_files[i].in_use && s_files[i].public_handle == fp) return &s_files[i];
    return NULL;
}

static int strcasecmp_local(const char* a, const char* b) {
    while (*a && *b) {
        int ca = tolower((unsigned char)*a++);
        int cb = tolower((unsigned char)*b++);
        if (ca != cb) return ca - cb;
    }
    return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}

static int path_is_save(const char* p) {
    const char* base = strrchr(p, '/');
    base = base ? base + 1 : p;
    return strcasecmp_local(base, SAVE_FILE_NAME) == 0;
}

static gba_file_t* alloc_slot(void) {
    for (int i = 0; i < MAX_FILES; ++i) {
        if (!s_files[i].in_use) {
            memset(&s_files[i], 0, sizeof(s_files[i]));
            s_files[i].in_use = 1;
            /* Unique, stable, allocation-free handle: the slot's token byte. */
            s_files[i].public_handle = (FILE*)&s_handle_tokens[i];
            return &s_files[i];
        }
    }
    return NULL;
}

FILE* gba_fopen(const char* path, const char* mode) {
    int writing = (mode && (strchr(mode, 'w') || strchr(mode, 'a')));
    int saving  = path_is_save(path);

    if (saving) {
        if (s_save_in_use) return NULL; /* only one save handle at a time */
        gba_file_t* h = alloc_slot();
        if (!h) return NULL;
        h->is_save = 1;
        h->wcap    = SAVE_BUF_CAP;
        h->wbuf    = s_save_buf;
        s_save_in_use = 1;
        memset(s_save_buf, 0, SAVE_BUF_CAP);
        if (writing) {
            h->is_write = 1;
            h->size = 0;
        } else {
            size_t got = 0;
            if (gba_sram_read(h->wbuf, h->wcap, &got) == 0) {
                h->size = got;
            } else {
                /* no save present */
                s_save_in_use = 0;
                h->in_use = 0;
                return NULL;
            }
        }
        return h->public_handle;
    }

    if (writing) return NULL; /* no general writable filesystem on GBA */

    const gba_blob_t* blob = gba_find_blob(path);
    if (!blob) return NULL;
    gba_file_t* h = alloc_slot();
    if (!h) return NULL;
    h->base = blob->bytes;
    h->size = blob->size;
    return h->public_handle;
}

int gba_fclose(FILE* fp) {
    gba_file_t* h = find_handle(fp);
    if (!h) return EOF;
    if (h->is_save && h->is_write) {
        gba_sram_write(h->wbuf, h->size);
    }
    if (h->is_save) s_save_in_use = 0;
    memset(h, 0, sizeof(*h));
    return 0;
}

size_t gba_fread(void* buf, size_t sz, size_t n, FILE* fp) {
    gba_file_t* h = find_handle(fp);
    if (!h) return 0;
    size_t want = sz * n;
    size_t have = (h->size > h->pos) ? h->size - h->pos : 0;
    if (want > have) want = have;
    const uint8_t* src = h->is_save ? h->wbuf : h->base;
    memcpy(buf, src + h->pos, want);
    h->pos += want;
    return sz ? (want / sz) : 0;
}

size_t gba_fwrite(const void* buf, size_t sz, size_t n, FILE* fp) {
    gba_file_t* h = find_handle(fp);
    if (!h || !h->is_save || !h->is_write) return 0;
    size_t want = sz * n;
    if (h->pos + want > h->wcap) want = h->wcap - h->pos;
    memcpy(h->wbuf + h->pos, buf, want);
    h->pos += want;
    if (h->pos > h->size) h->size = h->pos;
    return sz ? (want / sz) : 0;
}

int gba_fseek(FILE* fp, long ofs, int whence) {
    gba_file_t* h = find_handle(fp);
    if (!h) return -1;
    long target;
    if (whence == SEEK_SET) target = ofs;
    else if (whence == SEEK_CUR) target = (long)h->pos + ofs;
    else if (whence == SEEK_END) target = (long)h->size + ofs;
    else return -1;
    if (target < 0) target = 0;
    if ((size_t)target > h->size) target = (long)h->size;
    h->pos = (size_t)target;
    return 0;
}

long gba_ftell(FILE* fp) {
    gba_file_t* h = find_handle(fp);
    return h ? (long)h->pos : -1;
}

int gba_feof(FILE* fp) {
    gba_file_t* h = find_handle(fp);
    return h && h->pos >= h->size;
}

const uint8_t* gba_file_rom_ptr(FILE* fp, size_t* out_avail) {
    gba_file_t* h = find_handle(fp);
    if (!h || !h->base) {
        if (out_avail) *out_avail = 0;
        return NULL;
    }
    size_t avail = (h->size > h->pos) ? h->size - h->pos : 0;
    if (out_avail) *out_avail = avail;
    return h->base + h->pos;
}

/* ---- Static-pool allocator -----------------------------------------------
   Replaces newlib's heap with a fixed-size linked-block allocator backed by
   a BSS-resident pool. Goals:
     * No sbrk / no growing heap that competes with stack.
     * Bounded RAM usage so we can fit on the GBA's 256 KB EWRAM.
     * Free-list reuse so chtab churn between levels doesn't leak.
     * Pointers from gba_file_rom_ptr (>= 0x08000000) are silently ignored on
       free() — they live in cartridge ROM and have no allocator metadata.
   The allocator is intentionally simple (first-fit, single coalesce on free);
   PoP is single-threaded so we don't need locking. */

/* Peak boot-time demand, with ROM-direct sprites (no per-sprite pixel buffer):
     onscreen_surface_  320x192x8bpp .......... ~61 KB
     ~250 SDL_Surface structs @ ~170 B ......... ~45 KB
     chtab tables / dat_type handles ........... ~10 KB
     peel surfaces + copyprot dialog scratch ... ~15 KB
   i.e. ~130 KB peak. The old 96 KB pool ran dry mid-sprite-load, so
   SDL_CreateRGBSurface returned NULL and the boot hung on a black screen.
   160 KB gives ~30 KB of headroom. This is the ONLY general allocator in the
   build: newlib's heap is replaced by a tiny bounded static sbrk (below), and
   the public malloc/calloc/realloc/free are --wrap'd onto this pool, so the
   ROM performs no growing-heap allocation. */
#define GBA_POOL_BYTES (196 * 1024)
static uint8_t  gba_pool[GBA_POOL_BYTES] __attribute__((aligned(8)));

extern void gba_panic(uint8_t color_idx); /* gba_video.c: solid screen + halt */

typedef struct block_hdr {
    uint32_t          size;   /* payload size in bytes, bit 0 = in-use */
    struct block_hdr* prev;   /* doubly-linked free list (only meaningful when free) */
    struct block_hdr* next;
} block_hdr_t;

#define HDR_SIZE   ((uint32_t)sizeof(block_hdr_t))
#define ALIGN_UP(n) (((n) + 7u) & ~7u)

static block_hdr_t* g_free_list = NULL;
static int          g_pool_inited = 0;

static void pool_init(void) {
    block_hdr_t* h = (block_hdr_t*)gba_pool;
    h->size = (GBA_POOL_BYTES - HDR_SIZE) & ~1u;
    h->prev = NULL;
    h->next = NULL;
    g_free_list = h;
    g_pool_inited = 1;
}

static inline block_hdr_t* hdr_of(void* p) {
    return (block_hdr_t*)((uint8_t*)p - HDR_SIZE);
}
static inline void* payload_of(block_hdr_t* h) {
    return (void*)((uint8_t*)h + HDR_SIZE);
}

static void freelist_remove(block_hdr_t* h) {
    if (h->prev) h->prev->next = h->next;
    else         g_free_list   = h->next;
    if (h->next) h->next->prev = h->prev;
}
static void freelist_push(block_hdr_t* h) {
    h->prev = NULL;
    h->next = g_free_list;
    if (g_free_list) g_free_list->prev = h;
    g_free_list = h;
}

/* First-fit over the free list; returns a payload or NULL (no panic). */
static void* pool_first_fit(uint32_t need) {
    for (block_hdr_t* h = g_free_list; h != NULL; h = h->next) {
        if ((h->size & ~1u) >= need) {
            uint32_t blk = h->size & ~1u;
            freelist_remove(h);
            /* Split if the remainder is big enough to hold a header + 8B. */
            if (blk >= need + HDR_SIZE + 16) {
                block_hdr_t* rem = (block_hdr_t*)((uint8_t*)h + HDR_SIZE + need);
                rem->size = (blk - need - HDR_SIZE) & ~1u;
                freelist_push(rem);
                h->size = need | 1u;
            } else {
                h->size = blk | 1u;
            }
            return payload_of(h);
        }
    }
    return NULL;
}

/* Full bidirectional defragmentation. __wrap_free() only coalesces with the
   *following* block, so per-frame churn (e.g. the peel surfaces created while
   the kid runs) shreds the pool into many tiny free blocks — we observed 24 KB
   free split into 64 fragments, largest only 948 B, which OOM-panicked a peel
   alloc mid-run. This walks the pool in physical order and merges every run of
   adjacent free blocks, rebuilding the free list. Run on alloc failure. */
static void pool_coalesce_all(void) {
    g_free_list = NULL;
    uint8_t* end = gba_pool + GBA_POOL_BYTES;
    block_hdr_t* h = (block_hdr_t*)gba_pool;
    while ((uint8_t*)h < end) {
        uint32_t hsz = h->size & ~1u;
        block_hdr_t* nextp = (block_hdr_t*)((uint8_t*)h + HDR_SIZE + hsz);
        if (h->size & 1u) {            /* in use: skip */
            h = nextp;
            continue;
        }
        /* free: absorb all immediately-following free blocks */
        while ((uint8_t*)nextp < end && !(nextp->size & 1u)) {
            hsz += HDR_SIZE + (nextp->size & ~1u);
            nextp = (block_hdr_t*)((uint8_t*)h + HDR_SIZE + hsz);
        }
        h->size = hsz & ~1u;           /* mark free, store merged size */
        freelist_push(h);
        h = nextp;
    }
}

void* __wrap_malloc(size_t n) {
    if (!g_pool_inited) pool_init();
    if (n == 0) return NULL;
    uint32_t need = ALIGN_UP((uint32_t)n);

    void* p = pool_first_fit(need);
    if (p) return p;

    /* No single block big enough — defragment and try once more before
       giving up. This recovers the fragmented-but-ample case. */
    pool_coalesce_all();
    p = pool_first_fit(need);
    if (p) return p;

    /* Genuinely out of pool. Halt visibly so the condition is diagnosable
       rather than letting upstream deref a NULL surface. */
    gba_panic(3);
    return NULL;
}

void* __wrap_calloc(size_t nmemb, size_t size) {
    size_t n = nmemb * size;
    void* p = __wrap_malloc(n);
    if (p) memset(p, 0, n);
    return p;
}

void __wrap_free(void* p) {
    if (!p) return;
    if ((uintptr_t)p >= 0x08000000u) return;          /* ROM pointer: no-op */
    if ((uintptr_t)p <  (uintptr_t)gba_pool ||
        (uintptr_t)p >= (uintptr_t)(gba_pool + GBA_POOL_BYTES)) return; /* not ours */
    block_hdr_t* h = hdr_of(p);
    h->size &= ~1u;
    /* Coalesce with the immediately-following block if it's also free. */
    block_hdr_t* nxt = (block_hdr_t*)((uint8_t*)h + HDR_SIZE + (h->size & ~1u));
    if ((uint8_t*)nxt < (gba_pool + GBA_POOL_BYTES) && !(nxt->size & 1u)) {
        freelist_remove(nxt);
        h->size += HDR_SIZE + (nxt->size & ~1u);
    }
    freelist_push(h);
}

void* __wrap_realloc(void* p, size_t n) {
    if (!p)     return __wrap_malloc(n);
    if (n == 0) { __wrap_free(p); return NULL; }
    /* GBA path: assume in-pool, copy & free. PoP build doesn't actually
       exercise this — replay/midi/stb_vorbis are excluded. */
    if ((uintptr_t)p >= 0x08000000u) {
        void* q = __wrap_malloc(n);
        if (q) memcpy(q, p, n);
        return q;
    }
    block_hdr_t* h = hdr_of(p);
    uint32_t old = h->size & ~1u;
    if (old >= n) return p;
    void* q = __wrap_malloc(n);
    if (!q) return NULL;
    memcpy(q, p, old);
    __wrap_free(p);
    return q;
}

void gba_free_safe(void* p) { __wrap_free(p); }

/* Named entry points so the GBA port layer (sdl_shim.c, gba_file.c) can call
   the static pool explicitly, without using the malloc/calloc/realloc tokens.
   Upstream src/*.c still uses those tokens, transparently routed here by the
   linker's --wrap. */
void* gba_alloc(size_t n)                 { return __wrap_malloc(n); }
void* gba_calloc(size_t nmemb, size_t sz) { return __wrap_calloc(nmemb, sz); }
void* gba_realloc(void* p, size_t n)      { return __wrap_realloc(p, n); }
void  gba_free(void* p)                   { __wrap_free(p); }

/* ---- Bounded static newlib heap -----------------------------------------
   Newlib's *printf family can reach _malloc_r (e.g. _dtoa for %f, or
   __submore), which goes through _sbrk_r -> the heap window the linker
   reserved between __end__ and __eheap_end. That window is static on GBA but
   unbounded up to end-of-EWRAM, and competes with our pool. Override _sbrk_r
   with a tiny fixed static arena so any such allocation is bounded and fully
   static; the ROM then contains no growing heap at all. SDLPoP's GBA path
   formats only ints/strings, so this is rarely (if ever) touched. */
#define GBA_NEWLIB_HEAP_BYTES (8 * 1024)
static uint8_t s_newlib_heap[GBA_NEWLIB_HEAP_BYTES] __attribute__((aligned(8)));

struct _reent;
void* _sbrk_r(struct _reent* reent, ptrdiff_t incr) {
    (void)reent;
    static uint8_t* brk = s_newlib_heap;
    uint8_t* prev = brk;
    if (incr < 0) {
        brk += incr;
        if (brk < s_newlib_heap) brk = s_newlib_heap;
        return brk;
    }
    if (prev + incr > s_newlib_heap + sizeof(s_newlib_heap)) {
        return (void*)-1; /* ENOMEM; newlib malloc returns NULL */
    }
    brk += incr;
    return prev;
}

int gba_fgetc(FILE* fp) {
    gba_file_t* h = find_handle(fp);
    if (!h || h->pos >= h->size) return EOF;
    const uint8_t* src = h->is_save ? h->wbuf : h->base;
    return (int)src[h->pos++];
}

int gba_fputc(int c, FILE* fp) { (void)c; (void)fp; return EOF; }
int gba_ferror(FILE* fp)       { (void)fp; return 0; }

/* Stat-shaped emulation: SDLPoP uses stat() to distinguish files from
   directories. On GBA every blob is a "regular file"; nothing is a directory.
   The actual struct stat layout is provided by newlib; we just fill mode and
   size at the standard offsets via the public API. */
#include <sys/stat.h>
#undef stat   /* common.h redefines this; undefine for the wrapper body */
int gba_stat_compat(const char* path, struct stat* out) {
    const gba_blob_t* blob = gba_find_blob(path);
    if (!blob || !out) return -1;
    memset(out, 0, sizeof(*out));
    out->st_mode = 0100000u; /* S_IFREG */
    out->st_size = (long)blob->size;
    return 0;
}
