"""All parser and adapter allocations share a deterministic fault injector.
The production source is unchanged; this host-only harness reuses host.c I/O.
"""
from pathlib import Path
import subprocess, json, tempfile
r=Path(__file__).resolve().parents[2];w=r/'work/parser-safety';w.mkdir(exist_ok=True,parents=True)
s=(r/'tests/shell/host.c').read_text().split('int main(')[0]
s=s.replace('#include "alloc.h"','').replace('"../../src/', '"'+str(r/'src')+'/')
s+='''
static size_t measured;
static int trial(const char *source,size_t at,int sticky,int cancel){
 pf_fail=0; pf_hit=0; pf_sticky=0; polls=cancel_after=0;
 if(shell_run("seed=kept; original() { echo original; }"))return 120;
 pf_calls=0; pf_peak=pf_live; pf_fail=at; pf_sticky=sticky;cancel_after=cancel;
 int rc=shell_run(source); size_t calls=pf_calls; measured=calls; int hit=pf_hit;
 pf_fail=0;cancel_after=polls=0;
 if(input_ref!=-1||output_ref!=-1||error_ref!=-3)return 121;
 for(int i=0;i<128;i++)if(refs[i])return 122;
 if(shell_run("printf RECOVERED; original; test \\"$seed\\" = kept"))return 123;
 sh_functions_free(sh_functions);sh_functions=0;
 free(sh_owned_args);sh_owned_args=0;sh_argv=sh_initial_argv;sh_argc=1;env_free(sh_env);sh_env=0;
 if(pf_live)return 124;
 fprintf(stderr,"TRIAL calls=%zu at=%zu hit=%d status=%d live=%zu\\n",calls,at,hit,rc,pf_live);
 return 0;
}
int main(int argc,char **argv){if(argc<3)return 2;if(!strcmp(argv[1],"all")){int rc=trial(argv[2],0,0,0);if(rc)return rc;size_t n=measured;for(int sticky=0;sticky<2;sticky++)for(size_t at=1;at<=n;at++){rc=trial(argv[2],at,sticky,0);if(rc){fprintf(stderr,"FAILED at=%zu sticky=%d rc=%d\\n",at,sticky,rc);return rc;}}fprintf(stderr,"PASS trials=%zu\\n",n*2);return 0;}return trial(argv[2],strtoul(argv[1],0,10),argc>3&&!strcmp(argv[3],"sticky"),argc>4?atoi(argv[4]):0);}
'''
(w/'shell.c').write_text(s)
files=[str(w/'shell.c'),str(w/'fault.o')]+[str(p) for p in (r/'src/vendor/mrsh').glob('*.c')]+[str(p) for p in (r/'src/vendor/mrsh/parser').glob('*.c')]
subprocess.run(['cc','-std=c99','-D_POSIX_C_SOURCE=200809L','-g','-fsanitize=address,undefined','-Wno-multichar','-I'+str(r/'src/vendor/mrsh/include'),'-include',str(r/'tests/parser/fault.h'),*files,'-o',str(w/'shell')],check=True)

import re
cases=["f() { echo new; }; f; x=$(echo hi); { echo a; } > result", "while read x; do echo \"$x\"; done <<EOF > result\none\ntwo\nEOF\ncat result", "( seed=changed; original() { echo changed; }; original ); original", "for x in one two; do echo \"$x\"; done | cat", "echo $((1+2*3)); f() { (echo nested) > result; }; f"]
rows=[]
for source in cases:
 with tempfile.TemporaryDirectory() as td:
  p=subprocess.run([str(w/'shell'),'all',source],cwd=td,capture_output=True,timeout=45)
  if p.returncode: raise AssertionError((source,p.returncode,p.stderr.decode()[-4500:]))
  n=int(re.search(rb'PASS trials=(\d+)',p.stderr)[1]);rows.append(dict(source=source,trials=n,passed=True))
  print('PASS recovery',n)
(r/'outputs/parser-safety/shell-recovery.json').write_text(json.dumps(rows,indent=2)+'\n')
