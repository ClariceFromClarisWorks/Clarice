/* Compare the real BSD/newlib buffer fast path with libc, including refill errors. */
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "../../src/ports/nano/buffered-getc.h"
#undef getc
struct Source {int pos,fail;};
static int read_source(void *v,char *p,int n){struct Source *s=v;if(s->fail&&s->pos>=512){errno=EINTR;return -1;}if(s->pos==4096)return 0;if(n>4096-s->pos)n=4096-s->pos;for(int i=0;i<n;i++)p[i]=(char)((s->pos+i)%256);s->pos+=n;return n;}
static FILE *stream(struct Source *s,char *buf,int n){FILE *f=funopen(s,read_source,0,0,0);assert(f);assert(!setvbuf(f,buf,_IOFBF,n));return f;}
int main(void){
 for(int size=1;size<=4096;size*=8){
  struct Source a={0,0},b={0,0};char ab[4096],bb[4096];FILE *fa=stream(&a,ab,size),*fb=stream(&b,bb,size);
  for(int i=0;i<4096;i++){int x=getc(fa),y=port_nano_getc(fb);assert(x==i%256&&x==y);if(i%97==0){assert(ungetc(42,fa)==42&&ungetc(42,fb)==42);assert(getc(fa)==port_nano_getc(fb));}}
  assert(getc(fa)==EOF&&port_nano_getc(fb)==EOF&&feof(fa)&&feof(fb));
  clearerr(fa);clearerr(fb);assert(ungetc(255,fa)==255&&ungetc(255,fb)==255);assert(getc(fa)==port_nano_getc(fb));
  assert(!fclose(fa)&&!fclose(fb));
 }
 struct Source s={0,1};char buf[512];FILE *f=stream(&s,buf,512);for(int i=0;i<512;i++)assert(port_nano_getc(f)==i%256);
 assert(port_nano_getc(f)==EOF&&ferror(f)&&errno==EINTR);clearerr(f);s.fail=0;assert(port_nano_getc(f)==0&&!ferror(f));assert(!fclose(f));
 puts("PASS nano buffered getc: binary bytes, buffer boundaries, pushback, EOF, interrupted refill and recovery; libc owns streams");
}
