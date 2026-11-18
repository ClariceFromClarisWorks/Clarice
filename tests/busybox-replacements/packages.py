"""Verify sealed offline/HTTP replacement artifacts and compatibility payloads."""
from pathlib import Path
import sys,json,hashlib,struct
root=Path(__file__).resolve().parents[2];sys.path.insert(0,str(root/'scripts'));import release,repository
sealed=release.verified(root);commands={r['name']:r for r in sealed['commands']};rows=[]
assert len(commands)==50
for n in ['wc','tr','sort']:
 for name,image in [(n,'bb'+n),(n+'-sbase',n)]:
  r=commands[name];assert r['image']==image
  data=(root/'work/build'/f'{image}.clx').read_bytes();kit=(root/'work/distribution/kit/Utilities'/name).read_bytes();assert data==kit
  assert hashlib.sha256(data).hexdigest()==r['sha256']
  rows.append(dict(name=name,image=image,sha256=r['sha256'],package_bytes=len(data),image_bytes=struct.unpack_from('>I',data,8)[0]))
 assert commands[n]['sha256']!=commands[n+'-sbase']['sha256']
assert (root/'work/distribution/kit/Licenses/busybox/LICENSE').exists()
index=root/'work/replacement-snow/http/index.txt'
if index.exists():
 records=[x.split('\t') for x in index.read_text().splitlines()[1:]]
 assert sum(x[0]=='core' for x in records)==1
 for item in rows:
  record=next(x for x in records if x[:2]==['tool',item['name']])
  payload=(index.parent/record[5]).read_bytes()
  assert hashlib.sha256(payload).hexdigest()==item['sha256']
  assert record[2:4]==[str(sealed['build']),str(item['package_bytes'])]

(root/'outputs/busybox-replacements/packages.json').write_text(json.dumps(rows,indent=2)+'\n')
print('PASS 50-command sealed kit; six replacement/compatibility payloads and BusyBox licence')
