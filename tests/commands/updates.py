"""Focused host regression entry point; uses existing pinned-toolchain objects."""
from pathlib import Path
import importlib.util,re,subprocess,struct
root=Path(__file__).resolve().parents[2]
assert Path.cwd()==root,'run from repository root'
out=root/'outputs/task2';out.mkdir(exist_ok=True)
work=root/'work/task2';work.mkdir(exist_ok=True)
def run(args):
 p=subprocess.run(args,capture_output=True,text=True)
 if p.returncode:raise RuntimeError(p.stdout+p.stderr)
 return p.stdout
run(['cc','-std=c99','-Wall','-Wextra','-Wno-multichar','-Wno-unused-function','-Wno-misleading-indentation','-fsanitize=address,undefined','-g','tests/commands/update-host.c','-o',str(work/'update-host')])
log=run([str(work/'update-host')])
spec=importlib.util.spec_from_file_location('packer','scripts/pack-command.py')
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
# Legacy output stays byte-for-byte stable; versioned output preserves its payload.
log+=run(['python3','tests/commands/pack.py'])
for version in (1,2):
 b,names=m.pack(work/f'probe{version}.o',runtime=(1,0) if version==2 else None)
 (work/f'probe{version}.clx').write_bytes(b)
b,names=m.pack('work/build/core.o',(1,0));(work/'core-runtime1.clx').write_bytes(b)
legacy,_=m.pack('work/build/core.o');assert b[48:]==legacy[32:]
assert struct.unpack_from('>III',b,32)==(1,0,1)
h=2166136261
for c in b[:44]:h=((h^c)*16777619)&0xffffffff
assert struct.unpack_from('>I',b,44)[0]==h
s=Path('src/module.inc').read_text()
names=set(re.findall(r'(?:EXPORT|DATA)\((\w+)\)',s))|set(re.findall(r'!strcmp\(name,"([^"]+)"\)',s));names.discard('symbol')
(work/'exports.txt').write_text('\n'+'\n'.join(sorted(names))+'\n')
images=sorted(Path('work/build').glob('*.clx'))+sorted(work.glob('*.clx'))
log+=run([str(work/'update-host'),str(work/'exports.txt'),*map(str,images)])
(out/'host-tests.txt').write_text(log);print(log,end='')
