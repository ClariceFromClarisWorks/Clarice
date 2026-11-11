/* GNU nano platform boundary. No user database or Unix process model. */
#define erase mac_erase_enum
#define OK mac_OK_enum
#include <Files.h>
#include <Errors.h>
#undef erase
#undef OK
#include "nano-compat.h"
extern int resolve_ex(const char *,char *,char *,int);
extern int info(const char *,CInfoPBRec *);
extern unsigned long cli_ticks(void),cli_unix_time(void);
extern int app_screen_open(void),nano_main(int,char **);
extern int cli_remove(const char *),port_track_temp(const char *);
static int mac_error(int e){errno=e==fnfErr||e==dirNFErr?ENOENT:e==fLckdErr||e==wPrErr||e==permErr?EACCES:e==bdNamErr?ENAMETOOLONG:EIO;return -1;}
int port_nano_stat(const char *path,struct stat *st){char native[512];CInfoPBRec pb;int e=resolve_ex(path,native,0,1);if(!e)e=info(native,&pb);if(e)return mac_error(e);memset(st,0,sizeof *st);int dir=pb.hFileInfo.ioFlAttrib&16;st->st_mode=(dir?S_IFDIR:S_IFREG)|S_IRUSR|((pb.hFileInfo.ioFlAttrib&1)?0:S_IWUSR)|(dir?S_IXUSR:0);st->st_size=dir?0:pb.hFileInfo.ioFlLgLen;st->st_ino=dir?pb.dirInfo.ioDrDirID:pb.hFileInfo.ioDirID;st->st_dev=pb.hFileInfo.ioVRefNum;st->st_nlink=1;unsigned long t=dir?pb.dirInfo.ioDrMdDat:pb.hFileInfo.ioFlMdDat;st->st_mtime=t>=2082844800UL?t-2082844800UL:0;st->st_ctime=st->st_atime=st->st_mtime;return 0;}
int port_nano_access(const char *path,int mode){struct stat st;if(mode&~(R_OK|W_OK|X_OK)){errno=EINVAL;return -1;}if(port_nano_stat(path,&st))return -1;if(((mode&W_OK)&&!(st.st_mode&S_IWUSR))||((mode&X_OK)&&!S_ISDIR(st.st_mode))){errno=EACCES;return -1;}return 0;}
char *port_nano_realpath(const char *path,char *out){char native[512],canonical[512];CInfoPBRec pb;int e=resolve_ex(path,native,canonical,1);if(!e)e=info(native,&pb);if(e){mac_error(e);return 0;}if(!out)return strdup(canonical);strcpy(out,canonical);return out;}
#undef initscr
WINDOW *port_nano_initscr(void){if(app_screen_open())return 0;return initscr();}
unsigned int port_nano_save_id(void){return (unsigned int)cli_ticks();}
int port_nano_putenv(char *value){char name[64],*eq=strchr(value,'=');if(!eq||eq==value||(size_t)(eq-value)>=sizeof name){errno=EINVAL;return -1;}memcpy(name,value,eq-value);name[eq-value]=0;return setenv(name,eq+1,1);}
time_t port_nano_time(time_t *out){time_t t=cli_unix_time();if(out)*out=t;return t;}
int fcntl(int fd,int command,...){if(fd!=0){errno=ENOSYS;return -1;}if(command==F_GETFL)return O_RDONLY;if(command==F_SETFL){va_list ap;va_start(ap,command);int flags=va_arg(ap,int);va_end(ap);if(flags==O_RDONLY)return 0;}errno=ENOSYS;return -1;}
int mkstemps(char *name,int suffix){static unsigned serial;size_t n=strlen(name);if(suffix<0||(size_t)suffix+6>n||strncmp(name,"/tmp/",5)||memcmp(name+n-suffix-6,"XXXXXX",6)){errno=EINVAL;return -1;}char *p=name+n-suffix-6;const char *hex="0123456789abcdef";for(int tries=0;tries<32;tries++){unsigned long id=cli_ticks()+ ++serial;for(int i=0;i<6;i++)p[i]=hex[(id>>(4*i))&15];int fd=port_open(name,O_RDWR|O_CREAT|O_EXCL,0600);if(fd>=0){if(port_track_temp(name)){port_close(fd);cli_remove(name);return -1;}return fd;}if(errno!=EEXIST)return -1;}errno=EEXIST;return -1;}
int upstream_main(int argc,char **argv){return nano_main(argc,argv);}

/* POSIX dirname modifies its writable argument; no alternate filesystem. */
char *dirname(char *path){if(!path||!*path)return (char*)".";size_t n=strlen(path);while(n>1&&path[n-1]=='/')path[--n]=0;char *slash=strrchr(path,'/');if(!slash)return (char*)".";while(slash>path&&slash[-1]=='/')slash--;if(slash==path){path[1]=0;return path;}*slash=0;return path;}
