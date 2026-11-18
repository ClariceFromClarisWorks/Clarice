/* Exercise the actual adapter with failing native operations; no HFS claims. */
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
static int fail_begin,fail_open,fail_close,fail_commit,aborts,commits;
static void *cli_replace_begin(const char *p,char *s,unsigned long n){(void)p;assert(n>8);strcpy(s,"stage");return fail_begin?NULL:(void*)1;}
static void cli_replace_abort(void *p){assert(p==(void*)1);aborts++;}
static int cli_replace_commit(void *p){assert(p==(void*)1);commits++;return fail_commit?-1:0;}
static FILE *port_fopen(const char *p,const char *m){assert(!strcmp(p,"stage")&&!strcmp(m,"w"));return fail_open?NULL:tmpfile();}
static int port_fclose(FILE *f){int rc=fclose(f);if(fail_close){errno=EIO;return EOF;}return rc;}
#include "../../src/ports/xvi/save.inc"
int main(void){
 assert(!xvi_save_open("target","a")&&errno==ENOSYS);
 fail_begin=1;assert(!xvi_save_open("target","w"));assert(!aborts&&!commits);fail_begin=0;
 fail_open=1;assert(!xvi_save_open("target","w"));assert(aborts==1&&!commits);fail_open=0;
 FILE *f=xvi_save_open("target","w");assert(f);fputs("new bytes",f);assert(!xvi_save_close(f)&&commits==1);
 fail_close=1;f=xvi_save_open("target","w");assert(xvi_save_close(f)==EOF&&aborts==2&&commits==1);fail_close=0;
 fail_commit=1;f=xvi_save_open("target","w");assert(xvi_save_close(f)==-1&&commits==2);fail_commit=0;
 f=xvi_save_open("target","w");int fd=fileno(f);assert(fd>=0);/* induce stdio error without closing ownership */
 freopen("/dev/null","r",f);assert(fputc('x',f)==EOF);assert(xvi_save_close(f)==EOF&&aborts==3&&commits==2);
 f=tmpfile();assert(!xvi_save_close(f)&&commits==2);
 assert(!replacement&&!save_stream);puts("PASS: append refusal, begin/open/write/close/commit failures, success, unrelated read close");
}
