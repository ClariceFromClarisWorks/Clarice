#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
static int fault,opened,closed,renamed,writes;static size_t total;
static unsigned long cli_ticks(void){return 7;}
int cli_file_open(const char *p,const char *m){assert(!strcmp(m,"x"));assert(strstr(p,"/etc/clarice/vi-"));opened++;if(fault==1){errno=ENOSPC;return -1;}if(fault==2&&opened==1){errno=EEXIST;return -1;}return 9;}
long cli_file_write(int f,const void *p,unsigned long n){assert(f==9&&n<=4096);writes++;if(fault==3)return -1;if(fault==4)n=n>1?n/2:1;total+=n;return n;}
int cli_file_close(int f){assert(f==9);closed++;return fault==5?-1:0;}
int resolve_ex(const char *p,char *q,char *unused,int follow){(void)unused;assert(follow);strcpy(q,p);return fault==6?-1:0;}
int cli_native_rename(const char *a,const char *b){assert(strstr(a,".part")&&strstr(b,".recover"));renamed++;return fault==7?-1:0;}
#include "../../src/ports/busybox/recover.inc"
int main(void){char data[8192];memset(data,'X',sizeof data);for(fault=0;fault<=7;fault++){opened=closed=renamed=writes=0;total=0;bb_preserve("original",data,sizeof data);assert(closed==(fault!=1));assert(renamed==(fault==0||fault==2||fault==4||fault==7));if(!fault||fault==2||fault==4)assert(total==sizeof data);if(fault==2)assert(opened==2);}fault=0;bb_preserve(NULL,data,0);puts("PASS recovery: exclusive collisions, bounded short writes, create/write/close/resolve/rename failures, empty buffer");}
