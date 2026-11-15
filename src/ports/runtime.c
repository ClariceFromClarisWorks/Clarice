/* Bounded invocation ownership for unmodified upstream stdio callers.
 * No allocations or file references survive the command's return/exit/cancel. */
#define PORT_RUNTIME
#include "compat.h"
#include <setjmp.h>
#include <stdint.h>
#include <limits.h>
extern void *cli_alloc(unsigned long);
extern void cli_free(void *);
extern int cli_isatty(int);
extern long cli_read(int,void *,unsigned long);
extern long cli_write(int,const void *,unsigned long);
extern int cli_file_open(const char *,const char *);
extern long cli_file_read(int,void *,unsigned long);
extern long cli_file_write(int,const void *,unsigned long);
extern int cli_file_close(int);
extern int app_yield(void);
extern int upstream_main(int,char **);
#include "alloc-pages.inc"
typedef struct Stream {FILE *fp;int fd,kind;char *buffer;struct Stream *next;} Stream;
static Allocation *allocations,*spares;
static unsigned long spare_bytes;
extern void cli_port_stats(unsigned long,unsigned long);
static Stream streams[3],*extra_streams;
int port_fileno(FILE *fp){for(int i=0;i<3;i++)if(fp&&streams[i].fp==fp)return i;for(Stream *s=extra_streams;s;s=s->next)if(fp&&s->fp==fp&&s->kind==4)return s->fd;errno=EBADF;return -1;}
#define TEMP_LIMIT 20
static int *descriptors,descriptor_capacity; /* native reference + 1; zero unused */
static char temporary[TEMP_LIMIT][96];
extern long cli_file_size(int);
extern long cli_file_seek(int,long,int);
extern long cli_stream_size(int);
extern int cli_pause_ticks(unsigned long);
extern int cli_temp_create(char *,unsigned long);
extern int cli_remove(const char *);

static jmp_buf escape;
static int exit_status,cleaning,write_failed;
static unsigned poll_count;
void port_exit(int status){exit_status=status;longjmp(escape,1);}
void port_poll(void){int saved=errno;if(!cleaning&&!(++poll_count&255)&&app_yield())port_exit(130);errno=saved;}
static void yield_now(void){int saved=errno;if(!cleaning&&app_yield())port_exit(130);errno=saved;}
static void discard_spares(void){while(spares){Allocation *p=spares;spares=p->next;allocation_delete(p);}spare_bytes=0;}
void *port_malloc(size_t n){
 Allocation *p=NULL;port_poll();if(!n)n=1;
 if(n>SIZE_MAX-sizeof(*p)){errno=ENOMEM;return NULL;}
 /* Reuse short-lived regex scratch blocks within this invocation only. */
 Allocation **best=NULL;for(Allocation **link=&spares;*link;link=&(*link)->next)
  if((*link)->size>=n&&(*link)->size-n<=256&&(!best||(*link)->size<(*best)->size))best=link;
 if(best){p=*best;*best=p->next;spare_bytes-=sizeof(*p)+p->size;}
 else {
  if(owned>ULONG_MAX-n-sizeof(*p)){errno=ENOMEM;return NULL;}
  p=allocation_new(n);if(!p&&spares){discard_spares();p=allocation_new(n);}
  if(!p){errno=ENOMEM;return NULL;}
  if(owned>peak)peak=owned;
 }
 p->prev=NULL;p->next=allocations;if(allocations)allocations->prev=p;allocations=p;return p+1;
}
void port_free(void *ptr){
 if(!ptr)return;Allocation *p=(Allocation*)ptr-1;
 if(p->prev)p->prev->next=p->next;else allocations=p->next;if(p->next)p->next->prev=p->prev;
 if(!p->page&&!cleaning&&sizeof(*p)+p->size<=8192-spare_bytes){p->next=spares;spares=p;spare_bytes+=sizeof(*p)+p->size;}
 else {allocation_delete(p);}
}
void *port_calloc(size_t n,size_t size){if(size&&n>SIZE_MAX/size){errno=ENOMEM;return NULL;}size*=n;void *p=port_malloc(size);if(p)memset(p,0,size);return p;}
void *port_realloc(void *ptr,size_t n){if(!ptr)return port_malloc(n);if(!n){port_free(ptr);return NULL;}Allocation *p=(Allocation*)ptr-1;void *q=port_malloc(n);if(q){memcpy(q,ptr,n<p->size?n:p->size);port_free(ptr);}return q;}
char *port_strndup(const char *s,size_t n){size_t len=0;while(len<n&&s[len])len++;char *p=port_malloc(len+1);if(p){memcpy(p,s,len);p[len]=0;}return p;}
char *port_strdup(const char *s){return port_strndup(s,strlen(s));}
static int stream_read(void *cookie,char *buf,int n){Stream *s=cookie;int saved=errno;yield_now();errno=0;long got=s->kind==3?0:s->kind==4?port_read(s->fd,buf,n):s->kind==1?cli_file_read(s->fd,buf,n):cli_read(s->fd,buf,n);if(got<0){if(!errno)errno=EIO;}else errno=saved;return got;}
static int stream_write(void *cookie,const char *buf,int n){Stream *s=cookie;int saved=errno;yield_now();errno=0;long put=s->kind==3?n:s->kind==4?port_write(s->fd,buf,n):s->kind==1?cli_file_write(s->fd,buf,n):cli_write(s->fd,buf,n);if(put<0){if(!errno)errno=EIO;write_failed=1;}else errno=saved;return put;}
static int stream_close(void *cookie){Stream *s=cookie;int rc=s->fd<0?0:s->kind==4?port_close(s->fd):s->kind==1?cli_file_close(s->fd):0;s->fd=-1;if(rc&&!errno)errno=EIO;return rc;}
static fpos_t stream_seek(void *cookie,fpos_t offset,int whence){
 Stream *s=cookie;yield_now();
 if(offset>LONG_MAX||offset<LONG_MIN){errno=EOVERFLOW;return -1;}
 if(s->kind==4)return port_lseek(s->fd,(off_t)offset,whence);
 if(s->kind==1)return cli_file_seek(s->fd,(long)offset,whence);
 errno=ESPIPE;return -1;
}
static FILE *open_stream(Stream *s,int fd,int kind,int read_ok,int write_ok){
 s->fd=fd;s->kind=kind;s->buffer=port_malloc(512);if(!s->buffer)return NULL;
 s->fp=funopen(s,read_ok?stream_read:NULL,write_ok?stream_write:NULL,(kind==1||kind==4)?stream_seek:NULL,stream_close);
 if(!s->fp){port_free(s->buffer);s->buffer=NULL;return NULL;}
 int buffering=kind==0&&fd==2?_IONBF:kind==0&&fd==1&&cli_isatty(1)?_IOLBF:_IOFBF;
 setvbuf(s->fp,s->buffer,buffering,512);return s->fp;
}
FILE *port_standard(int fd){return streams[fd].fp;}
/* Stable cookies: linked records never move while libc holds their address. */
static Stream *free_slot(void){Stream *s=port_calloc(1,sizeof *s);if(!s)return NULL;s->fd=-1;s->next=extra_streams;extra_streams=s;return s;}
static void release_stream(Stream *s){
 if(s->fd>=0&&(s->kind==1||s->kind==4))stream_close(s);
 port_free(s->buffer);s->buffer=NULL;
 for(Stream **link=&extra_streams;*link;link=&(*link)->next)if(*link==s){*link=s->next;port_free(s);return;}
}
FILE *port_fopen(const char *name,const char *mode){
 if(!mode||!*mode){errno=EINVAL;return NULL;}yield_now();Stream *s=free_slot();if(!s)return NULL;
 int fd=cli_file_open(name,mode);if(fd<0){release_stream(s);return NULL;}
 FILE *fp=open_stream(s,fd,1,mode[0]=='r'||strchr(mode,'+'),mode[0]!='r'||strchr(mode,'+'));if(!fp){int e=errno;release_stream(s);errno=e;}return fp;
}
/* fdopen transfers ownership only on success. */
FILE *port_fdopen(int fd,const char *mode){
 if(fd<3||fd>=descriptor_capacity||!descriptors[fd]){errno=EBADF;return NULL;}
 if(!mode||!strchr("rwa",*mode)){errno=EINVAL;return NULL;}Stream *s=free_slot();if(!s)return NULL;
 FILE *fp=open_stream(s,fd,4,*mode=='r'||strchr(mode,'+'),*mode!='r'||strchr(mode,'+'));if(!fp){int e=errno;s->fd=-1;release_stream(s);errno=e;}return fp;
}
FILE *port_fmemopen(void *p,size_t n,const char *mode){Stream *s=free_slot();if(!s)return NULL;s->kind=2;s->fp=fmemopen(p,n,mode);if(!s->fp){release_stream(s);return NULL;}return s->fp;}
int port_fclose(FILE *fp){
 if(!fp){errno=EBADF;return EOF;}Stream *s=NULL;
 for(int i=0;i<3;i++)if(streams[i].fp==fp)s=&streams[i];
 if(!s)for(s=extra_streams;s&&s->fp!=fp;s=s->next){}
 if(!s){errno=EBADF;return EOF;}int rc=fclose(fp);s->fp=NULL;release_stream(s);return rc;
}
static void reserve_line(char **line,size_t *cap,size_t need){
 if(need>(size_t)LONG_MAX){fputs("text tool: line size overflow\n",port_standard(2));port_exit(2);}
 if(need<=*cap)return;
 size_t next=*cap?*cap:256;while(next<need){if(next>SIZE_MAX/2){next=need;break;}next*=2;}
 char *p=port_realloc(*line,next);if(!p){fputs("text tool: insufficient memory\n",port_standard(2));port_exit(2);}*line=p;*cap=next;
}
ssize_t port_getline(char **line,size_t *cap,FILE *fp){
 size_t n=0;int c;yield_now();
 while((c=fgetc(fp))!=EOF){
  reserve_line(line,cap,n+2);(*line)[n++]=c;if(c=='\n')break;
#if defined(__NEWLIB__) || defined(__APPLE__)
  /* Both tested runtimes expose the BSD FILE read cursor. Consume already
     buffered bytes in a span; fgetc alone performs refills/pushback/error work. */
  if(fp->_r>0){
   size_t take=(size_t)fp->_r;unsigned char *end=memchr(fp->_p,'\n',take);
   if(end)take=(size_t)(end-fp->_p)+1;
   reserve_line(line,cap,n+take+1);memcpy(*line+n,fp->_p,take);n+=take;
   fp->_p+=take;fp->_r-=take;if(end)break;
  }
#endif
  port_poll();
 }
 if(!n)return -1;(*line)[n]=0;return n;
}
int clarice_main(int argc,char **argv){
#ifdef PORT_NANO_PERF
 extern void app_nano_mark(int);app_nano_mark(1);
#endif
 exit_status=cleaning=write_failed=0;poll_count=0;peak=0;
 if(!setjmp(escape)){
  if(!open_stream(&streams[0],0,0,1,0)||!open_stream(&streams[1],1,0,0,1)||!open_stream(&streams[2],2,0,0,1))exit_status=2;
  else {
#ifdef PORT_AWK
   extern void port_awk_init(void);port_awk_init();
#endif
   exit_status=upstream_main(argc,argv);
  }
 }
 cleaning=1;
 /* Output flush errors must survive both normal return and upstream exit(0). */
 while(extra_streams){Stream *s=extra_streams;if(s->fp){if(port_fclose(s->fp))write_failed=1;}else release_stream(s);}
 for(int i=2;i>=0;i--)if(streams[i].fp&&port_fclose(streams[i].fp))write_failed=1;
 for(int i=3;i<descriptor_capacity;i++)if(descriptors[i]&&port_close(i))write_failed=1;
 for(int i=0;i<TEMP_LIMIT;i++)if(temporary[i][0]&&cli_remove(temporary[i]))write_failed=1;
 while(allocations)port_free(allocations+1);discard_spares();descriptors=NULL;descriptor_capacity=0;memset(streams,0,sizeof streams);
 cli_port_stats(peak,owned);
 if(write_failed&&exit_status!=130)exit_status=2;
 return exit_status;
}

int port_printf(const char *fmt,...){va_list ap;va_start(ap,fmt);int rc=port_vfprintf(port_standard(1),fmt,ap);va_end(ap);return rc;}
int port_puts(const char *s){FILE *fp=port_standard(1);if(fputs(s,fp)<0||fputc('\n',fp)==EOF)return EOF;return 0;}
void port_perror(const char *s){int e=errno;FILE *fp=port_standard(2);if(s&&*s)fprintf(fp,"%s: ",s);fprintf(fp,"%s\n",strerror(e));}
void port_abort(void){fputs("text tool: internal assertion failed\n",port_standard(2));port_exit(2);}

/* Pinned newlib lacks the C99 z/t printf length modifiers. The ABI uses
   equally sized size_t/ptrdiff_t and long; adapt formats, never argument data. */
static void convert_format(char converted[512],const char *fmt){
 size_t i=0,j=0;int conversion=0;
 for(;fmt[i];i++){
  if(j>=510){fputs("text tool: format exceeds 510 bytes\n",port_standard(2));port_exit(2);}
  char c=fmt[i];if(!conversion){if(c=='%')conversion=1;}
  else {if(c=='z'||c=='t')c='l';else if(c=='j'){converted[j++]='l';c='l';}else if(strchr("diouxXfFeEgGaAcspn%",c))conversion=0;}
  converted[j++]=c;
 }
 converted[j]=0;
}
int port_vfprintf(FILE *fp,const char *fmt,va_list ap){char converted[512];convert_format(converted,fmt);return vfprintf(fp,converted,ap);}
int port_vsnprintf(char *dst,size_t n,const char *fmt,va_list ap){char converted[512];convert_format(converted,fmt);return vsnprintf(dst,n,converted,ap);}
int port_snprintf(char *dst,size_t n,const char *fmt,...){va_list ap;va_start(ap,fmt);int rc=port_vsnprintf(dst,n,fmt,ap);va_end(ap);return rc;}
int port_vsprintf(char *dst,const char *fmt,va_list ap){char converted[512];convert_format(converted,fmt);return vsnprintf(dst,INT_MAX,converted,ap);}
int port_sprintf(char *dst,const char *fmt,...){va_list ap;va_start(ap,fmt);int rc=port_vsprintf(dst,fmt,ap);va_end(ap);return rc;}

int port_fprintf(FILE *fp,const char *fmt,...){va_list ap;va_start(ap,fmt);int rc=port_vfprintf(fp,fmt,ap);va_end(ap);return rc;}

/* awk close("/dev/stdout") redirects that stream to the null device. */
FILE *port_null_reopen(const char *path,const char *mode,FILE *fp){
 if(strcmp(path,"/dev/null")||strcmp(mode,"r+")){errno=EINVAL;return NULL;}
 for(int i=0;i<3;i++)if(streams[i].fp==fp){if(fflush(fp))return NULL;streams[i].kind=3;clearerr(fp);return fp;}
 errno=EBADF;return NULL;
}

int port_putchar(int c){return fputc(c,port_standard(1));}

/* Descriptor I/O shares the same command streams without leaking host handles.
   Never pass a Macintosh file reference directly to a portable caller. */
int port_open(const char *path,int flags,...){
 const char *mode;
 if(flags==O_RDONLY)mode="r";
 else if(flags==(O_WRONLY|O_CREAT|O_TRUNC))mode="w";
 else if(flags==(O_WRONLY|O_CREAT|O_APPEND))mode="a";
 else if(flags==(O_WRONLY|O_CREAT|O_EXCL))mode="x";
 else if(flags==(O_RDWR|O_CREAT|O_EXCL))mode="x+";
 else if(flags==O_RDWR)mode="r+";
 else {errno=EINVAL;return -1;}
 yield_now();int i;for(i=3;i<descriptor_capacity&&descriptors[i];i++);
 if(i>=descriptor_capacity){
  int old=descriptor_capacity;size_t cap=old?(size_t)old*2:8;
  if(cap>INT_MAX||cap>SIZE_MAX/sizeof *descriptors){errno=EMFILE;return -1;}
  int *p=port_realloc(descriptors,cap*sizeof *p);if(!p)return -1;
  memset(p+old,0,(cap-old)*sizeof *p);descriptors=p;descriptor_capacity=(int)cap;
 }
 int ref=cli_file_open(path,mode);if(ref<0)return -1;descriptors[i]=ref+1;return i;
}
int port_close(int fd){if(fd<3||fd>=descriptor_capacity||!descriptors[fd]){errno=EBADF;return -1;}int ref=descriptors[fd]-1;descriptors[fd]=0;return cli_file_close(ref);}
ssize_t port_read(int fd,void *p,size_t n){yield_now();if(fd==0)return cli_read(0,p,n);if(fd<3||fd>=descriptor_capacity||!descriptors[fd]){errno=EBADF;return -1;}return cli_file_read(descriptors[fd]-1,p,n);}
ssize_t port_write(int fd,const void *p,size_t n){
 yield_now();long rc;
 if(fd==1||fd==2){if(fflush(port_standard(fd)))return -1;rc=cli_write(fd,p,n);}
 else if(fd>=3&&fd<descriptor_capacity&&descriptors[fd])rc=cli_file_write(descriptors[fd]-1,p,n);
 else {errno=EBADF;return -1;}
 if(rc<0)write_failed=1;return rc;
}
int port_fstat(int fd,struct stat *st){
 memset(st,0,sizeof *st);long n;
 if(fd>=0&&fd<3){if(cli_isatty(fd)){st->st_mode=S_IFCHR;return 0;}n=cli_stream_size(fd);}
 else if(fd>=3&&fd<descriptor_capacity&&descriptors[fd])n=cli_file_size(descriptors[fd]-1);
 else {errno=EBADF;return -1;}
 if(n<0)return -1;st->st_mode=S_IFREG;st->st_size=n;return 0;
}
off_t port_lseek(int fd,off_t offset,int whence){if(fd<3||fd>=descriptor_capacity||!descriptors[fd]){errno=EBADF;return -1;}return cli_file_seek(descriptors[fd]-1,offset,whence);}
unsigned port_sleep(unsigned seconds){while(seconds--){if(cli_pause_ticks(60))port_exit(130);}return 0;}
const char *port_temp_new(void){
 yield_now();for(int i=0;i<TEMP_LIMIT;i++)if(!temporary[i][0]){
  if(cli_temp_create(temporary[i],sizeof temporary[i])){temporary[i][0]=0;return NULL;}return temporary[i];
 }errno=EMFILE;return NULL;
}
void port_temp_remove(const char *name){for(int i=0;i<TEMP_LIMIT;i++)if(temporary[i]==name){if(cli_remove(name)){fputs("sort: cannot remove temporary file\n",port_standard(2));port_exit(2);}temporary[i][0]=0;return;}port_abort();}

int port_track_temp(const char *name){if(strlen(name)>=sizeof temporary[0]){errno=ENAMETOOLONG;return -1;}for(int i=0;i<TEMP_LIMIT;i++)if(!temporary[i][0]){strcpy(temporary[i],name);return 0;}errno=EMFILE;return -1;}
