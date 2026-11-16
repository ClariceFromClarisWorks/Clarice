"""Regression for the original parser-construction OOM crash.
The historical unsafe result remains in outputs/shell-elastic.
"""
from pathlib import Path
import subprocess,json,os
r=Path(__file__).resolve().parents[2];w=r/'work/shell-elastic/parser-probe';w.mkdir(parents=True,exist_ok=True)
s=(r/'src/vendor/mrsh/parser/parser.c').read_text().replace('calloc(1, sizeof(struct mrsh_parser))','(getenv("PARSER_PROBE_FAIL") ? NULL : calloc(1, sizeof(struct mrsh_parser)))')
(w/'parser.c').write_text(s)
files=['tests/shell/host.c',str(w/'parser.c')]+['src/vendor/mrsh/'+x for x in ['ast.c','array.c','buffer.c','arithm.c','parser/program.c','parser/word.c','parser/arithm.c']]
subprocess.run(['cc','-std=c99','-g','-fsanitize=address,undefined','-Wno-multichar','-Isrc/vendor/mrsh/include','-Isrc/vendor/mrsh/parser',*files,'-o',str(w/'probe')],cwd=r,check=True)
p=subprocess.run([str(w/'probe'),':'],capture_output=True,env={**os.environ,'PARSER_PROBE_FAIL':'1','UBSAN_OPTIONS':'halt_on_error=1'},timeout=5)
safe=p.returncode==2 and b'out of memory' in p.stderr and b'Sanitizer' not in p.stderr and b'runtime error:' not in p.stderr
assert safe,(p.returncode,p.stderr)
(r/'outputs/parser-safety/constructor.json').write_text(json.dumps(dict(handled_safely=safe,status=p.returncode,evidence=p.stderr.decode().strip(),scope='host parser construction failure'),indent=2)+'\n')
print('PASS parser construction failure returns status 2 without dereferencing NULL')
