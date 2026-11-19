#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
static void *save_token;static int save_fd=-1;
static int failure,aborts,commits,closes;
static void *cli_replace_begin(const char *p,char *s,unsigned long n){assert(n>6);strcpy(s,"stage");return failure==1?NULL:(void*)1;}
static int port_open(const char *p,int mode){assert(!strcmp(p,"stage"));assert(mode==(O_WRONLY|O_CREAT|O_TRUNC));return failure==2?-1:3;}
static ssize_t full_write(int fd,const void *p,size_t n){assert(fd==3);return failure==3?-1:failure==4?n-1:n;}
static int port_close(int fd){assert(fd==3);closes++;return failure==5?-1:0;}
static void cli_replace_abort(void *t){assert(t==(void*)1);aborts++;}
static int cli_replace_commit(void *t){assert(t==(void*)1);commits++;return failure==6?-1:0;}
#include "../../src/ports/busybox/save.inc"
int main(void){for(failure=1;failure<=6;failure++){int c=commits,a=aborts;assert(bb_save_file("target","hello",5)==-1);assert(!save_token&&save_fd==-1);assert(commits==c+(failure==6));assert(aborts==a+(failure>=2&&failure<=5));}failure=0;assert(bb_save_file("target","hello",5)==5);assert(bb_save_file("target","",0)==0);assert(!save_token&&save_fd==-1);puts("PASS: begin/open/write/short-write/close/commit failures, retry, empty save");}
