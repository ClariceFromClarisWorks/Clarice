/* Minimal declarations/bindings for selected genuine BusyBox sources.
   This is a Clarice port boundary, not a replacement implementation of libbb. */
#ifndef CLARICE_BUSYBOX_H
#define CLARICE_BUSYBOX_H
#include "config.h"
#include "../terminal/ncurses-compat.h"
#include "termios.h"
#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <ctype.h>
#include "keys.h"
#define optind bb_optind
extern int bb_optind;
#define FAST_FUNC
#define ALIGN1
#define ALWAYS_INLINE inline
#define UNUSED_PARAM __attribute__((unused))
#define MAIN_EXTERNALLY_VISIBLE
#define TRUE 1
#define FALSE 0
#define STRERROR_FMT "%s"
#define STRERROR_ERRNO , strerror(errno)
void *memrchr(const void *,int,size_t);
char *skip_whitespace(const char *),*skip_non_whitespace(const char *);
#ifdef CLARICE_VI_PROFILE
extern unsigned long cli_ticks(void),bb_profile[5];
#endif
#define BB_VER "1.37.0"
#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))
typedef signed char smallint;
typedef unsigned char smalluint;
struct globals;
extern struct globals *ptr_to_globals;
#define SET_PTR_TO_GLOBALS(p) (ptr_to_globals=(p))
typedef struct llist_t {struct llist_t *link; char *data;} llist_t;
void llist_add_to_end(llist_t **,void *);
void *llist_pop(llist_t **);
void *xmalloc(size_t),*xzalloc(size_t),*xrealloc(void *,size_t);
char *xstrdup(const char *),*xstrndup(const char *,size_t),*xasprintf(const char *,...);
int index_in_strings(const char *,const char *);
unsigned bb_strtou(const char *,char **,int);
char *is_prefixed_with(const char *,const char *);
char *strchrnul(const char *,int);
void bb_simple_error_msg_and_die(const char *) __attribute__((noreturn));
void bb_show_usage(void);
int bb_vi_options(int,char **,llist_t **);
void bb_preserve(const char *,const char *,size_t);
int bb_save_file(const char *,const char *,size_t);
int bb_access(const char *,int),bb_stat(const char *,struct stat *);
#define access bb_access
#define stat(p,s) bb_stat(p,s)
void bb_out(const void *,size_t);
void bb_putchar(int);
void fputs_stdout(const char *);
void fflush_all(void);
#define TERMIOS_RAW_CRNL 6
void set_termios_to_raw(int,struct termios *,int);
void tcsetattr_stdin_TCSANOW(struct termios *);
int get_terminal_width_height(int,unsigned *,unsigned *);
struct pollfd {int fd;short events,revents;};
#define POLLIN 1
int bb_poll(struct pollfd *,unsigned,int);
#define poll bb_poll
#define safe_poll bb_poll
ssize_t bb_read(int,void *,size_t),bb_write(int,const void *,size_t);
#undef read
#undef write
#define read bb_read
#define write bb_write
#define safe_read bb_read
#define safe_write bb_write
ssize_t full_read(int,void *,size_t),full_write(int,const void *,size_t);
int64_t read_key(int,char *,int),safe_read_key(int,char *,int);
#endif
