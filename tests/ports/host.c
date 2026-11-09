#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
extern int clarice_main(int,char **);
void *cli_alloc(unsigned long n){return malloc(n);}
void cli_free(void *p){free(p);}
void cli_port_stats(unsigned long peak,unsigned long remaining){if(peak>96UL*1024||remaining)abort();}
static unsigned long polls;
int app_yield(void){const char *s=getenv("CLARICE_CANCEL_AFTER");return s&&++polls>=strtoul(s,0,10);}
int cli_isatty(int fd){return isatty(fd);}
long cli_read(int fd,void *p,unsigned long n){return read(fd,p,n);}
long cli_write(int fd,const void *p,unsigned long n){if(getenv("CLARICE_FAIL_WRITE")&&fd==1){errno=ENOSPC;return -1;}return write(fd,p,n);}
int cli_file_open(const char *p,const char *m){return open(p,m[0]=='r'?O_RDONLY:O_WRONLY|O_CREAT|(m[0]=='a'?O_APPEND:O_TRUNC),0600);}
long cli_file_read(int fd,void *p,unsigned long n){return read(fd,p,n);}
long cli_file_write(int fd,const void *p,unsigned long n){static unsigned writes;const char *fail=getenv("CLARICE_FAIL_FILE_WRITE");if(fail&&++writes>=strtoul(fail,0,10)){errno=ENOSPC;return -1;}return write(fd,p,n);}
int cli_file_close(int fd){return close(fd);}
int main(int argc,char **argv){return clarice_main(argc,argv);}
#include <sys/stat.h>
#include <time.h>
int cli_environment(int index,char *out,unsigned long cap){const char *names[]={"HOME","PWD","AWK_TEST"};if(index<0||index>=3)return 0;const char *value=getenv(names[index]);if(!value)return 0;size_t n=strlen(names[index]),m=strlen(value);if(n+m+2>cap)return 0;memcpy(out,names[index],n);out[n]='=';memcpy(out+n+1,value,m+1);return 1;}
int cli_path_kind(const char *p){struct stat st;if(stat(p,&st))return -1;return S_ISDIR(st.st_mode);}
unsigned long cli_unix_time(void){return (unsigned long)time(NULL);}

long cli_file_size(int fd){struct stat st;return fstat(fd,&st)?-1:st.st_size;}
long cli_stream_size(int fd){return cli_file_size(fd);}
long cli_file_seek(int fd,long offset,int whence){return lseek(fd,offset,whence);}
int cli_pause_ticks(unsigned long n){for(unsigned long i=0;i<n;i++){if(app_yield())return 1;usleep(16667);}return 0;}
int cli_temp_create(char *path,unsigned long cap){const char *dir=getenv("CLARICE_TEST_TMP");if(!dir)dir="/tmp";if(strlen(dir)+32>=cap){errno=ENAMETOOLONG;return -1;}strcpy(path,dir);strcat(path,"/clarice-sort-XXXXXX");int fd=mkstemp(path);if(fd<0)return -1;return close(fd);}
int cli_remove(const char *path){return unlink(path);}
