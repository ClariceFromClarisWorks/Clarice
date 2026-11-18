"""Add optional BusyBox vi to a private test repository using its existing format."""
from pathlib import Path
import argparse,hashlib,binascii,struct
p=argparse.ArgumentParser();p.add_argument('repository',type=Path);p.add_argument('image',type=Path);p.add_argument('version',type=int);a=p.parse_args()
data=a.image.read_bytes();assert data[:4]==b'CLX1' and struct.unpack_from('>I',data,4)[0]==2
assert struct.unpack_from('>II',data,32)==(1,9)
name='bbvi';digest=hashlib.sha256(data).hexdigest();filename=f'{name}-{digest}.clx'
line=f'tool\t{name}\t{a.version}\t{len(data)}\t{binascii.crc32(data):08x}\t{filename}'
index=a.repository/'index.txt';rows=index.read_text().splitlines();assert rows[0]=='CLARICE-REPOSITORY 1'
for row in rows[1:]:
 fields=row.split('\t')
 if fields[:2]==['tool',name] and fields[2]==str(a.version) and row!=line:raise ValueError('published version changed')
rows=[r for r in rows if not r.startswith('tool\tbbvi\t')]
target=a.repository/filename
if target.exists():assert target.read_bytes()==data
else:
 temporary=target.with_suffix('.partial');temporary.write_bytes(data);temporary.replace(target)
rows.append(line);temporary=index.with_suffix('.partial');temporary.write_text('\n'.join(rows)+'\n');temporary.replace(index)
print(line)
