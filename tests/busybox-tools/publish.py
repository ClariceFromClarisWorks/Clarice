"""Publish prefixed experiments into a private existing Clarice repository."""
from pathlib import Path
import sys,hashlib,binascii,struct,json,subprocess
r=Path(__file__).resolve().parents[2];h=r/'work/busybox-tools-snow/http';w=r/'work/busybox-tools'
subprocess.run([sys.executable,str(r/"tests/busybox-tools/check-build.py")],check=True,stdout=subprocess.DEVNULL)
version=int(sys.argv[1]);rows=(h/'index.txt').read_text().splitlines();records=[]
for name in json.loads((r/'src/ports/busybox-tools/help.json').read_text()):
 name='bb'+name;b=(w/(name+'.clx')).read_bytes();assert struct.unpack_from('>II',b,32)==(1,9)
 digest=hashlib.sha256(b).hexdigest();filename=f'{name}-{digest}.clx';line=f'tool\t{name}\t{version}\t{len(b)}\t{binascii.crc32(b):08x}\t{filename}'
 for old in rows:
  fields=old.split('\t')
  if fields[:3]==['tool',name,str(version)]:assert old==line,'Never reuse a version for changed bytes'
 rows=[s for s in rows if not s.startswith('tool\t'+name+'\t')];rows.append(line);(h/filename).write_bytes(b)
 records.append(dict(name=name,version=version,sha256=digest,package_bytes=len(b),image_bytes=struct.unpack_from('>I',b,8)[0]))
(h/'index.txt.partial').write_text('\n'.join(rows)+'\n');(h/'index.txt.partial').replace(h/'index.txt')
(r/'outputs/busybox-tools/packages.json').write_text(json.dumps(records,indent=2)+'\n');print(len(records),'packages',version)
