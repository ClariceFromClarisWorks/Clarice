#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
extern int clarice_main(int,char **);
typedef union HostAllocation {long double align;unsigned long size;} HostAllocation;
static unsigned long host_owned;
void *cli_alloc(unsigned long n){
 const char *limit=getenv("CLARICE_TEST_ALLOC_BUDGET");
 if(limit&&(n>strtoul(limit,0,10)||host_owned>strtoul(limit,0,10)-n)){errno=ENOMEM;return NULL;}
 HostAllocation *p=malloc(sizeof(*p)+n);if(!p)return NULL;p->size=n;host_owned+=n;return p+1;
}
void cli_free(void *p){if(p){HostAllocation *h=(HostAllocation*)p-1;host_owned-=h->size;free(h);}}
void cli_port_stats(unsigned long peak,unsigned long remaining){if(remaining||host_owned)abort();if(getenv("CLARICE_TEST_STATS"))fprintf(stderr,"PORT_STATS peak=%lu remaining=%lu\n",peak,remaining);}
static unsigned long polls;
int app_yield(void){const char *s=getenv("CLARICE_CANCEL_AFTER");return s&&++polls>=strtoul(s,0,10);}
int cli_isatty(int fd){return isatty(fd);}
long cli_read(int fd,void *p,unsigned long n){return read(fd,p,n);}
long cli_write(int fd,const void *p,unsigned long n){if(getenv("CLARICE_FAIL_WRITE")&&fd==1){errno=ENOSPC;return -1;}return write(fd,p,n);}
int cli_file_open(const char *p,const char *m){return open(p,(strchr(m,'+')?O_RDWR:m[0]=='r'?O_RDONLY:O_WRONLY)|(m[0]=='r'?0:O_CREAT|(m[0]=='a'?O_APPEND:m[0]=='x'?O_EXCL:m[0]=='c'?0:O_TRUNC)),0600);}
long cli_file_read(int fd,void *p,unsigned long n){return read(fd,p,n);}
long cli_file_write(int fd,const void *p,unsigned long n){static unsigned writes;const char *fail=getenv("CLARICE_FAIL_FILE_WRITE");if(fail&&++writes>=strtoul(fail,0,10)){errno=ENOSPC;return -1;}return write(fd,p,n);}
int cli_file_close(int fd){return close(fd);}
int main(int argc,char **argv){return clarice_main(argc,argv);}
#include <sys/stat.h>
#include <time.h>
int cli_environment(int index,char *out,unsigned long cap){
 const char *count=getenv("CLARICE_TEST_ENV_COUNT");
 if(count){if(index<0||index>=atoi(count))return 0;char name[32];snprintf(name,sizeof name,"ENV%d=",index);const char *value=getenv("CLARICE_TEST_ENV_VALUE");if(!value)value="value";if(strlen(name)+strlen(value)+1>cap)return -1;strcpy(out,name);strcat(out,value);return 1;}
 const char *names[]={"HOME","PWD","AWK_TEST","TMPDIR"};int at=0;for(int i=0;i<4;i++){const char *value=getenv(names[i]);if(!value)continue;if(at++!=index)continue;size_t n=strlen(names[i]),m=strlen(value);if(n+m+2>cap)return -1;memcpy(out,names[i],n);out[n]='=';memcpy(out+n+1,value,m+1);return 1;}return 0;
}
int cli_path_kind(const char *p){struct stat st;if(stat(p,&st))return -1;return S_ISDIR(st.st_mode);}
unsigned long cli_unix_time(void){return (unsigned long)time(NULL);}

long cli_file_size(int fd){struct stat st;return fstat(fd,&st)?-1:st.st_size;}
long cli_stream_size(int fd){return cli_file_size(fd);}
long cli_file_seek(int fd,long offset,int whence){return lseek(fd,offset,whence);}
int cli_pause_ticks(unsigned long n){for(unsigned long i=0;i<n;i++){if(app_yield())return 1;usleep(16667);}return 0;}
int cli_temp_create(char *path,unsigned long cap){const char *dir=getenv("CLARICE_TEST_TMP");if(!dir)dir="/tmp";if(strlen(dir)+32>=cap){errno=ENAMETOOLONG;return -1;}strcpy(path,dir);strcat(path,"/clarice-sort-XXXXXX");int fd=mkstemp(path);if(fd<0)return -1;return close(fd);}
int cli_remove(const char *path){return remove(path);}
unsigned long cli_ticks(void){const char *fixed=getenv("CLARICE_TEST_TICKS");if(fixed)return strtoul(fixed,0,10);struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return (unsigned long)t.tv_sec*60+(unsigned long)t.tv_nsec*60/1000000000;}

int cli_file_resize(int fd,long size){return ftruncate(fd,size);}

int resolve_ex(const char *path,char *native,char *canonical,int follow){(void)follow;if(strlen(path)>=512)return -37;strcpy(native,path);strcpy(canonical,path);return 0;}
int protected_path(const char *p){return !strcmp(p,"/");}
int cli_native_mkdir(const char *p){
 const char *failure=getenv("CLARICE_MKDIR_ERROR");if(failure)return atoi(failure);
 if(!mkdir(p,0700))return 0;
 switch(errno){case EEXIST:return -48;case ENOENT:return -120;case EACCES:return -54;case ENOSPC:return -34;default:return -36;}
}
