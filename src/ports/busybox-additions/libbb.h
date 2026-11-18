#ifndef CLARICE_BB_ADDITIONS_H
#define CLARICE_BB_ADDITIONS_H
#include "../busybox-tools/libbb.h"
#define move_from_unaligned16(v,p) memcpy(&(v),(p),2)
#define move_from_unaligned32(v,p) memcpy(&(v),(p),4)
#define move_from_unaligned64(v,p) memcpy(&(v),(p),8)
char *strcpy_and_process_escape_sequences(char *,const char *);
#include <setjmp.h>
#undef SET_PTR_TO_GLOBALS
#define SET_PTR_TO_GLOBALS(p) (ptr_to_globals=(struct globals *)(p))
#define ALIGN2 __attribute__((aligned(2)))
#define BB_LITTLE_ENDIAN 0
#define BB_BIG_ENDIAN 1
#define BB_MMU 0
#define BB_UNALIGNED_MEMACCESS_OK 0
#define USE_FOR_NOMMU(...)
#define USE_FOR_MMU(...)
#define SEAMLESS_COMPRESSION 0
extern int bb_got_signal;
int xopen(const char *,int);
void xread(int,void *,size_t);
void bb_copyfd_exact_size(int,int,off_t);
#define PACKED __attribute__((packed))
#define BUILD_BUG_ON(c) ((void)sizeof(char[1-2*!!(c)]))
#define SWAP_LE32(v) __builtin_bswap32(v)
extern uint32_t *global_crc32_table;
uint32_t *crc32_new_table_le(void);
uint32_t *global_crc32_new_table_le(void);
#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))
#define OFF_T_MAX LONG_MAX
#define BB_STRTOOFF bb_strtol
#define xfopen_for_read(s) xfopen(s,"r")
#define xstrtou_range xstrtoul_range
#define xstrtoull_range xstrtoul_range
#define BASE64_FLAG_UU_STOP 0x100
#define BASE64_32 0x200
extern const char bb_uuenc_tbl_base64[];
void bb_uuencode(char *,const void *,int,const char *);
void read_base64(FILE *,FILE *,int);
unsigned long isqrt(unsigned long long);
long bb_strtol(const char *,char **,int),xstrtoll(const char *,int);
char *concat_path_file(const char *,const char *);
int addition_stat(const char *,struct stat *);
#define stat(path,buf) addition_stat(path,buf)
void xstat(const char *,struct stat *);
FILE *addition_diff_input(const char *);
#endif
