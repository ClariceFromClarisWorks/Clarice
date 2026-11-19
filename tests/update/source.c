#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#define fsRdPerm 1
#define fsWrPerm 2
#define paramErr -50
#define ioErr -36
#define userCanceledErr -128
static unsigned char input[10000],output[10000];
static long length,offset,written;static int live,cancel,readfail,writefail,closefail,grow,polls,cancel_after;
static unsigned long NetParseIP(const char *s){unsigned a,b,c,d;char x;if(sscanf(s,"%u.%u.%u.%u%c",&a,&b,&c,&d,&x)!=4||a>255||b>255||c>255||d>255)return 0;return (a<<24)|(b<<16)|(c<<8)|d;}
static int open_path(const char *p,int mode,short *r){if(!strcmp(p,"/missing"))return -43;*r=mode;live++;return 0;}
static int GetEOF(short r,long *n){(void)r;*n=length+(grow&&offset==length);return 0;}
static int io_read(short r,void *p,long n){assert(r==1);if(readfail&&(readfail==1||offset>=2048))return ioErr;memcpy(p,input+offset,n);offset+=n;return 0;}
static int io_write(short r,const void *p,long n){assert(r==2);if(writefail)return ioErr;memcpy(output+written,p,n);written+=n;return 0;}
static int FSClose(short r){(void)r;live--;return closefail?ioErr:0;}
static int app_yield(void){return cancel||(cancel_after&&++polls>=cancel_after);}
#include "../../src/update/source.h"
static void reset(long n){length=n;offset=written=live=cancel=readfail=writefail=closefail=grow=polls=cancel_after=0;memset(output,0,sizeof output);}
int main(void){
 assert(update_source_valid("/mnt/Macintosh Exchange/Clarice/index.txt"));
 assert(update_source_valid("http://192.168.1.112:8087/repo/index.txt"));
 const char *bad[]={"ftp://10.0.0.1/index.txt","http://8.8.8.8/index.txt","relative/index.txt","//server/index.txt","/mnt/share/","/mnt/a\n/index.txt",""};
 for(unsigned i=0;i<sizeof bad/sizeof *bad;i++)assert(!update_source_valid(bad[i]));
 for(unsigned i=0;i<sizeof input;i++)input[i]=(unsigned char)i;
 for(int n=0;n<=10000;n+=1000){reset(n);assert(!update_copy_source("/mnt/Share With Spaces/pkg.bin","/tmp/snapshot",10000));assert(written==n&&!memcmp(input,output,n)&&!live);}
 reset(10000);assert(update_copy_source("/x","/tmp/y",9999)==paramErr&&!live&&!written);
 reset(10000);cancel=1;assert(update_copy_source("/x","/tmp/y",0)==userCanceledErr&&!live&&!written);
 reset(10000);readfail=1;assert(update_copy_source("/x","/tmp/y",0)==ioErr&&!live);
 reset(10000);writefail=1;assert(update_copy_source("/x","/tmp/y",0)==ioErr&&!live);
 reset(10000);cancel_after=2;assert(update_copy_source("/x","/tmp/y",0)==userCanceledErr&&!live&&written==2048);
 reset(10000);readfail=2;assert(update_copy_source("/x","/tmp/y",0)==ioErr&&!live&&written==2048);
 reset(10000);closefail=1;assert(update_copy_source("/x","/tmp/y",0)==ioErr&&!live);
 reset(10000);grow=1;assert(update_copy_source("/x","/tmp/y",0)==ioErr&&!live);
 reset(1);assert(update_copy_source("/missing","/tmp/y",0)==-43&&!live);
 puts("PASS source selection, exact binary copy, limits, cancellation, read/write/close failures, changed length, descriptor cleanup");
}
