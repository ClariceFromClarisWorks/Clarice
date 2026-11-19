"""Build disposable mounted-source failure fixtures using a real sealed command."""
from pathlib import Path
import zlib
r=Path(__file__).resolve().parents[2];d=r/'work/share-snow/http';d.mkdir(parents=True,exist_ok=True)
b=(r/'work/build/free.clx').read_bytes();(d/'probe.clx').write_bytes(b)
def row(name,version,filename='probe.clx',crc=None):
 return f'tool\t{name}\t{version}\t{len(b)}\t{zlib.crc32(b) if crc is None else crc:08x}\t{filename}\n'
core=(d/'repo/index.txt').read_text().splitlines()[1]+'\n'
for name,body in [('partial',row('sharetwo',1,'missing.clx')+row('shareone',1)),('resume',row('shareone',1)+row('sharetwo',1)),('corrupt',row('shareone',2,crc=0))]:
 (d/(name+'.txt')).write_text('CLARICE-REPOSITORY 1\n'+core+body)
