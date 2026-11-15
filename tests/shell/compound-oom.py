"""Inject failures only in the newly added compound redirect ownership paths.
Copies production files into ignored work; no fault hooks in shipped code.
"""
from pathlib import Path
import subprocess,os,tempfile,json
r=Path(__file__).resolve().parents[2];w=r/'work/utility-correctness/compound-oom';w.mkdir(parents=True,exist_ok=True)
for name in ['ast.c','parser/program.c']:
 s=(r/'src/vendor/mrsh'/name).read_text()
 s='#include <stddef.h>\nextern void *fault_alloc(size_t,size_t);\nextern long fault_add(void *,void *);\n'+s
 s=s.replace('calloc(1, sizeof *cmd->redirects)','fault_alloc(1, sizeof *cmd->redirects)').replace('calloc(1, sizeof *copy->redirects)','fault_alloc(1, sizeof *copy->redirects)')
 s=s.replace('mrsh_array_add(cmd->redirects, redir)','fault_add(cmd->redirects, redir)').replace('mrsh_array_add(copy->redirects,r)','fault_add(copy->redirects,r)')
 (w/Path(name).name).write_text(s)
(w/'fault.c').write_text('''#include <stdlib.h>
#include <mrsh/array.h>
#include <stdio.h>
static int calls;
static int fail(void){const char *s=getenv("COMPOUND_FAIL");int hit=s&&++calls==atoi(s);if(hit)fprintf(stderr,"INJECTED:%d\\n",calls);return hit;}
void *fault_alloc(size_t a,size_t b){return fail()?NULL:calloc(a,b);}
long fault_add(void *a,void *v){return fail()?-1:mrsh_array_add(a,v);}
''')
files=['tests/shell/host.c',str(w/'ast.c'),str(w/'program.c'),str(w/'fault.c')]+['src/vendor/mrsh/'+x for x in ['array.c','buffer.c','arithm.c','parser/parser.c','parser/word.c','parser/arithm.c']]
subprocess.run(['cc','-std=c99','-g','-fsanitize=address,undefined','-Wno-multichar','-Isrc/vendor/mrsh/include','-Isrc/vendor/mrsh/parser',*files,'-o',str(w/'test')],cwd=r,check=True)
source="f() { { echo original; } > output; }; f; x=temp true; f; echo END"
rows=[]
for n in range(1,19):
 with tempfile.TemporaryDirectory() as td:
  p=subprocess.run([str(w/'test'),source],cwd=td,capture_output=True,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0','COMPOUND_FAIL':str(n)},timeout=5)
  ok=p.returncode in [0,2] and b'Sanitizer' not in p.stderr and b'leaked native' not in p.stderr
  if n<=2:ok=ok and not (Path(td)/'output').exists() and p.returncode==2
  rows.append(dict(failure=n,injected=b'INJECTED:' in p.stderr,status=p.returncode,passed=ok))
(r/'outputs/utility-correctness/compound-oom.json').write_text(json.dumps(rows,indent=2)+'\n')
assert all(x['passed'] for x in rows),rows
print(f'PASS 18 allocation-position trials: {sum(x["injected"] for x in rows)} injected failures; no crashes or open refs')
