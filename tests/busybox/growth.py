"""Compile the actual patched vi growth routine with deterministic failure."""
from pathlib import Path
import subprocess
s=Path('work/busybox/vi.c').read_text();a=s.index('static uintptr_t text_hole_make(char *p, int size)\t');b=s.index('\n// close a hole',a)
prefix=r'''
#include <assert.h>
#include <stdint.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#define ENABLE_FEATURE_VI_YANKMARK 0
static char *text,*end,*dot,*screenbegin;static int text_size,fail;
static jmp_buf escape;
static void bb_simple_error_msg_and_die(const char *s){(void)s;longjmp(escape,1);}
static void *xrealloc(void *p,size_t n){if(fail)longjmp(escape,1);void *q=malloc(n);assert(q);memcpy(q,p,text_size);free(p);return q;}
'''
suffix=r'''
int main(void){text_size=16;text=malloc(16);memcpy(text,"abcdefghijklmno",16);end=text+15;dot=screenbegin=text;
 char *old=text;fail=1;if(!setjmp(escape)){text_hole_make(text+3,8);abort();}
 assert(text==old&&end==text+15&&text_size==16&&!memcmp(text,"abcdefghijklmno",15));
 fail=0;text_hole_make(text+3,8);assert(end-text==23&&!memcmp(text,"abc        defghijklmno",23));
 old=text;if(!setjmp(escape)){text_hole_make(text,INT_MAX);abort();}assert(text==old&&end-text==23);free(text);return 0;}
'''
p=Path('work/busybox/growth-test.c');p.write_text(prefix+s[a:b]+suffix)
subprocess.run(['clang','-fsanitize=address,undefined','-g',str(p),'-o','work/busybox/growth-test'],check=True)
subprocess.run(['work/busybox/growth-test'],check=True)
print('PASS actual vi growth: allocation failure preserves old bounds/content; moving growth; overflow rejection')
