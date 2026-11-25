"""Matched parser CPU/copy-work experiment; host seconds are not SE timings."""
from pathlib import Path
import subprocess,sys
r=Path(__file__).resolve().parents[2];w=r/'work/userland-perf/profile';w.mkdir(parents=True,exist_ok=True)
ref=sys.argv[1] if len(sys.argv)>1 else None
files=['ast.c','array.c','buffer.c','arithm.c','parser/parser.c','parser/program.c','parser/word.c','parser/arithm.c']
for name in files:
 p=w/name;p.parent.mkdir(exist_ok=True)
 source=subprocess.check_output(['git','show',ref+':src/vendor/mrsh/'+name]).decode() if ref else (r/'src/vendor/mrsh'/name).read_text()
 if name=='parser/parser.c':
  source='unsigned long long moved;\n'+source
  source=source.replace('memmove(parser->buf.data, parser->buf.data + n, parser->buf.len - n);','moved += parser->buf.len - n; memmove(parser->buf.data, parser->buf.data + n, parser->buf.len - n);')
  source=source.replace('memmove(parser->buf.data, parser_data(parser), parser->buf.len);','moved += parser->buf.len; memmove(parser->buf.data, parser_data(parser), parser->buf.len);')
 p.write_text(source)
for h in (r/'src/vendor/mrsh/include').rglob('*.h'):
 target=w/'include'/h.relative_to(r/'src/vendor/mrsh/include');target.parent.mkdir(parents=True,exist_ok=True)
 target.write_bytes(subprocess.check_output(['git','show',ref+':'+str(h.relative_to(r))]) if ref else h.read_bytes())
subprocess.run(['cc','-O2','-D_POSIX_C_SOURCE=200809L','-I'+str(w/'include'),str(r/'tests/userland-perf/bench.c'),*[str(w/f) for f in files],'-o',str(w/'bench')],check=True)
subprocess.run([str(w/'bench')],check=True)
