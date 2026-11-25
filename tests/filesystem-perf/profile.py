"""Build disposable instrumentation around actual utility entry points, not Core.
Enumeration counts include the known two catalog/two volume calls in native_nth;
resolver/internal-copy calls are intentionally outside these scoped counters.
"""
from pathlib import Path
import subprocess,sys
r=Path.cwd();w=r/'work/filesystem-perf';variant=sys.argv[1]
pre=r'''
#include "/src/src/commands/mac-files.h"
static unsigned long allocations,live,peak,requested,infos,nths,catalogs,polls,yield_ticks,last,maxgap;
static void *measure_alloc(unsigned long n){allocations++;requested+=n;unsigned long *p=cli_alloc(n+sizeof *p);if(!p)return 0;*p=n;live+=n;if(live>peak)peak=live;return p+1;}
static void measure_free(void *p){if(p){unsigned long *q=(unsigned long*)p-1;live-=*q;cli_free(q);}}
static int measure_info(const char*p,CInfoPBRec*b){infos++;return info(p,b);}
static int measure_nth(const char*p,int n,char*l,CInfoPBRec*b){nths++;return cli_native_nth(p,n,l,b);}
static int measure_catalog(CInfoPBRec*b){catalogs++;return PBGetCatInfoSync(b);}
static int measure_yield(void){unsigned long t=cli_ticks(),gap=t-last;if(gap>maxgap)maxgap=gap;polls++;int e=app_yield();last=cli_ticks();yield_ticks+=last-t;return e;}
#define cli_alloc measure_alloc
#define cli_free measure_free
#define info measure_info
#define cli_native_nth measure_nth
#define PBGetCatInfoSync measure_catalog
#define app_yield measure_yield
#define clarice_main measured_main
'''
post=r'''
#undef clarice_main
int clarice_main(int argc,char**argv){if(argc<2)return 2;char b[300];unsigned long start=last=cli_ticks();int rc=measured_main(argc-1,argv+1);unsigned long ticks=cli_ticks()-start;snprintf(b,sizeof b,"PROFILE ticks=%lu yields=%lu yield_ticks=%lu maxgap=%lu allocs=%lu requested=%lu peak=%lu live=%lu enum_catalog=%lu enum_volume=%lu info=%lu status=%d\n",ticks,polls,yield_ticks,maxgap,allocations,requested,peak,live,catalogs+2*nths,2*nths,infos,rc);errsay(b);return rc;}
'''
for name in ('files','inspect'):
 src=(subprocess.check_output(['git','show','0f82946:src/commands/'+name+'.c'],text=True) if variant=='before' else (r/'src/commands'/f'{name}.c').read_text())
 src=src.replace('#include "mac-files.h"','').replace('#include "../help-output.h"','#include "/src/src/help-output.h"').replace('#include "file-progress.h"','#include "/src/src/commands/file-progress.h"')
 (w/f'{variant}-{name}.c').write_text(pre+src+post)
image='ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c'
cmd='''set -e
for name in files inspect; do
 m68k-apple-macos-gcc -m68000 -Os -ffunction-sections -fdata-sections -Wno-multichar -c /src/work/filesystem-perf/V-$name.c -o /src/work/filesystem-perf/V-$name-main.o
 extra=""; if [ "$name" = inspect ]; then extra=/src/work/build/fnmatch-byte.o; fi
 m68k-apple-macos-ld -r --gc-sections -u clarice_main /src/work/filesystem-perf/V-$name-main.o $extra "$(m68k-apple-macos-gcc -print-libgcc-file-name)" -o /src/work/filesystem-perf/V-$name.o
done'''.replace('V-',variant+'-')
subprocess.run(['docker','run','--rm','--pull','never','--platform','linux/amd64','--network','none','-v',str(r)+':/src',image,'sh','-ec',cmd],check=True)
for name in ('files','inspect'):subprocess.run([sys.executable,'scripts/pack-command.py',str(w/f'{variant}-{name}.o'),str(w/f'{variant}-{name}-profile.clx'),'--runtime','1.10'],check=True)
