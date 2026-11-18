"""Compare hfsutils MacBinary exports from this task's stopped disposable disks."""
from pathlib import Path
import argparse,struct,hashlib,json
p=argparse.ArgumentParser();p.add_argument('source',type=Path);p.add_argument('saved',type=Path);p.add_argument('output',type=Path);a=p.parse_args()
def read(path):
 b=path.read_bytes();d,r=struct.unpack_from('>II',b,83);at=128+((d+127)//128)*128
 assert len(b)>=at+r
 return dict(data=b[128:128+d],resource=b[at:at+r],type=b[65:69].decode(),creator=b[69:73].decode(),created=struct.unpack_from('>I',b,91)[0])
s=read(a.source);t=read(a.saved)
assert s['resource'] and len(s['resource'])==len(t['resource'])
# The documented resource-fork header has system-owned bytes 16..127.
# Retain raw differences; never call a normalized match byte identity.
differences=[i for i,(x,y) in enumerate(zip(s['resource'],t['resource'])) if x!=y]
assert all(16<=i<128 for i in differences),differences
assert s['resource'][:16]==t['resource'][:16]
assert s['resource'][128:]==t['resource'][128:]
assert t['data']==b'Fork preservation\n'
for k in ['type','creator','created']:assert s[k]==t[k],k
r=dict(resource_bytes=len(t['resource']),resource_sha256=hashlib.sha256(t['resource']).hexdigest(),resource_byte_identical=not differences,system_reserved_differing_offsets=differences,resource_application_area_data_map_identical=True,source_resource_sha256=hashlib.sha256(s['resource']).hexdigest(),data_bytes=len(t['data']),data_sha256=hashlib.sha256(t['data']).hexdigest(),type=t['type'],creator=t['creator'],creation_date_preserved=True,scope='Stopped isolated Snow native HFS readback; original and edited copy; original copy-time flags difference documented in guest snapshots')
a.output.write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r,indent=2))
