from pathlib import Path
import subprocess
import runpy,tempfile,atexit,shutil
root=Path(__file__).resolve().parents[2]
patch=runpy.run_path(str(root/'scripts/patch-nano-growth.py'))
block=patch['GROWTH']
assert patch['apply'](patch['apply'](patch['OLD']))==block
try: patch['apply']('unexpected upstream source')
except ValueError: pass
else: raise AssertionError('source mismatch accepted')
build=Path(tempfile.mkdtemp(prefix='clarice-nano-growth-'))
atexit.register(shutil.rmtree, build)
prefix=r'''
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <setjmp.h>
#undef SIZE_MAX
#define SIZE_MAX UINT32_MAX
static size_t oldsize, ceiling=SIZE_MAX;
static jmp_buf escape;
static void die(const char *s){(void)s;longjmp(escape,1);}
static void *moving(void *p,size_t n){if(n>ceiling)return NULL;void *q=malloc(n);assert(q);memcpy(q,p,oldsize<n?oldsize:n);free(p);oldsize=n;return q;}
static void *nrealloc(void *p,size_t n){void *q=moving(p,n);if(!q)die("");return q;}
#define realloc moving
#define _(s) s
#define LUMPSIZE 120
static char *grow(char *buf,size_t *cap){size_t bufsize=*cap;
'''
suffix=r'''
*cap=bufsize;return buf;}
int main(void){
 for(int mode=0;mode<3;mode++){
 size_t cap=120;oldsize=cap;char *p=malloc(cap);assert(p);
 for(size_t n=0;n<65536;n++){
  p[n]=(char)n;
  if(n+1==cap){ceiling=mode==0?SIZE_MAX:mode==1?cap+120:cap;
   if(mode==2){if(!setjmp(escape)){grow(p,&cap);abort();}for(size_t j=0;j<=n;j++)assert((unsigned char)p[j]==(unsigned char)j);ceiling=SIZE_MAX;}
   p=grow(p,&cap);
   for(size_t j=0;j<=n;j++)assert((unsigned char)p[j]==(unsigned char)j);
  }
 }
 free(p);
 }
 size_t cap=SIZE_MAX-119;char byte=1;if(!setjmp(escape)){grow(&byte,&cap);abort();}assert(byte==1);
 puts("PASS geometric growth: moving allocations, exact fallback, failure preserves bytes, overflow and 64 KiB lines");
}
'''
(build/'growth.c').write_text(prefix+block+suffix)
subprocess.run(['cc','-std=c99','-fsanitize=address,undefined',str(build/'growth.c'),'-o',str(build/'growth')],check=True)
subprocess.run([str(build/'growth')],check=True)
