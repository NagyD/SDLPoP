/* Minimal, self-contained printf-to-string family for the GBA build.
 *
 * devkitARM's newlib vfprintf-to-string (_svfprintf_r / __ssputs_r /
 * __ssprint_r / __locale_mb_cur_max) spins forever on this target — it froze
 * the game whenever a snprintf ran during gameplay (e.g. a resource filename
 * built in the open_dat path on the first move). Rather than fight newlib's
 * locale/multibyte machinery, route the game's snprintf/vsnprintf/sprintf/
 * vsprintf through this tiny formatter via the Makefile's -Wl,--wrap.
 *
 * Supports the specifiers SDLPoP actually uses: %d %i %u %x %X %c %s %p %%,
 * an optional field width with optional '0' pad, and the 'l'/'h' length
 * modifiers (parsed and ignored — values fit in int/long the same here).
 * Floating point (%f) is not used on GBA (only under CHECK_TIMING).
 */
#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>

struct sink { char* buf; size_t cap; size_t len; };

static void emit(struct sink* s, char c) {
    if (s->len + 1 < s->cap) s->buf[s->len] = c;
    s->len++;
}
static void emit_str(struct sink* s, const char* p, int width, int left, char pad) {
    int n = 0;
    const char* q = p ? p : "(null)";
    while (q[n]) n++;
    if (!left) for (int i = n; i < width; ++i) emit(s, pad);
    for (int i = 0; i < n; ++i) emit(s, q[i]);
    if (left) for (int i = n; i < width; ++i) emit(s, ' ');
}
static void emit_num(struct sink* s, unsigned long v, int base, int is_neg,
                     int upper, int width, int left, char pad) {
    char tmp[32];
    int n = 0;
    const char* digs = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    if (v == 0) tmp[n++] = '0';
    while (v) { tmp[n++] = digs[v % base]; v /= base; }
    if (is_neg) tmp[n++] = '-';
    /* zero-pad goes after the sign; space-pad before. */
    int total = n;
    if (!left && pad == '0') {
        /* print sign first, then zeros */
        int sign = is_neg ? 1 : 0;
        if (is_neg) { emit(s, '-'); n--; }
        for (int i = total; i < width; ++i) emit(s, '0');
        for (int i = n - 1; i >= 0; --i) emit(s, tmp[i]);
        (void)sign;
        return;
    }
    if (!left) for (int i = total; i < width; ++i) emit(s, ' ');
    for (int i = n - 1; i >= 0; --i) emit(s, tmp[i]);
    if (left) for (int i = total; i < width; ++i) emit(s, ' ');
}

int __wrap_vsnprintf(char* buf, size_t size, const char* fmt, va_list ap) {
    struct sink s = { buf, size, 0 };
    for (const char* p = fmt; *p; ++p) {
        if (*p != '%') { emit(&s, *p); continue; }
        ++p;
        int left = 0; char pad = ' '; int width = 0;
        for (;; ++p) {
            if (*p == '-') left = 1;
            else if (*p == '0') pad = '0';
            else if (*p == '+' || *p == ' ' || *p == '#') { /* ignore */ }
            else break;
        }
        while (*p >= '0' && *p <= '9') { width = width * 10 + (*p - '0'); ++p; }
        if (*p == '.') { ++p; while (*p >= '0' && *p <= '9') ++p; } /* precision: skip */
        int lng = 0;
        while (*p == 'l' || *p == 'h' || *p == 'z') { if (*p == 'l') lng++; ++p; }
        switch (*p) {
            case 'd': case 'i': {
                long v = lng ? va_arg(ap, long) : (long)va_arg(ap, int);
                int neg = v < 0;
                unsigned long uv = neg ? (unsigned long)(-v) : (unsigned long)v;
                emit_num(&s, uv, 10, neg, 0, width, left, pad);
            } break;
            case 'u': {
                unsigned long v = lng ? va_arg(ap, unsigned long) : (unsigned long)va_arg(ap, unsigned int);
                emit_num(&s, v, 10, 0, 0, width, left, pad);
            } break;
            case 'x': case 'X': {
                unsigned long v = lng ? va_arg(ap, unsigned long) : (unsigned long)va_arg(ap, unsigned int);
                emit_num(&s, v, 16, 0, *p == 'X', width, left, pad);
            } break;
            case 'p': {
                uintptr_t v = (uintptr_t)va_arg(ap, void*);
                emit(&s, '0'); emit(&s, 'x');
                emit_num(&s, (unsigned long)v, 16, 0, 0, width, left, pad);
            } break;
            case 'c': emit(&s, (char)va_arg(ap, int)); break;
            case 's': emit_str(&s, va_arg(ap, const char*), width, left, ' '); break;
            case '%': emit(&s, '%'); break;
            case '\0': goto done;
            default:  emit(&s, '%'); emit(&s, *p); break;
        }
    }
done:
    if (s.cap) s.buf[s.len < s.cap ? s.len : s.cap - 1] = '\0';
    return (int)s.len;
}

int __wrap_snprintf(char* buf, size_t size, const char* fmt, ...) {
    va_list ap; va_start(ap, fmt);
    int r = __wrap_vsnprintf(buf, size, fmt, ap);
    va_end(ap);
    return r;
}
int __wrap_vsprintf(char* buf, const char* fmt, va_list ap) {
    return __wrap_vsnprintf(buf, (size_t)0x7fffffff, fmt, ap);
}
int __wrap_sprintf(char* buf, const char* fmt, ...) {
    va_list ap; va_start(ap, fmt);
    int r = __wrap_vsnprintf(buf, (size_t)0x7fffffff, fmt, ap);
    va_end(ap);
    return r;
}
