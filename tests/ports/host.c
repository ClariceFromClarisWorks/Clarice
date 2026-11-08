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
long cli_file_write(int fd,const void *p,unsigned long n){return write(fd,p,n);}
int cli_file_close(int fd){return close(fd);}
int main(int argc,char **argv){return clarice_main(argc,argv);}
