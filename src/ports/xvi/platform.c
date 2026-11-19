/* Clarice xvi adaptation, Clarice contributors, 18 November 2026.
 * Distributed under licenses/xvi/COPYING. No Unix process/permission emulation. */
#define Line mac_Line
#define normal mac_normal
#define erase mac_erase_enum
#define OK mac_OK_enum
#include <Files.h>
#include <Errors.h>
#undef Line
#undef normal
#undef erase
#undef OK
#include "compat.h"
#include "xvi.h"
extern unsigned long cli_ticks(void);
extern int app_screen_open(void),app_screen_rows(void),app_columns(void),app_screen_pending(void),app_yield(void);
extern void app_screen_close(void);
extern int cli_pause_ticks(unsigned long),cli_remove(const char *),resolve_ex(const char *,char *,char *,int),info(const char *,CInfoPBRec *);
/* Existing runtime 1.9 binary session ownership; no Core modifications. */
extern long cli_remote_wire(int,void *,unsigned long);
extern int xvi_runtime_main(int,char **);
unsigned int LI,CO;
int cost_goto=8;
bool_t can_scroll_area=FALSE,can_del_line=FALSE,can_ins_line=FALSE,can_inschar=FALSE,subshells=FALSE;
static int remote,opened;
static char output[512];static unsigned output_used;
void flush_output(void){if(!output_used)return;long n=remote?cli_remote_wire(3,output,output_used):port_write(1,output,output_used);if(n!=(long)output_used)port_exit(1);output_used=0;}
void outchar(int c){if(output_used==sizeof output)flush_output();output[output_used++]=c;}
void outstr(char *s){while(*s)outchar((unsigned char)*s++);}
void tty_goto(int row,int col){char b[32];snprintf(b,sizeof b,"\033[%d;%dH",row+1,col+1);outstr(b);}
void erase_line(void){outstr("\033[K");}
void erase_display(void){outstr("\033[2J\033[H");}
void set_colour(int c){outstr(c==2||c==3?"\033[7m":"\033[0m");}
void alert(void){outchar(7);}
void tty_linefeed(void){outstr("\r\n");}
/* Optional operations never called: capability flags above are false. */
void insert_a_line(void){port_abort();}void delete_a_line(void){port_abort();}
void inschar(int c){(void)c;port_abort();}
void scroll_up(int a,int b,int n){(void)a;(void)b;(void)n;port_abort();}
void scroll_down(int a,int b,int n){(void)a;(void)b;(void)n;port_abort();}
void sys_init(void){
 if(!cli_isatty(0)||!cli_isatty(1)){fputs("xvi: interactive terminal required\n",stderr);port_exit(2);}
 remote=cli_remote_wire(0,0,0)>0;
 if(remote){if(cli_remote_wire(1,0,0)){fputs("xvi: Telnet BINARY negotiation and empty input queue required\n",stderr);port_exit(1);}opened=1;LI=24;CO=80;}
 else{if(app_screen_open()){fputs("xvi: cannot open local screen\n",stderr);port_exit(1);}opened=1;LI=app_screen_rows();CO=app_columns();}
 outstr("\033[?25h\033[0m");
 char key[2]={0,0};int codes[]={K_UARROW,K_DARROW,K_RARROW,K_LARROW};char *seq[]={"\033[A","\033[B","\033[C","\033[D"};
 for(int i=0;i<4;i++){key[0]=codes[i];xvi_keymap(seq[i],key);}
}
void sys_startv(void){}
void sys_endv(void){if(opened){outstr("\033[0m");tty_goto(LI-1,0);erase_line();flush_output();}}
void sys_exit(int code){sys_endv();port_exit(code);}
static int byte_in(long milliseconds){unsigned long start=cli_ticks(),limit=(milliseconds+16)/17;unsigned char c;
 flush_output();
 for(;;){long n;
  if(remote){n=cli_remote_wire(2,&c,1);if(n<0)port_exit(1);if(n)return c;}
  else if(app_screen_pending()||!milliseconds){n=port_read(0,&c,1);if(n<0)port_exit(130);if(n)return c;}
  if(milliseconds&&cli_ticks()-start>=limit)return EOF;
  if(cli_pause_ticks(1))port_exit(130);
 }
}
int inchar(long ms){return byte_in(ms);}
void Wait200ms(void){if(cli_pause_ticks(12))port_exit(130);}
int call_system(char *s){(void)s;errno=ENOSYS;return -1;}
int call_shell(char *s){return call_system(s);}
bool_t sys_pipe(char *s,int (*w)(FILE *),long (*r)(FILE *)){(void)s;(void)w;(void)r;errno=ENOSYS;return FALSE;}
char *fexpand(char *s,bool_t complete){(void)complete;return s;}
int xvi_chdir(const char *s){(void)s;errno=ENOSYS;return -1;}
char *xvi_getcwd(char *p,size_t n){char *s=port_curses_getenv("PWD");if(!s||strlen(s)>=n){errno=ERANGE;return NULL;}strcpy(p,s);return p;}
int xvi_remove(const char *s){return cli_remove(s);}
int xvi_access(const char *s,int mode){char path[512];CInfoPBRec pb;int e=resolve_ex(s,path,0,1);if(!e)e=info(path,&pb);if(e){errno=e==fnfErr||e==dirNFErr?ENOENT:EIO;return -1;}if((mode&W_OK)&&(pb.hFileInfo.ioFlAttrib&1)){errno=EACCES;return -1;}return 0;}
char *tempfname(char *s){(void)s;char p[64];static unsigned serial;for(int i=0;i<32;i++){snprintf(p,sizeof p,"/tmp/xvi-%08lx-%u",cli_ticks(),++serial);int fd=port_open(p,O_WRONLY|O_CREAT|O_EXCL,0600);if(fd>=0){if(port_close(fd))return NULL;return strdup(p);}if(errno!=EEXIST)return NULL;}return NULL;}
#include "save.inc"
int upstream_main(int argc,char **argv){
 if(argc==2&&(!strcmp(argv[1],"--help")||!strcmp(argv[1],"--version"))){
 puts("xvi: STEVIE-derived portable vi, upstream 109d2f39849e (2.50 version string)");
 if(!strcmp(argv[1],"--version"))return 0;
 puts("Usage: xvi [-R] [-s setting] [-c command] [-t tag] [file ...]\nAlso: +line or +/pattern before file; -w lines is accepted but ignored.\nvi keys: i insert, Escape normal, :w save, :q quit, :q! discard.\nClarice investigation build: local screen or 80x24 ANSI Telnet BINARY.\nSafe HFS replacement retains resource forks and Finder metadata.\nUnsupported: shell/filter commands, :cd, append writes, wildcard completion.\nByte-oriented text; Unix LF default. No OS crash recovery guarantee.\nExperimental: unsaved edits can be lost on remote disconnect/cancel.\nStatus: 0 quit/help; 1 startup/failure; 2 adapter usage; 130 cancellation.\nSource and copying terms: licenses/xvi/COPYING; scripts/prepare-xvi.py.");return 0;
 }
 for(int i=1;i<argc;i++)if(argv[i][0]=='-'&&argv[i][1]=='-'){fprintf(stderr,"xvi: unknown option: %s\n",argv[i]);return 2;}
 defscr_main(argc,argv);return 0;
}
void startup_error(char *s){sys_endv();fprintf(stderr,"xvi: %s\n",s);}
int clarice_main(int argc,char **argv){int rc=xvi_runtime_main(argc,argv);if(opened){if(remote)cli_remote_wire(4,0,rc!=0);else app_screen_close();}return rc;}
/* Newlib FILE is already the runtime's public stream representation. */
long atol(const char *s){return strtol(s,NULL,10);}
void rewind(FILE *f){(void)fseek(f,0,SEEK_SET);f->_flags &= ~(__SERR|__SEOF);}
