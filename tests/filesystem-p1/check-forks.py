"""After guest exit/FlushVol and offline hcopy exports. No live image access."""
from pathlib import Path
import hashlib,json
p=Path('work/filesystem-p1')
def read(path):
 b=path.read_bytes();n=int.from_bytes(b[83:87],'big');q=int.from_bytes(b[87:91],'big');o=128+((n+127)//128)*128
 return b[128:128+n],b[o:o+q],(b[65:73],b[73]&~1,b[101],b[91:99])
source=read(p/'native-source.bin');rows={}
r=source[1];d,m,dn,mn=[int.from_bytes(r[i:i+4],'big') for i in (0,4,8,12)]
assert d>=256 and m>=256 and d+dn<=len(r) and m+mn<=len(r) and mn>=28
for name in ['source','copy','move','sentinel','cancel']:
 data,resource,meta=read(p/('native-'+name+'.bin'));assert data==source[0] and meta==source[2]
 changed=[i for i,(a,b) in enumerate(zip(r,resource)) if a!=b];assert len(resource)==len(r)
 assert all(16<=i<128 for i in changed),changed
 # Rejected removal/cancelled replacement must leave the original completely intact.
 if name in ['source','sentinel','cancel']:assert not changed
 rows[name]=dict(data_size=len(data),resource_size=len(resource),data_sha256=hashlib.sha256(data).hexdigest(),protected_resource_sha256=hashlib.sha256(resource[:16]+resource[128:]).hexdigest(),changed_system_reserved_offsets=changed,metadata_equal=True)
assert 'Clv tmp' not in (p/'native-readback.txt').read_text()
for name,path in [('Install','work/install-ux/build/Clarice.bin'),('Recover','work/update-helper/ClariceUpdater.bin')]:
 assert read(p/'offline-readback'/(name+'.bin'))==read(Path(path))
Path('outputs/filesystem-p1/forks.json').write_text(json.dumps(rows,indent=2)+'\n')
print('PASS native resource header/payload/map, data, Finder fields/dates; sentinel/cancel forks byte-identical; only copied/moved system scratch differs. Offline installer/helper forks verified.')
