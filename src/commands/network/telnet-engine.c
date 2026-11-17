/* Clarice allocation and parser-observation boundary, not another parser. */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <limits.h>
extern void *cli_alloc(unsigned long);
extern void cli_free(void *);
typedef struct { size_t size; } Owned;
static void *owned_malloc(size_t n) {
 Owned *p;
 if (n > ULONG_MAX - sizeof *p) return NULL;
 p = cli_alloc(n + sizeof *p);
 if (!p) return NULL;
 p->size = n; return p + 1;
}
static void owned_free(void *p) { if (p) cli_free((Owned *)p - 1); }
static void *owned_calloc(size_t n, size_t size) {
 void *p;
 if (size && n > ULONG_MAX / size) return NULL;
 p = owned_malloc(n * size); if (p) memset(p, 0, n * size); return p;
}
static void *owned_realloc(void *old, size_t n) {
 void *p = owned_malloc(n);
 if (!p) return NULL;
 if (old) { size_t before = ((Owned *)old - 1)->size;
  memcpy(p, old, before < n ? before : n); owned_free(old); }
 return p;
}
/* errno is not supplied by cli_alloc. Avoid pulling in a second libc for
   allocation diagnostics; upstream still emits its full error/event. */
#undef errno
#define errno 0
#define strerror(e) "allocation failed"
#define malloc owned_malloc
#define calloc owned_calloc
#define realloc owned_realloc
#define free owned_free
#include "../vendor/libtelnet/libtelnet.c"
#undef malloc
#undef calloc
#undef realloc
#undef free
#undef strerror
int clarice_telnet_pending(telnet_t *t) {
 return t && t->state != TELNET_STATE_DATA;
}
int clarice_telnet_enabled(telnet_t *t, unsigned char option, int local) {
 telnet_rfc1143_t q = _get_rfc1143(t, option);
 return (local ? Q_US(q) : Q_HIM(q)) == Q_YES;
}
