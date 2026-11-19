"""Check the existing CLX format limits and imports before test publication."""
import sys,struct,re,json,hashlib
from pathlib import Path
b=Path(sys.argv[1]).read_bytes();fields=struct.unpack('>4s11I',b[:48]);magic,version,size,entry,nr,ni=fields[:6]
assert magic==b'CLX1' and version==2
assert 4<=size<=262144 and entry<size and entry%2==0 and nr<=8192 and ni<=128
assert len(b)==48+size+12*nr+64*ni and fields[8:10]==(1,9)
names=[b[48+size+12*nr+64*i:48+size+12*nr+64*(i+1)].split(b'\0')[0].decode() for i in range(ni)]
exports=set(re.findall(r'CLX_IMPORT\("([^"]+)"',Path(sys.argv[2]).read_text()));assert not set(names)-exports,set(names)-exports
print(json.dumps(dict(package_bytes=len(b),image_bytes=size,relocations=nr,imports=ni,missing_imports=[],sha256=hashlib.sha256(b).hexdigest()),indent=2))
