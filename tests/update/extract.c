#include <assert.h>
#include <stdio.h>
#include "../../src/update/extract.h"
static unsigned char source[8192],forks[2][4096];
static unsigned at,offset[2],which;static int opened,calls,fail;
static int fault(void){return ++calls==fail?-36:0;}
static int seek(void *c,uint32_t n){(void)c;int e=fault();if(!e)at=n;return e;}
static int read_block(void *c,void *p,unsigned n){(void)c;int e=fault();if(!e){assert(at+n<=sizeof source);memcpy(p,source+at,n);at+=n;}return e;}
static int begin(void *c,int fork){(void)c;int e=fault();assert(!opened);if(!e){opened=1;which=fork;}return e;}
static int write_block(void *c,const void *p,unsigned n){(void)c;int e=fault();assert(opened);if(!e){assert(offset[which]+n<=4096);memcpy(forks[which]+offset[which],p,n);offset[which]+=n;}return e;}
static int end(void *c){(void)c;assert(opened);opened=0;return fault();}
static int poll(void *c){(void)c;return fault();}
int main(void){UpdatePackage p={3333,2777,3584,6400,0,0,0};UpdateExtractOps o={0,seek,read_block,begin,write_block,end,poll};for(unsigned i=0;i<sizeof source;i++)source[i]=(unsigned char)(i*17+i/256);
 assert(!update_extract(&o,&p));int total=calls;assert(!opened&&offset[0]==3333&&offset[1]==2777&&!memcmp(forks[0],source+128,3333)&&!memcmp(forks[1],source+3584,2777));
 for(fail=1;fail<=total;fail++){calls=0;offset[0]=offset[1]=0;assert(update_extract(&o,&p)==-36);assert(!opened);}
 p.data_size=0;p.resource_offset=128;fail=calls=0;offset[0]=offset[1]=0;assert(!update_extract(&o,&p)&&offset[0]==0&&offset[1]==2777);
 puts("PASS: binary preservation of both forks, empty data fork, every seek/open/read/write/poll/close failure; no open writer retained");}
