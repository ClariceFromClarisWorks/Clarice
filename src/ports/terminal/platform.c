/* Virtual terminal services for upstream ncurses. No OS signals, processes,
   serial-port control, or arbitrary-descriptor select is emulated. */
#include "ncurses-compat.h"
#include "termios.h"
#include "sys/ioctl.h"
#include <stdarg.h>
extern unsigned long cli_ticks(void);
extern int app_columns(void),app_screen_rows(void),app_screen_pending(void),app_yield(void);
extern void app_screen_flush_input(void);
extern int cli_pause_ticks(unsigned long);
static unsigned char canonical[256];
static size_t canonical_count,canonical_at;
static struct termios modes={ICRNL,OPOST|ONLCR,CS8,ICANON|ECHO|ISIG,{4,0,8,3,21,1,28,17,19,26,0},B38400,B38400};
int tcgetattr(int fd,struct termios *out){if(!out||!cli_isatty(fd)){errno=ENOTTY;return -1;}*out=modes;return 0;}
int tcsetattr(int fd,int action,const struct termios *in){if(!in||!cli_isatty(fd)){errno=ENOTTY;return -1;}if(action<0||action>2){errno=EINVAL;return -1;}modes=*in;if(action==TCSAFLUSH){app_screen_flush_input();canonical_count=canonical_at=0;}return 0;}
int tcflush(int fd,int queue){if(!cli_isatty(fd)||queue<0||queue>2){errno=EINVAL;return -1;}if(queue!=TCOFLUSH){app_screen_flush_input();canonical_count=canonical_at=0;}return 0;}
speed_t cfgetospeed(const struct termios *t){return t->c_ospeed;}
speed_t cfgetispeed(const struct termios *t){return t->c_ispeed;}
int cfsetospeed(struct termios *t,speed_t s){t->c_ospeed=s;return 0;}
int cfsetispeed(struct termios *t,speed_t s){t->c_ispeed=s;return 0;}
int ioctl(int fd,unsigned long request,...){va_list ap;va_start(ap,request);void *p=va_arg(ap,void*);va_end(ap);if(!cli_isatty(fd)||!p){errno=ENOTTY;return -1;}if(request==TIOCGWINSZ){struct winsize *w=p;memset(w,0,sizeof *w);w->ws_row=app_screen_rows();w->ws_col=app_columns();return 0;}if(request==FIONREAD){*(int*)p=app_screen_pending();return 0;}errno=ENOTTY;return -1;}
/* A private, bounded environment snapshot for this invocation. */
extern int cli_environment(int,char *,unsigned long);
static char *environment[24];
static int env_ready;
static int env_slot(const char *name){size_t n=strlen(name);for(int i=0;i<24;i++)if(environment[i]&&!strncmp(environment[i],name,n)&&environment[i][n]=='=')return i;return -1;}
static int env_store(const char *name,const char *value,int overwrite){
 if(!name||!*name||strchr(name,'=')||!value){errno=EINVAL;return -1;}
 int i=env_slot(name);if(i>=0&&!overwrite)return 0;if(i<0)for(i=0;i<24&&environment[i];i++);
 size_t a=strlen(name),b=strlen(value);if(i==24||a+b+2>1024){errno=ENOMEM;return -1;}
 char *p=malloc(a+b+2);if(!p)return -1;memcpy(p,name,a);p[a]='=';memcpy(p+a+1,value,b+1);free(environment[i]);environment[i]=p;return 0;
}
static void env_init(void){if(env_ready)return;env_ready=1;char line[1024];for(int i=0;i<18;i++)if(cli_environment(i,line,sizeof line)>0){char *eq=strchr(line,'=');if(eq){*eq=0;env_store(line,eq+1,1);}}env_store("TERM","clarice",1);}
char *port_curses_getenv(const char *name){env_init();int i=env_slot(name);return i<0?0:strchr(environment[i],'=')+1;}
int setenv(const char *name,const char *value,int overwrite){env_init();return env_store(name,value,overwrite);}
int unsetenv(const char *name){env_init();if(!name||!*name||strchr(name,'=')){errno=EINVAL;return -1;}int i=env_slot(name);if(i>=0){free(environment[i]);environment[i]=0;}return 0;}
/* Synchronous signal delivery inside the active command; no process IDs,
   asynchronous OS delivery, job control or catchable hardware faults. */
typedef void (*Handler)(int);
static Handler handlers[5];
static sigset_t handler_masks[5],blocked_signals,pending_signals;
static int signal_slot(int sig){return sig==SIGINT?0:sig==SIGTERM?1:sig==SIGHUP?2:sig==SIGQUIT?3:sig==SIGCONT?4:-1;}
Handler signal(int sig,Handler handler){int i=signal_slot(sig);if(i<0||handler==SIG_ERR){errno=EINVAL;return SIG_ERR;}Handler old=handlers[i];handlers[i]=handler;return old;}
int sigaction(int sig,const struct sigaction *action,struct sigaction *old){int i=signal_slot(sig);if(i<0||(action&&action->sa_flags)){errno=EINVAL;return -1;}if(old){memset(old,0,sizeof *old);old->sa_handler=handlers[i];old->sa_mask=handler_masks[i];}if(action){handlers[i]=action->sa_handler;handler_masks[i]=action->sa_mask;}return 0;}
#ifndef sigemptyset
int sigemptyset(sigset_t *set){*set=0;return 0;}
#endif
#ifndef sigfillset
int sigfillset(sigset_t *set){*set=(sigset_t)~0UL;return 0;}
#endif
static sigset_t signal_bit(int sig){return (sigset_t)1 << (sig-1);}
int raise(int sig){int i=signal_slot(sig);if(i<0){errno=EINVAL;return -1;}sigset_t bit=signal_bit(sig);if(blocked_signals&bit){pending_signals|=bit;return 0;}Handler h=handlers[i];if(h==SIG_IGN)return 0;if(h==SIG_DFL)port_exit(128+sig);sigset_t saved=blocked_signals;blocked_signals|=handler_masks[i]|bit;h(sig);blocked_signals=saved;int supported[]={SIGINT,SIGTERM,SIGHUP,SIGQUIT,SIGCONT};for(int j=0;j<5;j++){bit=signal_bit(supported[j]);if((pending_signals&bit)&&!(blocked_signals&bit)){pending_signals&=~bit;raise(supported[j]);}}return 0;}
/* Keyboard VINTR is a signal only in ISIG mode. raw() leaves Ctrl+C as data. */
static ssize_t canonical_result(void *buffer,size_t n){size_t left=canonical_count-canonical_at;if(n>left)n=left;memcpy(buffer,canonical+canonical_at,n);canonical_at+=n;if(canonical_at==canonical_count)canonical_at=canonical_count=0;return n;}
ssize_t port_terminal_read(int fd,void *buffer,size_t n){
 if(fd!=0||!cli_isatty(fd))return port_read(fd,buffer,n);
 if(!n)return 0;
 if(canonical_at)return canonical_result(buffer,n);
 unsigned long start=cli_ticks();
 for(;;){unsigned char c;
  if(!(modes.c_lflag&ICANON)&&!modes.c_cc[VMIN]&&!app_screen_pending()){
   if(!modes.c_cc[VTIME]||cli_ticks()-start>=(unsigned long)modes.c_cc[VTIME]*6)return 0;
   if(cli_pause_ticks(1)){errno=EINTR;return -1;}continue;
  }
  ssize_t got=port_read(fd,&c,1);if(got<=0)return got;
  if((modes.c_lflag&ISIG)&&c==modes.c_cc[VINTR]){canonical_count=canonical_at=0;raise(SIGINT);errno=EINTR;return -1;}
  if((modes.c_iflag&IGNCR)&&c=='\r')continue;
  if((modes.c_iflag&ICRNL)&&c=='\r')c='\n';else if((modes.c_iflag&INLCR)&&c=='\n')c='\r';
  if(modes.c_lflag&ICANON){
   if(c==modes.c_cc[VERASE]||c==127){if(canonical_count){canonical_count--;if(modes.c_lflag&ECHO)port_write(1,"\b \b",3);}continue;}
   if(c==modes.c_cc[VKILL]){while(canonical_count){canonical_count--;if(modes.c_lflag&ECHO)port_write(1,"\b \b",3);}continue;}
   if(c==modes.c_cc[VEOF])return canonical_result(buffer,n);
   if(canonical_count==sizeof canonical-1&&c!='\n'){if(modes.c_lflag&ECHO)port_write(1,"\a",1);continue;}
   canonical[canonical_count++]=c;
   if(modes.c_lflag&ECHO)port_write(1,&c,1);
   if(c=='\n')return canonical_result(buffer,n);
  }else{if(modes.c_lflag&ECHO)port_write(1,&c,1);*(unsigned char*)buffer=c;return 1;}
 }
}
int select(int nfds,fd_set *readfds,fd_set *writefds,fd_set *exceptfds,struct timeval *tv){
 if(nfds<0||nfds>3||writefds||exceptfds||(tv&&(tv->tv_sec<0||tv->tv_usec<0||tv->tv_usec>=1000000))||(readfds&&(FD_ISSET(1,readfds)||FD_ISSET(2,readfds)))){errno=EINVAL;return -1;}int wants=readfds&&FD_ISSET(0,readfds);unsigned long start=cli_ticks(),ticks=tv?(unsigned long)tv->tv_sec*60+(tv->tv_usec+16666)/16667:0;
 for(;;){if(wants&&app_screen_pending()){FD_ZERO(readfds);FD_SET(0,readfds);return 1;}if(tv&&cli_ticks()-start>=ticks){if(readfds)FD_ZERO(readfds);return 0;}if(cli_pause_ticks(1)){errno=EINTR;return -1;}}
}
/* ncurses uses elapsed time for keyboard deadlines, not civil time. */
int gettimeofday(struct timeval *tv,void *zone){(void)zone;if(!tv)return -1;unsigned long t=cli_ticks();tv->tv_sec=t/60;tv->tv_usec=(t%60)*1000000UL/60;return 0;}
