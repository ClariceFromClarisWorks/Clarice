#include "libbb.h"
#include "common_bufsiz.h"
union bb_scratch bb_scratch;
const char *applet_name;
int xfunc_error_retval=1,logmode=1;
struct globals *ptr_to_globals;
static void message(int syserr,const char *fmt,va_list ap){int e=errno;if(!logmode)return;fprintf(stderr,"%s: ",applet_name);vfprintf(stderr,fmt,ap);if(syserr)fprintf(stderr,": %s",strerror(e));fputc('\n',stderr);}
void bb_error_msg(const char *s,...){va_list a;va_start(a,s);message(0,s,a);va_end(a);}
void bb_perror_msg(const char *s,...){va_list a;va_start(a,s);message(1,s,a);va_end(a);}
void bb_error_msg_and_die(const char *s,...){va_list a;va_start(a,s);message(0,s,a);va_end(a);exit(xfunc_error_retval);}
void bb_perror_msg_and_die(const char *s,...){va_list a;va_start(a,s);message(1,s,a);va_end(a);exit(xfunc_error_retval);}
void bb_simple_error_msg(const char *s){bb_error_msg("%s",s);}
void bb_simple_perror_msg(const char *s){bb_perror_msg("%s",s);}
void bb_simple_error_msg_and_die(const char *s){bb_error_msg_and_die("%s",s);}
void bb_simple_perror_msg_and_die(const char *s){bb_perror_msg_and_die("%s",s);}
void bb_show_usage(void){bb_error_msg("invalid arguments; use --help");exit(2);}
void *xmalloc(size_t n){void *p=malloc(n);if(!p)bb_simple_error_msg_and_die("out of memory");return p;}
void *xzalloc(size_t n){void *p=xmalloc(n);memset(p,0,n);return p;}
void *xrealloc(void *p,size_t n){p=realloc(p,n);if(!p&&n)bb_simple_error_msg_and_die("out of memory");return p;}
char *xstrdup(const char *s){char *p=strdup(s);if(!p)bb_simple_error_msg_and_die("out of memory");return p;}
char *xstrndup(const char *s,size_t n){char *p=strndup(s,n);if(!p)bb_simple_error_msg_and_die("out of memory");return p;}
char *xasprintf(const char *s,...){va_list a,b;va_start(a,s);va_copy(b,a);int n=vsnprintf(NULL,0,s,a);va_end(a);if(n<0)bb_simple_error_msg_and_die("format error");char *p=xmalloc(n+1);vsnprintf(p,n+1,s,b);va_end(b);return p;}
unsigned string_array_len(char **v){unsigned n=0;while(v[n])n++;return n;}
unsigned long xstrtoul_range(const char *s,int base,unsigned long lo,unsigned long hi){char *e;errno=0;unsigned long v=strtoul(s,&e,base);if(errno||!*s||*s=='-'||*e||v<lo||v>hi)bb_error_msg_and_die("invalid number: %s",s);return v;}
unsigned xatoi_positive(const char *s){return xstrtoul_range(s,10,0,INT_MAX);}
unsigned xatou(const char *s){return xstrtoul_range(s,10,0,UINT_MAX);}
long xatol(const char *s){char *e;errno=0;long n=strtol(s,&e,10);if(errno||!*s||*e)bb_error_msg_and_die("invalid number: %s",s);return n;}
FILE *fopen_or_warn_stdin(const char *s){FILE *f=LONE_DASH(s)?stdin:fopen(s,"r");if(!f)bb_simple_perror_msg(s);return f;}
FILE *xfopen_stdin(const char *s){FILE *f=fopen_or_warn_stdin(s);if(!f)exit(xfunc_error_retval);return f;}
FILE *xfopen(const char *s,const char *m){FILE *f=fopen(s,m);if(!f)bb_simple_perror_msg_and_die(s);return f;}
int fclose_if_not_stdin(FILE *f){if(ferror(f))bb_simple_perror_msg_and_die("input");int rc=f!=stdin?fclose(f):0;if(rc)bb_simple_perror_msg_and_die("close");return rc;}
void die_if_ferror(FILE *f,const char *s){if(ferror(f))bb_simple_perror_msg_and_die(s);}
int fflush_all(void){int a=fflush(stdout),b=fflush(stderr);return a||b;}
void fflush_stdout_and_exit(int s){if(fflush(stdout)||ferror(stdout))s=xfunc_error_retval;exit(s);}
void xwrite(int fd,const void *p,size_t n){if(full_write(fd,p,n)!=(ssize_t)n)bb_simple_perror_msg_and_die("write");}
char *strchrnul(const char *s,int c){while(*s&&*s!=(char)c)s++;return (char *)s;}
void fputs_stdout(const char *s){fputs(s,stdout);}
int bb_putchar(int c){return putchar(c);}

FILE *fopen_or_warn(const char *s,const char *m){FILE *f=fopen(s,m);if(!f)bb_simple_perror_msg(s);return f;}

int open_or_warn_stdin(const char *s){int fd=LONE_DASH(s)?0:open(s,O_RDONLY);if(fd<0)bb_simple_perror_msg(s);return fd;}
void xfunc_die(void){exit(xfunc_error_retval);}

/* Numeric boundary: BusyBox b/k/m suffix semantics, checked 32-bit range. */
const struct suffix_mult bkm_suffixes[]={{"b",512},{"k",1024},{"m",1024*1024},{"",0}};
unsigned long xatoul_sfx(const char *s,const struct suffix_mult *table){char *e;errno=0;unsigned long n=strtoul(s,&e,10);if(errno||e==s||*s=='-')bb_error_msg_and_die("invalid number: %s",s);if(*e){for(;table->mult;table++)if(!strcmp(e,table->suffix)){if(n>ULONG_MAX/table->mult)bb_error_msg_and_die("number too large: %s",s);return n*table->mult;}bb_error_msg_and_die("invalid suffix: %s",s);}return n;}
off_t xlseek(int fd,off_t n,int whence){off_t r=lseek(fd,n,whence);if(r<0)bb_simple_perror_msg_and_die("seek");return r;}

int fdprintf(int fd,const char *fmt,...){if(fd!=1&&fd!=2){errno=EBADF;return -1;}FILE *f=port_standard(fd);va_list a;va_start(a,fmt);int n=vfprintf(f,fmt,a);va_end(a);if(fflush(f))return -1;return n;}

/* Existing vendored musl/sbase helpers retain their collision-free names. */
extern char *xstrcasestr(const char *,const char *);
char *strcasestr(const char *h,const char *n){return xstrcasestr(h,n);}
