#include <assert.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
static int fail_stat,fail_resize,fail_seek,resizes;
static int port_fstat(int fd,struct stat *st){if(fail_stat){errno=EIO;return -1;}return fstat(fd,st);}
static int port_ftruncate(int fd,off_t n){if(fail_resize){errno=ENOSPC;return -1;}resizes++;return ftruncate(fd,n);}
static off_t port_lseek(int fd,off_t n,int w){struct stat st;assert(!fstat(fd,&st));if(fail_seek||n>st.st_size){errno=EINVAL;return -1;}return lseek(fd,n,w);}
#include "../../src/ports/vi/temp.inc"
int main(void){
 FILE *f=tmpfile();assert(f);int fd=fileno(f);char b[1024],readback[1024];memset(b,'A',sizeof b);
 assert(!vi_temp_seek(fd,2048,1)&&resizes==1);assert(write(fd,b,sizeof b)==sizeof b);
 assert(!vi_temp_seek(fd,3072,1)&&resizes==1);memset(b,'B',sizeof b);assert(write(fd,b,sizeof b)==sizeof b);
 assert(!vi_temp_seek(fd,2048,0));assert(read(fd,readback,sizeof readback)==sizeof readback&&readback[0]=='A'&&readback[1023]=='A');
 assert(!vi_temp_seek(fd,3072,0));assert(read(fd,readback,sizeof readback)==sizeof readback&&!memcmp(b,readback,sizeof b));
 assert(!vi_temp_seek(fd,0,0));assert(read(fd,readback,sizeof readback)==sizeof readback);for(int i=0;i<1024;i++)assert(!readback[i]);
 assert(vi_temp_seek(fd,5000,0)<0&&resizes==1);
 fail_stat=1;assert(vi_temp_seek(fd,5000,1)<0);fail_stat=0;
 fail_resize=1;assert(vi_temp_seek(fd,5000,1)<0&&resizes==1);fail_resize=0;
 fail_seek=1;assert(vi_temp_seek(fd,0,0)<0);fail_seek=0;
 assert(!vi_temp_seek(fd,0,0));fclose(f);puts("PASS: HFS-style EOF bound, gap extension, distinct block readback, zero gap, metadata/resize/seek failures");
}
