"""Exercise the production binary import lookup, including aliases and unknowns."""
from pathlib import Path
import re, subprocess, tempfile
root=Path(__file__).resolve().parents[2]
s=(root/'src/module.inc').read_text()
a=s.index('static void *clx_import(');b=s.index('\n#endif\n/* Runtime',a)
lookup=s[a:b]
entries=re.findall(r'CLX_IMPORT\("([^"]+)", ([&\w]+)\)',(root/'src/clx-imports.inc').read_text())
names=[n for n,_ in entries]
assert names==sorted(set(names)), 'export names must be unique and sorted'
# Bindings with deliberately different public and implementation names.
assert dict(entries)['KILLIO']=='module_kill_io'
assert dict(entries)['FSREAD']=='FSRead'
assert dict(entries)['output_ref']=='&output_ref'
for profile in (False,True):
 active=[(n,v) for n,v in entries if profile or n!='app_nano_mark']
 symbols={v.lstrip('&'):v.startswith('&') for _,v in entries}
 declarations='\n'.join('#define '+n+' stub_'+n for n in symbols if n!='strcmp')+'\n'+'\n'.join(('static int '+n+';' if data else 'static void '+n+'(void){}') for n,data in symbols.items() if n!='strcmp' and (profile or n!='app_nano_mark'))
 checks='\n'.join('comparisons=0;assert(clx_import("%s")== (void*)(%s));assert(comparisons<=8);'%(n,v) for n,v in active)
 absent=['','not_an_export','FSRead','fsread','cli_native_open_extra','app_nano_mar','~']
 if not profile:absent+=['app_nano_mark']
 checks+='\n'+'\n'.join('assert(!clx_import("%s"));'%n for n in absent)
 code='''#include <assert.h>
#include <string.h>
static int comparisons;
static int counted(const char *a,const char *b){comparisons++;return strcmp(a,b);}
#define strcmp counted
'''+('#define CLARICE_NANO_PERF 1\n' if profile else '')+declarations+'\n'+lookup+'\nint main(void){'+checks+'return 0;}\n'
 with tempfile.TemporaryDirectory() as d:
  p=Path(d);(p/'test.c').write_text(code)
  subprocess.run(['cc','-std=c99','-Wall','-Wextra','-Wno-incompatible-library-redeclaration','-fsanitize=address,undefined','-I'+str(root/'src'),str(p/'test.c'),'-o',str(p/'test')],check=True)
  subprocess.run([str(p/'test')],check=True)
 print(('profiling' if profile else 'normal')+': '+str(len(active))+' exact bindings, aliases, unknowns; at most 8 comparisons; ASan/UBSan passed')
