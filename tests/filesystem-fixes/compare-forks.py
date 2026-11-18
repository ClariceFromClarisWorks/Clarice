"""Compare offline MacBinary exports after Macintosh Shut Down."""
from pathlib import Path
import hashlib,json
p=Path('outputs/filesystem-fixes/forks')
def read(name):
 b=(p/name).read_bytes();n=int.from_bytes(b[83:87],'big');q=int.from_bytes(b[87:91],'big');off=128+((n+127)//128)*128
 assert len(b)>=off+q
 return dict(data_bytes=n,resource_bytes=q,data_sha256=hashlib.sha256(b[128:128+n]).hexdigest(),resource_sha256=hashlib.sha256(b[off:off+q]).hexdigest(),type_creator=b[65:73].decode(),timestamps=b[91:99].hex())
a,b=read('source.bin'),read('moved.bin');assert a==b
(p/'comparison.json').write_text(json.dumps(dict(source=a,moved=b,equal=True),indent=2)+'\n')
print('PASS persisted data/resource bytes, type/creator and timestamps')
