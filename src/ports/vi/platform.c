/* Clarice boundary for genuine Berkeley/Heirloom ex/vi. */
#include <limits.h>
#include <Memory.h>
#include <Files.h>
#include <Errors.h>
#include "compat.h"
#include "../safe-save.h"
#include "termios.h"
#include "sys/ioctl.h"
extern unsigned long cli_ticks(void);
extern int cli_pause_ticks(unsigned long),app_screen_open(void),app_screen_rows(void),app_columns(void),app_screen_pending(void);
extern void app_screen_close(void);
extern long cli_remote_wire(int,void *,unsigned long);
extern int resolve_ex(const char *,char *,char *,int),info(const char *,CInfoPBRec *);
extern int vi_runtime_main(int,char **),main(int,char **);
static int remote,opened;
static Ptr lines;
static long lineused,lineallocated;
static unsigned long deadline;
static void (*alarm_handler)(int);
#include "save.inc"
#include "temp.inc"
void *vi_sbrk(intptr_t n){
 if(n<0||n>LONG_MAX-lineused){errno=ENOMEM;return (void*)-1;}
 if(!lines){lines=NewPtr(4096);if(!lines){errno=ENOMEM;return (void*)-1;}lineallocated=4096;}
 if(lineused+n>lineallocated){SetPtrSize(lines,lineused+n);if(MemError()){errno=ENOMEM;return (void*)-1;}lineallocated=lineused+n;}
 void *p=lines+lineused;lineused+=n;return p;
}
int vi_temp_id(void){return (int)(cli_ticks()&0x7fffffff);}
unsigned vi_alarm(unsigned seconds){unsigned old=deadline&&deadline>cli_ticks()?(deadline-cli_ticks()+59)/60:0;deadline=seconds?cli_ticks()+60UL*seconds:0;return old;}
int vi_sigaction(int sig,const struct sigaction *a,struct sigaction *old){
 extern int sigaction(int,const struct sigaction *,struct sigaction *);
 if(sig==SIGALRM){if(old){memset(old,0,sizeof *old);old->sa_handler=alarm_handler;}if(a)alarm_handler=a->sa_handler;return 0;}
 if(a){struct sigaction b=*a;b.sa_flags=0;return sigaction(sig,&b,old);}return sigaction(sig,0,old);
}
ssize_t vi_read(int fd,void *p,size_t n){
 if(fd!=0)return port_read(fd,p,n);if(!n)return 0;
 for(;;){if(deadline&&cli_ticks()>=deadline){deadline=0;if(alarm_handler&&alarm_handler!=SIG_IGN)alarm_handler(SIGALRM);errno=EINTR;return -1;}
 long got=0;if(remote)got=cli_remote_wire(2,p,n);else if(app_screen_pending())got=port_read(0,p,1);
 if(got<0)port_exit(1);if(got)return got;if(cli_pause_ticks(1))port_exit(130);
 }
}
ssize_t vi_write(int fd,const void *p,size_t n){long got=(opened&&fd==1&&remote)?cli_remote_wire(3,(void*)p,n):port_write(fd,p,n);vi_save_written(fd,got,n);if(opened&&remote&&fd==1&&got!=(long)n)port_exit(1);return got;}
int vi_ioctl(int fd,unsigned long req,...){va_list ap;va_start(ap,req);void *p=va_arg(ap,void*);va_end(ap);extern int ioctl(int,unsigned long,...);if(remote&&req==TIOCGWINSZ){struct winsize *w=p;memset(w,0,sizeof *w);w->ws_row=24;w->ws_col=80;return 0;}return ioctl(fd,req,p);}
int vi_stat(const char *s,struct stat *st){char path[512];CInfoPBRec pb;int e=resolve_ex(s,path,0,1);if(!e)e=info(path,&pb);if(e){errno=e==fnfErr||e==dirNFErr?ENOENT:EIO;return -1;}memset(st,0,sizeof *st);st->st_mode=(pb.hFileInfo.ioFlAttrib&16)?S_IFDIR:S_IFREG;st->st_mode|=S_IRUSR;if(!(pb.hFileInfo.ioFlAttrib&1))st->st_mode|=S_IWUSR;st->st_size=pb.hFileInfo.ioFlLgLen;st->st_ino=pb.hFileInfo.ioDirID;st->st_blksize=512;return 0;}
char *vi_getenv(const char *name){
 if(!strcmp(name,"TERM"))return "clarice";
 if(!strcmp(name,"TERMCAP"))return "clarice|Clarice ANSI:co#80:li#24:am:bs:cl=\\E[H\\E[2J:cm=\\E[%i%d;%dH:ce=\\E[K:cd=\\E[J:up=\\E[A:nd=\\E[C:do=^J:cr=^M:so=\\E[7m:se=\\E[m:ku=\\E[A:kd=\\E[B:kr=\\E[C:kl=\\E[D:vs=\\E[?25h:";
 return port_curses_getenv(name);
}
#define UNSUPPORTED errno=ENOSYS;return -1
pid_t vi_fork(void){UNSUPPORTED;}pid_t vi_wait(int *p){(void)p;UNSUPPORTED;}
int vi_pipe(int *p){(void)p;UNSUPPORTED;}int vi_execl(const char *p,const char *a,...){(void)p;(void)a;UNSUPPORTED;}
int vi_execv(const char *p,char *const *a){(void)p;(void)a;UNSUPPORTED;}int vi_kill(pid_t p,int s){(void)p;(void)s;UNSUPPORTED;}
int vi_dup(int fd){(void)fd;UNSUPPORTED;}int vi_chdir(const char *p){(void)p;UNSUPPORTED;}
char *vi_ttyname(int fd){(void)fd;errno=ENOSYS;return NULL;}
int upstream_main(int argc,char **argv){
 if(argc==2&&(!strcmp(argv[1],"--help")||!strcmp(argv[1],"--version"))){puts("vi: original Berkeley ex/vi 3.7, Heirloom upstream; experimental Clarice port");if(!strcmp(argv[1],"--help"))puts("Usage: vi [-R] [-c command] [+line] [file ...]\nvi keys: i insert; Escape normal; :w save; :q quit; :q! discard.\n80x24 ANSI Telnet BINARY; byte-oriented text; safe HFS replacement.\nUnsupported: processes, shell/filter, cd, append writes, auto exrc, recovery.\nExperimental: unsaved edits can be lost on disconnect or cancellation.\nStatus 0 quit/help, 1 failure, 2 adapter usage, 130 cancellation.");return 0;}
 for(int i=1;i<argc;i++)if(!strcmp(argv[i],"-r")||!strcmp(argv[i],"-L")||!strncmp(argv[i],"--",2)){fprintf(stderr,"vi: unsupported option: %s\n",argv[i]);return 2;}
 if(!cli_isatty(0)||!cli_isatty(1)){fputs("vi: terminal required\n",stderr);return 2;}
 remote=cli_remote_wire(0,0,0)>0;
 if(remote){if(cli_remote_wire(1,0,0)){fputs("vi: Telnet BINARY and empty input queue required\n",stderr);return 1;}}
 else if(app_screen_open())return 1;opened=1;
 return main(argc,argv);
}
int clarice_main(int argc,char **argv){int rc=vi_runtime_main(argc,argv);if(lines){DisposePtr(lines);lines=0;}if(opened){if(remote)cli_remote_wire(4,0,rc!=0);else app_screen_close();}return rc;}

char *vi_getcwd(char *p,size_t n){char *s=port_curses_getenv("PWD");if(!s||strlen(s)>=n){errno=ERANGE;return NULL;}strcpy(p,s);return p;}
int vi_chmod(const char *s,mode_t mode){(void)s;(void)mode;errno=ENOSYS;return -1;}
int vi_access(const char *s,int mode){struct stat st;if(vi_stat(s,&st))return -1;if((mode&W_OK)&&!(st.st_mode&S_IWUSR)){errno=EACCES;return -1;}return 0;}
time_t time(time_t *out){extern unsigned long cli_unix_time(void);time_t t=cli_unix_time();if(out)*out=t;return t;}
