"""Reproduce the pre-existing upstream parser OOM gap, without shipped hooks.
This is an investigation probe: observing the unsafe dereference is NOT a pass
for allocation-failure handling. The adapter's separate failure tests must pass.
"""
from pathlib import Path
import subprocess,json,os
r=Path(__file__).resolve().parents[2];w=r/'work/shell-elastic/parser-probe';w.mkdir(parents=True,exist_ok=True)
s=(r/'src/vendor/mrsh/parser/parser.c').read_text().replace('calloc(1, sizeof(struct mrsh_parser))','(getenv("PARSER_PROBE_FAIL") ? NULL : calloc(1, sizeof(struct mrsh_parser)))')
(w/'parser.c').write_text(s)
files=['tests/shell/host.c',str(w/'parser.c')]+['src/vendor/mrsh/'+x for x in ['ast.c','array.c','buffer.c','arithm.c','parser/program.c','parser/word.c','parser/arithm.c']]
subprocess.run(['cc','-std=c99','-g','-fsanitize=address,undefined','-Wno-multichar','-Isrc/vendor/mrsh/include','-Isrc/vendor/mrsh/parser',*files,'-o',str(w/'probe')],cwd=r,check=True)
p=subprocess.run([str(w/'probe'),':'],capture_output=True,env={**os.environ,'PARSER_PROBE_FAIL':'1','UBSAN_OPTIONS':'halt_on_error=1'},timeout=5)
unsafe=b'member access within null pointer' in p.stderr
assert unsafe,(p.returncode,p.stderr)
(r/'outputs/shell-elastic/parser-oom-gap.json').write_text(json.dumps(dict(handled_safely=False,reproduced=unsafe,status=p.returncode,evidence=p.stderr.decode().splitlines()[0].replace(str(r)+'/', ''),scope='host-only parser construction failure; no guest fault injected'),indent=2)+'\n')
print('CONFIRMED pre-existing parser constructor dereferences NULL on allocation failure')
