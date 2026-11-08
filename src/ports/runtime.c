/* Bounded invocation ownership for unmodified upstream stdio callers.
 * No allocations or file references survive the command's return/exit/cancel. */
#define PORT_RUNTIME
#include "compat.h"
#include <setjmp.h>
#include <stdint.h>
#define PORT_LIMIT (96UL*1024)
#define LINE_LIMIT (16UL*1024)
#define FILE_LIMIT 12
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
typedef struct Allocation {struct Allocation *prev,*next;size_t size;double align;} Allocation;
typedef struct Stream {FILE *fp;int fd,kind;char *buffer;} Stream;
static Allocation *allocations,*spares;
static unsigned long spare_bytes;
static unsigned long owned,peak;
extern void cli_port_stats(unsigned long,unsigned long);
static Stream streams[FILE_LIMIT];
static jmp_buf escape;
static int exit_status,cleaning,write_failed;
static unsigned poll_count;
void port_exit(int status){exit_status=status;longjmp(escape,1);}
void port_poll(void){int saved=errno;if(!cleaning&&!(++poll_count&255)&&app_yield())port_exit(130);errno=saved;}
static void yield_now(void){int saved=errno;if(!cleaning&&app_yield())port_exit(130);errno=saved;}
static void discard_spares(void){while(spares){Allocation *p=spares;spares=p->next;owned-=sizeof(*p)+p->size;cli_free(p);}spare_bytes=0;}
void *port_malloc(size_t n){
 Allocation *p=NULL;port_poll();if(!n)n=1;
 if(n>PORT_LIMIT-sizeof(*p)){errno=ENOMEM;return NULL;}
 /* Reuse short-lived regex scratch blocks within this invocation only. */
 Allocation **best=NULL;for(Allocation **link=&spares;*link;link=&(*link)->next)
  if((*link)->size>=n&&(*link)->size-n<=256&&(!best||(*link)->size<(*best)->size))best=link;
 if(best){p=*best;*best=p->next;spare_bytes-=sizeof(*p)+p->size;}
 else {
  if(owned>PORT_LIMIT-n-sizeof(*p))discard_spares();
  if(owned>PORT_LIMIT-n-sizeof(*p)){errno=ENOMEM;return NULL;}
  p=cli_alloc(sizeof(*p)+n);if(!p&&spares){discard_spares();p=cli_alloc(sizeof(*p)+n);}
  if(!p){errno=ENOMEM;return NULL;}
  p->size=n;owned+=sizeof(*p)+n;if(owned>peak)peak=owned;
 }
 p->prev=NULL;p->next=allocations;if(allocations)allocations->prev=p;allocations=p;return p+1;
}
void port_free(void *ptr){
 if(!ptr)return;Allocation *p=(Allocation*)ptr-1;
 if(p->prev)p->prev->next=p->next;else allocations=p->next;if(p->next)p->next->prev=p->prev;
 if(!cleaning&&sizeof(*p)+p->size<=8192-spare_bytes){p->next=spares;spares=p;spare_bytes+=sizeof(*p)+p->size;}
 else {owned-=sizeof(*p)+p->size;cli_free(p);}
}
void *port_calloc(size_t n,size_t size){if(size&&n>SIZE_MAX/size){errno=ENOMEM;return NULL;}size*=n;void *p=port_malloc(size);if(p)memset(p,0,size);return p;}
void *port_realloc(void *ptr,size_t n){if(!ptr)return port_malloc(n);if(!n){port_free(ptr);return NULL;}Allocation *p=(Allocation*)ptr-1;void *q=port_malloc(n);if(q){memcpy(q,ptr,n<p->size?n:p->size);port_free(ptr);}return q;}
char *port_strndup(const char *s,size_t n){size_t len=0;while(len<n&&s[len])len++;char *p=port_malloc(len+1);if(p){memcpy(p,s,len);p[len]=0;}return p;}
char *port_strdup(const char *s){return port_strndup(s,strlen(s));}
static int stream_read(void *cookie,char *buf,int n){Stream *s=cookie;int saved=errno;yield_now();long got=s->kind==1?cli_file_read(s->fd,buf,n):cli_read(s->fd,buf,n);if(got<0)errno=EIO;else errno=saved;return got;}
static int stream_write(void *cookie,const char *buf,int n){Stream *s=cookie;int saved=errno;yield_now();long put=s->kind==1?cli_file_write(s->fd,buf,n):cli_write(s->fd,buf,n);if(put<0){errno=EIO;write_failed=1;}else errno=saved;return put;}
static int stream_close(void *cookie){Stream *s=cookie;int rc=s->kind==1?cli_file_close(s->fd):0;if(rc)errno=EIO;return rc;}
static FILE *open_stream(int slot,int fd,int kind,int read_ok,int write_ok){
 Stream *s=&streams[slot];s->fd=fd;s->kind=kind;s->buffer=port_malloc(512);if(!s->buffer)return NULL;
 s->fp=funopen(s,read_ok?stream_read:NULL,write_ok?stream_write:NULL,NULL,stream_close);
 if(!s->fp){port_free(s->buffer);s->buffer=NULL;return NULL;}
 int buffering=kind==0&&fd==2?_IONBF:kind==0&&fd==1&&cli_isatty(1)?_IOLBF:_IOFBF;
 setvbuf(s->fp,s->buffer,buffering,512);return s->fp;
}
FILE *port_standard(int fd){return streams[fd].fp;}
static int free_slot(void){for(int i=3;i<FILE_LIMIT;i++)if(!streams[i].fp)return i;errno=EMFILE;return -1;}
FILE *port_fopen(const char *name,const char *mode){int slot=free_slot();if(slot<0)return NULL;yield_now();int fd=cli_file_open(name,mode);if(fd<0)return NULL;FILE *fp=open_stream(slot,fd,1,mode[0]=='r',mode[0]!='r');if(!fp)cli_file_close(fd);return fp;}
FILE *port_fmemopen(void *p,size_t n,const char *mode){int slot=free_slot();if(slot<0)return NULL;FILE *fp=fmemopen(p,n,mode);streams[slot].fp=fp;streams[slot].kind=2;return fp;}
int port_fclose(FILE *fp){for(int i=0;i<FILE_LIMIT;i++)if(fp&&streams[i].fp==fp){int rc=fclose(fp);streams[i].fp=NULL;port_free(streams[i].buffer);streams[i].buffer=NULL;return rc;}errno=EBADF;return EOF;}
static void reserve_line(char **line,size_t *cap,size_t need){
 if(need>LINE_LIMIT+1){fputs("text tool: line exceeds 16384-byte limit\n",port_standard(2));port_exit(2);}
 if(need<=*cap)return;
 size_t next=*cap?*cap:256;while(next<need){next*=2;if(next>LINE_LIMIT+1)next=LINE_LIMIT+1;}
 char *p=port_realloc(*line,next);if(!p){fputs("text tool: allocation limit reached\n",port_standard(2));port_exit(2);}*line=p;*cap=next;
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
 if(!setjmp(escape)){
  if(!open_stream(0,0,0,1,0)||!open_stream(1,1,0,0,1)||!open_stream(2,2,0,0,1))exit_status=2;
  else exit_status=upstream_main(argc,argv);
 }
 cleaning=1;
 /* Output flush errors must survive both normal return and upstream exit(0). */
 for(int i=FILE_LIMIT-1;i>=0;i--)if(streams[i].fp&&port_fclose(streams[i].fp))write_failed=1;
 while(allocations)port_free(allocations+1);discard_spares();
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
int port_vfprintf(FILE *fp,const char *fmt,va_list ap){
 char converted[512];size_t i=0,j=0;int conversion=0;
 for(;fmt[i];i++){
  if(j==sizeof converted-1){fputs("text tool: format exceeds 511 bytes\n",port_standard(2));port_exit(2);}
  char c=fmt[i];if(!conversion){if(c=='%')conversion=1;}
  else {if(c=='z'||c=='t')c='l';else if(strchr("diouxXfFeEgGaAcspn%",c))conversion=0;}
  converted[j++]=c;
 }
 converted[j]=0;return vfprintf(fp,converted,ap);
}
int port_fprintf(FILE *fp,const char *fmt,...){va_list ap;va_start(ap,fmt);int rc=port_vfprintf(fp,fmt,ap);va_end(ap);return rc;}
