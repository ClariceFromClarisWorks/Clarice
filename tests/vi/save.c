#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
static int fail_begin,fail_open,fail_close,fail_commit,aborts,commits;
static void *cli_replace_begin(const char *p,char *s,unsigned long n){(void)p;assert(n>8);strcpy(s,"stage");return fail_begin?NULL:(void*)1;}
static void cli_replace_abort(void *p){assert(p==(void*)1);aborts++;}
static int cli_replace_commit(void *p){assert(p==(void*)1);commits++;return fail_commit?-1:0;}
static int port_open(const char *p,int mode){assert(!strcmp(p,"stage"));assert(mode==(O_WRONLY|O_CREAT|O_TRUNC));return fail_open?-1:3;}
static int port_close(int fd){assert(fd>=3);if(fail_close){errno=EIO;return -1;}return 0;}
#include "../../src/ports/vi/save.inc"
int main(void){
 fail_begin=1;assert(vi_save_create("target")==-1);fail_begin=0;
 fail_open=1;assert(vi_save_create("target")==-1&&aborts==1);fail_open=0;
 assert(vi_save_create("target")==3);assert(vi_save_create("other")==-1&&errno==EBUSY);
 vi_save_written(3,7,7);assert(vi_save_close(3)==0&&commits==1);
 assert(vi_save_create("target")==3);vi_save_written(3,4,7);assert(vi_save_close(3)==-1&&aborts==2&&commits==1);
 assert(vi_save_create("target")==3);fail_close=1;assert(vi_save_close(3)==-1&&aborts==3);fail_close=0;
 assert(vi_save_create("target")==3);fail_commit=1;assert(vi_save_close(3)==-1&&commits==2);fail_commit=0;
 assert(vi_save_create("target")==3);assert(vi_save_abort_close(3)==0&&aborts==4);
 assert(vi_save_create("target")==3);assert(vi_save_close(3)==0&&commits==3);
 assert(vi_save_close(4)==0&&commits==3);assert(!save_token&&save_fd==-1);
 puts("PASS: begin/open/short-write/close/commit failures; error unwind; retry; success; unrelated descriptor");
}
