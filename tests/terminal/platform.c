/* Host checks for bounded per-invocation terminal services. */
#define sigaction terminal_sigaction
#define signal terminal_signal
#define raise terminal_raise
#define select terminal_select
#define gettimeofday terminal_gettimeofday
#define setenv terminal_setenv
#define unsetenv terminal_unsetenv
#include "../../src/ports/terminal/platform.c"
#undef malloc
#undef free
#undef memcpy
#undef memset
#undef puts
#undef printf
#undef fprintf
#undef exit
#undef abort
#include <assert.h>
static unsigned long clock_ticks;
static const unsigned char *input_bytes;
static size_t input_left;
static int seen_signal;
void *port_malloc(size_t n){return malloc(n);}
void port_free(void *p){free(p);}
int cli_environment(int i,char *p,unsigned long n){const char *v=i==0?"HOME=/":i==1?"TEST=parent":0;if(!v)return 0;assert(strlen(v)+1<n);strcpy(p,v);return 1;}
unsigned long cli_ticks(void){return clock_ticks;}
int app_columns(void){return 54;}
int app_screen_rows(void){return 25;}
int app_screen_pending(void){return (int)input_left;}
int app_yield(void){return 0;}
void app_screen_flush_input(void){input_left=0;}
int cli_pause_ticks(unsigned long n){clock_ticks+=n;return 0;}
int cli_isatty(int fd){return fd>=0&&fd<=2;}
ssize_t port_read(int fd,void *p,size_t n){assert(fd==0);if(n>input_left)n=input_left;memcpy(p,input_bytes,n);input_bytes+=n;input_left-=n;return n;}
ssize_t port_write(int fd,const void *p,size_t n){(void)fd;(void)p;return n;}
void port_exit(int n){(void)n;assert(!"unexpected default signal exit");abort();}
static void handler(int sig){seen_signal=sig;}
static void input(const char *s){input_bytes=(const unsigned char*)s;input_left=strlen(s);}
int main(void){
 assert(!strcmp(port_curses_getenv("TERM"),"clarice"));
 assert(!strcmp(port_curses_getenv("TEST"),"parent"));
 assert(!setenv("TEST","child",0));assert(!strcmp(port_curses_getenv("TEST"),"parent"));
 assert(!setenv("TEST","child",1));assert(!strcmp(port_curses_getenv("TEST"),"child"));
 assert(setenv("bad=name","x",1)==-1);assert(!unsetenv("TEST"));assert(!port_curses_getenv("TEST"));
 assert(signal(SIGINT,handler)==SIG_DFL);assert(!raise(SIGINT)&&seen_signal==SIGINT);
 assert(signal(SIGINT,SIG_IGN)==handler);seen_signal=0;assert(!raise(SIGINT)&&!seen_signal);
 assert(signal(SIGKILL,handler)==SIG_ERR);signal(SIGINT,handler);
 struct termios t;assert(!tcgetattr(0,&t));char b[8];input("ab\bcd\r");assert(port_terminal_read(0,b,2)==2&&!memcmp(b,"ac",2));assert(port_terminal_read(0,b,8)==2&&!memcmp(b,"d\n",2));
 input("\003");assert(port_terminal_read(0,b,1)==-1&&errno==EINTR&&seen_signal==SIGINT);
 t.c_lflag=0;t.c_iflag=0;t.c_cc[VMIN]=1;assert(!tcsetattr(0,TCSANOW,&t));input("\003");assert(port_terminal_read(0,b,1)==1&&b[0]==3);
 t.c_cc[VMIN]=0;t.c_cc[VTIME]=2;tcsetattr(0,TCSANOW,&t);input("");unsigned long before=clock_ticks;assert(port_terminal_read(0,b,1)==0&&clock_ticks-before==12);
 struct winsize w;assert(!ioctl(0,TIOCGWINSZ,&w)&&w.ws_row==25&&w.ws_col==54);
 fd_set f;FD_ZERO(&f);FD_SET(0,&f);struct timeval wait={0,50000};before=clock_ticks;assert(!select(1,&f,0,0,&wait)&&clock_ticks-before==3&&!FD_ISSET(0,&f));
 input("x");FD_SET(0,&f);assert(select(1,&f,0,0,&wait)==1&&FD_ISSET(0,&f));tcflush(0,TCIFLUSH);assert(!input_left);
 for(int i=0;i<24;i++)free(environment[i]);
 puts("PASS: environment isolation, signal handlers, canonical/raw input, timeout, size and readiness");
}
