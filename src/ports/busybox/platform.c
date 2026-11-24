#include "../../help-output.h"
#include <Files.h>
#include <Errors.h>
#include "libbb.h"
#include "../safe-save.h"
extern int cli_pause_ticks(unsigned long),app_screen_open(void),app_screen_rows(void),app_columns(void),app_screen_pending(void);
extern void app_screen_close(void);
extern unsigned long cli_ticks(void);
extern long cli_remote_wire(int,void *,unsigned long);
extern int resolve_ex(const char *,char *,char *,int),info(const char *,CInfoPBRec *);
extern int bb_runtime_main(int,char **),vi_main(int,char **);
struct globals *ptr_to_globals;
int bb_optind;
#ifdef CLARICE_VI_PROFILE
unsigned long bb_profile[5];
#endif
static int remote,opened,look=-1;
static unsigned char out[1024];static size_t outlen;
static void *save_token;static int save_fd=-1;
void bb_simple_error_msg_and_die(const char *s){fprintf(stderr,"vi: %s\n",s);port_exit(1);}
void *xmalloc(size_t n){void *p=malloc(n);if(!p)bb_simple_error_msg_and_die("out of memory");return p;}
void *xzalloc(size_t n){void *p=xmalloc(n);memset(p,0,n);return p;}
void *xrealloc(void *p,size_t n){p=realloc(p,n);if(!p)bb_simple_error_msg_and_die("out of memory");return p;}
char *xstrdup(const char *s){return s?strcpy(xmalloc(strlen(s)+1),s):NULL;}
char *xstrndup(const char *s,size_t n){size_t k=0;while(k<n&&s[k])k++;char *p=xmalloc(k+1);memcpy(p,s,k);p[k]=0;return p;}
char *xasprintf(const char *fmt,...){va_list ap,aq;va_start(ap,fmt);va_copy(aq,ap);int n=vsnprintf(NULL,0,fmt,ap);va_end(ap);if(n<0)bb_simple_error_msg_and_die("format error");char *p=xmalloc(n+1);vsnprintf(p,n+1,fmt,aq);va_end(aq);return p;}
char *strchrnul(const char *s,int c){while(*s&&*s!=c)s++;return (char*)s;}
unsigned bb_strtou(const char *s,char **end,int base){char *e;errno=0;unsigned long v=strtoul(s,&e,base);if(e==s||(!end&&*e)||v>UINT_MAX)errno=ERANGE;if(end)*end=e;return v;}
int index_in_strings(const char *list,const char *key){int i=0;while(*list){if(!strcmp(list,key))return i;list+=strlen(list)+1;i++;}return -1;}
ssize_t bb_write(int fd,const void *p,size_t n){ssize_t got=opened&&remote&&fd==1?cli_remote_wire(3,(void*)p,n):port_write(fd,p,n);if(n&&got==0){errno=EIO;got=-1;}if(opened&&fd==1&&got!=(ssize_t)n)port_exit(1);return got;}
void fflush_all(void){if(outlen){size_t n=outlen;outlen=0;bb_write(1,out,n);}}
void bb_out(const void *p,size_t n){while(n){size_t k=sizeof(out)-outlen;if(k>n)k=n;memcpy(out+outlen,p,k);outlen+=k;p=(const char*)p+k;n-=k;if(outlen==sizeof(out))fflush_all();}}
void fputs_stdout(const char *s){bb_out(s,strlen(s));}
void bb_putchar(int c){unsigned char ch=c;bb_out(&ch,1);}
int bb_poll(struct pollfd *p,unsigned n,int ms){if(n!=1||p->fd!=0||p->events!=POLLIN){errno=EINVAL;return -1;}unsigned long start=cli_ticks(),ticks=ms<0?0:((unsigned long)ms*60+999)/1000;
 for(;;){if(look>=0){p->revents=POLLIN;return 1;}if(remote){unsigned char c;long got=cli_remote_wire(2,&c,1);if(got<0)port_exit(1);if(got){look=c;continue;}}else if(app_screen_pending()){p->revents=POLLIN;return 1;}if(ms>=0&&cli_ticks()-start>=ticks){p->revents=0;return 0;}if(cli_pause_ticks(1))port_exit(130);}}
ssize_t bb_read(int fd,void *p,size_t n){if(fd!=0)return port_read(fd,p,n);if(!n)return 0;struct pollfd f={0,POLLIN,0};if(bb_poll(&f,1,-1)<0)return -1;if(look>=0){*(unsigned char*)p=look;look=-1;return 1;}return port_read(fd,p,1);}
int get_terminal_width_height(int fd,unsigned *cols,unsigned *rows){(void)fd;*cols=remote?80:app_columns();*rows=remote?24:app_screen_rows();return 0;}
void set_termios_to_raw(int fd,struct termios *old,int flags){struct termios t;(void)flags;if(tcgetattr(fd,old))bb_simple_error_msg_and_die("terminal modes unavailable");t=*old;t.c_lflag=0;t.c_iflag=0;t.c_oflag=0;if(tcsetattr(fd,TCSANOW,&t))bb_simple_error_msg_and_die("raw mode failed");}
void tcsetattr_stdin_TCSANOW(struct termios *old){tcsetattr(0,TCSANOW,old);}
int bb_stat(const char *s,struct stat *st){char path[512];CInfoPBRec pb;int e=resolve_ex(s,path,0,1);if(!e)e=info(path,&pb);if(e){errno=e==fnfErr||e==dirNFErr?ENOENT:EIO;return -1;}memset(st,0,sizeof *st);st->st_mode=(pb.hFileInfo.ioFlAttrib&16)?S_IFDIR:S_IFREG;st->st_size=pb.hFileInfo.ioFlLgLen;return 0;}
int bb_access(const char *s,int mode){char path[512];CInfoPBRec pb;int e=resolve_ex(s,path,0,1);if(!e)e=info(path,&pb);if(e){errno=ENOENT;return -1;}if((mode&W_OK)&&(pb.hFileInfo.ioFlAttrib&1)){errno=EACCES;return -1;}return 0;}
#include "save.inc"
#include "recover.inc"

static int vi_help(void){return help_output(1,
 "Usage:\n"
 "  vi [-R] [-c command] [file ...]\n"
 "Edit byte-oriented text with BusyBox 1.37.0 vi. Safe HFS saves preserve metadata.\n"
 "Options:\n"
 "  -R\tRead-only mode.\n"
 "  -c COMMAND\tRun a startup command. EXINIT is also supported.\n"
 "Keys: i insert, Escape normal, :w save, :q quit, :q! discard.\n"
 "Supports literal search and substitution, undo, repeat, marks and named buffers.\n"
 "Remote editing uses 80x24 Telnet BINARY. No regex, full ex, shell/filter, job control or automatic .exrc.\n"
 "After failure or disconnect, check /etc/clarice/vi-*.recover for unsaved text. Recovery needs free disk space; .part files may be incomplete.\n"
 "Status: 0 quit/help, 1 failure, 2 usage, 130 cancelled.\n");}
void bb_show_usage(void){(void)vi_help();}
int bb_vi_options(int argc,char **argv,llist_t **cmds){int flags=0;optind=1;while(optind<argc&&argv[optind][0]=='-'){char *s=argv[optind++];if(!strcmp(s,"--"))break;if(!strcmp(s,"-R"))flags|=8;else if(!strcmp(s,"-c")&&optind<argc)llist_add_to_end(cmds,xstrdup(argv[optind++]));else {fprintf(stderr,"vi: unsupported option: %s\n",s);port_exit(2);}}return flags;}
int upstream_main(int argc,char **argv){if(argc==2&&(!strcmp(argv[1],"--help")||!strcmp(argv[1],"--version")||!strcmp(argv[1],"-h")||!strcmp(argv[1],"-H")))return vi_help();for(int i=1;i<argc&&argv[i][0]=='-';i++){if(!strcmp(argv[i],"--"))break;if(!strcmp(argv[i],"-R"))continue;if(!strcmp(argv[i],"-c")&&i+1<argc){i++;continue;}fprintf(stderr,"vi: unsupported option: %s\n",argv[i]);return 2;}if(!cli_isatty(0)||!cli_isatty(1)){fputs("vi: terminal required\n",stderr);return 2;}remote=cli_remote_wire(0,0,0)>0;if(remote){if(cli_remote_wire(1,0,0)){fputs("vi: Telnet BINARY required\n",stderr);return 1;}}else if(app_screen_open())return 1;opened=1;int rc=vi_main(argc,argv);fflush_all();
#ifdef CLARICE_VI_PROFILE
fprintf(stderr,"vi profile: sync=%lu format=%lu status=%lu cmd=%lu commands=%lu\n",bb_profile[0],bb_profile[1],bb_profile[2],bb_profile[3],bb_profile[4]);
#endif
return rc;}
int clarice_main(int argc,char **argv){int rc=bb_runtime_main(argc,argv);if(save_token){if(save_fd>=0)port_close(save_fd);cli_replace_abort(save_token);save_token=NULL;}if(opened){if(remote)cli_remote_wire(4,0,rc!=0);else app_screen_close();}return rc;}

/* BusyBox libbb/read.c; Copyright 1999-2004 Erik Andersen; GPLv2 or later. */
ssize_t FAST_FUNC full_read(int fd, void *buf, size_t len)
{
	ssize_t cc;
	ssize_t total;

	total = 0;

	while (len) {
		cc = safe_read(fd, buf, len);

		if (cc < 0) {
			if (total) {
				/* we already have some! */
				/* user can do another read to know the error code */
				return total;
			}
			return cc; /* read() returns -1 on failure. */
		}
		if (cc == 0)
			break;
		buf = ((char *)buf) + cc;
		total += cc;
		len -= cc;
	}

	return total;
}

/* Copyright (C) 2005 Free Software Foundation, Inc.
 * memrchr() is a GNU function that might not be available everywhere.
 * It's basically the inverse of memchr() - search backwards in a
 * memory block for a particular character.
 */
void* FAST_FUNC memrchr(const void *s, int c, size_t n)
{
	const char *start = s, *end = s;

	end += n - 1;

	while (end >= start) {
		if (*end == (char)c)
			return (void *) end;
		end--;
	}

	return NULL;
}
