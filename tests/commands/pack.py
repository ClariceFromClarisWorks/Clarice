"""Check CLX1 packer determinism, format and native import/relocation bounds."""
from pathlib import Path
import importlib.util,struct
spec=importlib.util.spec_from_file_location('packer','scripts/pack-command.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
b,names=m.pack('work/build/core.o');assert b==Path('work/build/core.clx').read_bytes()
magic,abi,size,entry,nr,ni,checksum,reserved=struct.unpack_from('>4s7I',b)
assert magic==b'CLX1' and abi==1 and not reserved and entry<size and not entry%2
assert len(b)==32+size+nr*12+ni*64
h=2166136261
for c in b[32:]:h=((h^c)*16777619)&0xffffffff
assert h==checksum
last=-4
for i in range(nr):
 off,kind,target=struct.unpack_from('>III',b,32+size+i*12)
 assert not off%2 and last+4<=off<=size-4
 assert (kind==1 and target<=size) or (kind==2 and target<ni)
 last=off
assert b'cli_read' in names and b'emit' in names
print(f'CLX1 verified: {len(b)} bytes, {size} image bytes, {nr} relocations, {ni} imports')
