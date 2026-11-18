/* Clarice bindings for selected genuine BusyBox 1.37.0 applets. */
#ifndef CLARICE_BBTOOLS_H
#define CLARICE_BBTOOLS_H
#include "config.h"
#include "../compat.h"
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <ctype.h>
#include <getopt.h>
#include <time.h>
#include <alloca.h>
#include <strings.h>
#define FAST_FUNC
#define MAIN_EXTERNALLY_VISIBLE
#define FIX_ALIASING
#define ALIGN_PTR
#define ALIGN1
#define ALWAYS_INLINE inline
#define UNUSED_PARAM __attribute__((unused))
#define NOINLINE __attribute__((noinline))
#define NORETURN __attribute__((noreturn))
#define PUSH_AND_SET_FUNCTION_VISIBILITY_TO_HIDDEN
#define POP_SAVED_FUNCTION_VISIBILITY
#define TRUE 1
#define FALSE 0
#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))
#define OFF_FMT "l"
#define XATOOFF(s) xatol(s)
#define BB_VER "1.37.0"
#define GETOPT_RESET() do { optind=0; opterr=0; } while(0)
#define overlapping_strcpy(d,s) memmove(d,s,strlen(s)+1)
#define isprint_asciionly(c) ((unsigned)((c)-32)<95)
#define die_if_ferror_stdout() die_if_ferror(stdout,"stdout")
#define setbuf(f,b) setvbuf(f,b,(b)?_IOFBF:_IONBF,BUFSIZ)
#define bb_perror_nomsg_and_die() bb_simple_perror_msg_and_die("")
char *last_char_is(const char *,int);
char *single_argv(char **),*bb_get_last_path_component_strip(char *);
char *port_path_dirname(char *);
#define dirname port_path_dirname
struct suffix_mult {const char *suffix; unsigned mult;};
extern const struct suffix_mult bkm_suffixes[];
unsigned long xatoul_sfx(const char *,const struct suffix_mult *);
#define xatou_sfx xatoul_sfx
off_t xlseek(int,off_t,int);
#define exit_SUCCESS() exit(0)
char *strcasestr(const char *,const char *);
#define fflush_stdout_and_exit_SUCCESS() fflush_stdout_and_exit(0)
#define xfopen_for_write(s) xfopen(s,"w")
#define fopen_for_read(s) fopen(s,"r")
#define bb_die_memory_exhausted() bb_simple_error_msg_and_die("out of memory")
#define safe_read port_read
#define safe_write port_write
#define bb_msg_standard_input "-"
#define bb_msg_read_error "read error"
#define bb_msg_write_error "write error"
#define bb_msg_memory_exhausted "out of memory"
#define bb_msg_requires_arg "%s requires an argument"
#define LOGMODE_STDIO 1
#define No_argument "\0"
#define Required_argument "\1"
#define Optional_argument "\2"
#define NOT_LONE_DASH(s) (!LONE_DASH(s))
#define LONE_DASH(s) ((s)[0]=='-' && !(s)[1])
typedef unsigned long uoff_t;
typedef int exitcode_t;
extern const char *const bb_argv_dash[];
int open_or_warn_stdin(const char *),bb_cat(char **);
off_t bb_copyfd_eof(int,int),bb_copyfd_size(int,int,off_t);
void xfunc_die(void) NORETURN;
uint32_t *crc32_filltable(uint32_t *,int);
uint32_t crc32_block_endian0(uint32_t,const void *,unsigned,uint32_t *),crc32_block_endian1(uint32_t,const void *,unsigned,uint32_t *);
typedef signed char smallint;
typedef unsigned char smalluint;
typedef struct llist_t { struct llist_t *link; char *data; } llist_t;
extern const char *applet_name;
extern int xfunc_error_retval,logmode;
extern uint32_t option_mask32;
struct globals;
extern struct globals *ptr_to_globals;
#define SET_PTR_TO_GLOBALS(p) (ptr_to_globals=(p))
#define xrealloc_vector(v,shift,idx) xrealloc_vector_helper(v,(sizeof((v)[0])<<8)+(shift),idx)
void *xrealloc_vector_helper(void *,unsigned,int);
void *xmalloc(size_t),*xzalloc(size_t),*xrealloc(void *,size_t);
char *xstrdup(const char *),*xstrndup(const char *,size_t),*xasprintf(const char *,...);
void bb_error_msg(const char *,...);
void bb_error_msg_and_die(const char *,...) NORETURN;
void bb_perror_msg(const char *,...);
void bb_perror_msg_and_die(const char *,...) NORETURN;
void bb_simple_error_msg(const char *);
void bb_simple_error_msg_and_die(const char *) NORETURN;
void bb_simple_perror_msg(const char *);
void bb_simple_perror_msg_and_die(const char *) NORETURN;
void bb_show_usage(void) NORETURN;
void fflush_stdout_and_exit(int) NORETURN;
int fflush_all(void);
void die_if_ferror(FILE *,const char *);
int fclose_if_not_stdin(FILE *);
FILE *fopen_or_warn(const char *,const char *);
FILE *fopen_or_warn_stdin(const char *),*xfopen_stdin(const char *),*xfopen(const char *,const char *);
unsigned string_array_len(char **);
uint32_t getopt32(char **,const char *,...);
uint32_t getopt32long(char **,const char *,const char *,...);
unsigned xatoi_positive(const char *),xatou(const char *);
long xatol(const char *);
unsigned long xstrtoul_range(const char *,int,unsigned long,unsigned long);
void llist_add_to_end(llist_t **,void *),llist_add_to(llist_t **,void *);
void *llist_pop(llist_t **);
void llist_free(llist_t *,void (*)(void *));
char *is_prefixed_with(const char *,const char *);
char *skip_whitespace(const char *),*skip_non_whitespace(const char *);
int index_in_strings(const char *,const char *);
char bb_process_escape_sequence(const char **);
int fdprintf(int,const char *,...);
ssize_t full_read(int,void *,size_t);
ssize_t full_write(int,const void *,size_t);
void xwrite(int,const void *,size_t);
char *xmalloc_fgetline(FILE *),*xmalloc_fgets(FILE *),*xmalloc_fgetline_str(FILE *,const char *);
char *bb_get_chunk_from_file(FILE *,size_t *);
char *strchrnul(const char *,int);
int bb_putchar(int);
void fputs_stdout(const char *);
#endif
