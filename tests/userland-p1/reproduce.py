"""Reproduce the accepted review baseline without changing the checkout."""
from pathlib import Path
import subprocess,tempfile,os
r=Path.cwd()
def old(p):return subprocess.check_output(['git','show','28f187f:'+p],text=True)
with tempfile.TemporaryDirectory() as d:
 d=Path(d);(d/'sh.inc').write_text(old('src/sh.inc'))
 host=(r/'tests/shell/host.c').read_text().replace('../../src/',str(r/'src')+'/').replace(str(r/'src/sh.inc'),str(d/'sh.inc'))
 (d/'host.c').write_text(host)
 sources=['ast.c','array.c','buffer.c','arithm.c','parser/parser.c','parser/program.c','parser/word.c','parser/arithm.c']
 (d/'word.c').write_text(old('src/vendor/mrsh/parser/word.c'))
 args=[str(d/'word.c') if s=='parser/word.c' else str(r/'src/vendor/mrsh'/s) for s in sources]
 subprocess.run(['cc','-std=c99','-g','-fsanitize=address,undefined','-Wno-multichar','-I'+str(r/'src/vendor/mrsh/include'),'-I'+str(r/'tests/shell'),'-I'+str(r/'src'),str(d/'host.c'),*args,'-o',str(d/'shell')],check=True)
 a=subprocess.run([str(d/'shell'),'X=y true'],capture_output=True,text=True);assert a.returncode==123;print('REPRO saved assignment frame remains linked after free')
 script=r'case abc in \*) echo BAD;; *) echo good;; esac'
 a=subprocess.run([str(d/'shell'),script],capture_output=True,text=True);assert a.stdout=='BAD\n';print('REPRO escaped wildcard becomes active case pattern')
 (d/'module.inc').write_text(old('src/module.inc'))
 host=(r/'tests/commands/update-host.c').read_text().replace('../../src/',str(r/'src')+'/').replace(str(r/'src/module.inc'),str(d/'module.inc'))
 (d/'module-test.c').write_text(host)
 subprocess.run(['cc','-g','-fsanitize=address,undefined','-Wno-multichar','-I'+str(r/'src'),str(d/'module-test.c'),'-o',str(d/'module')],check=True)
 a=subprocess.run([str(d/'module')],capture_output=True,text=True);assert 'double-free' in a.stderr;print('REPRO failed FSClose double-frees relocation metadata')
