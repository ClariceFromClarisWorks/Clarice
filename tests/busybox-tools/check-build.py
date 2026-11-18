"""Validate all experimental CLX sizes/imports and retain reproducible hashes."""
from pathlib import Path
import re,struct,json,hashlib
r=Path(__file__).resolve().parents[2]
exports=set(re.findall(r'CLX_IMPORT\("([^"]+)"',(r/'src/clx-imports.inc').read_text()));rows=[]
for name in json.loads((r/'src/ports/busybox-tools/help.json').read_text()):
 p=r/'work/busybox-tools'/('bb'+name+'.clx');b=p.read_bytes();f=struct.unpack('>4s11I',b[:48]);magic,version,size,entry,nr,ni=f[:6]
 assert magic==b'CLX1' and version==2 and f[8:10]==(1,9)
 assert 4<=size<=262144 and entry<size and entry%2==0 and nr<=8192 and ni<=128
 assert len(b)==48+size+12*nr+64*ni
 names=[b[48+size+12*nr+64*i:48+size+12*nr+64*(i+1)].split(b'\0')[0].decode() for i in range(ni)]
 assert not set(names)-exports,(name,set(names)-exports)
 rows.append(dict(name=name,package_bytes=len(b),image_bytes=size,relocations=nr,imports=ni,sha256=hashlib.sha256(b).hexdigest()))
print(json.dumps(rows,indent=2))
