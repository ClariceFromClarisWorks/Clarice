"""Validate persisted forks; exclude only documented system-reserved bytes.
Native Resource Manager probe independently checks every resource's semantics.
"""
from pathlib import Path
import struct,hashlib,json,re
w=Path('work/file-progress/resource');out=Path('outputs/file-progress/resource')
def unpack(path):
 b=path.read_bytes();d,r=struct.unpack_from('>II',b,83);off=128+(d+127)//128*128
 assert len(b)>=off+r
 return b[128:128+d],b[off:off+r],b[65:73],b[91:99]
def parse(r):
 # Resource map format used by existing scripts/release.py, extended for names.
 rd,rm,dl,ml=struct.unpack_from('>4I',r);assert rd>=256 and rd+dl<=len(r) and rm+ml<=len(r)
 t=rm+struct.unpack_from('>H',r,rm+24)[0];names=rm+struct.unpack_from('>H',r,rm+26)[0]
 assert rm<=t<rm+ml and rm<=names<=rm+ml
 count=struct.unpack_from('>H',r,t)[0]+1;assert t+2+count*8<=rm+ml
 entries=[]
 for i in range(count):
  pos=t+2+i*8;typ=r[pos:pos+4].decode('ascii');n,refs=struct.unpack_from('>HH',r,pos+4);refs+=t;assert refs+(n+1)*12<=rm+ml
  for j in range(n+1):
   ref=refs+j*12;rid,name=struct.unpack_from('>hH',r,ref);a=r[ref+4];off=rd+int.from_bytes(r[ref+5:ref+8],'big');assert off+4<=rd+dl
   size=struct.unpack_from('>I',r,off)[0];assert off+4+size<=rd+dl;payload=r[off+4:off+4+size]
   assert name!=65535;name+=names;assert name<rm+ml and name+1+r[name]<=rm+ml
   entries.append(dict(type=typ,id=rid,name=r[name+1:name+1+r[name]].decode(),attributes=a,length=size,sha256=hashlib.sha256(payload).hexdigest()))
 return sorted(entries,key=lambda x:(x['type'],x['id']))
expected=json.loads((w/'expected.json').read_text())
for x in expected:x.pop('fnv')
expected.sort(key=lambda x:(x['type'],x['id']))
a=unpack(w/'valid.bin');assert parse(a[1])==expected
source=unpack(w/'source.bin');assert a[:3]==source[:3] # imported source remains untouched
def validate_copy(source,b):
 assert b[0]==source[0] and b[2:]==source[2:]
 assert len(b[1])==len(source[1]);assert parse(b[1])==expected
 diffs=[i for i,(x,y) in enumerate(zip(source[1],b[1])) if x!=y]
 assert all(16<=i<128 for i in diffs),diffs
 assert b[1][:16]==source[1][:16] and b[1][128:]==source[1][128:]
 return diffs
results={}
for name in ['before','after','back-before','back-after']:
 b=unpack(w/(name+'.bin'));diffs=validate_copy(source,b)
 results[name]=dict(data_sha256=hashlib.sha256(b[0]).hexdigest(),resource_sha256=hashlib.sha256(b[1]).hexdigest(),resource_differing_offsets=diffs,all_non_system_bytes_equal=True,resources=expected)
text=(out/'guest.txt').read_text();statuses=re.findall(r'^([^\n:]+):(\d+)$',text,re.M)
labels={'source','source-after'}|{v+'-'+step for v in ['before','after'] for step in ['create','read-create','replace','read-replace','rename','read-rename','move','read-move','return','read-return','flush']}
assert len(statuses)==24 and {label for label,_ in statuses}==labels and all(code=='0' for _,code in statuses),statuses
assert text.count('PASS native resource types/IDs/names/attributes/sizes/payloads')==12
# Ensure semantic/copy checks do not hide corruption outside the reserved area.
for off in [0,128,255,256,len(source[1])-1]:
 damaged=bytearray(source[1]);damaged[off]^=1
 try:validate_copy(source,(source[0],bytes(damaged),*source[2:]))
 except (AssertionError,struct.error,UnicodeDecodeError):pass
 else:raise AssertionError('corruption accepted at '+str(off))
(out/'validation.json').write_text(json.dumps(results,indent=2)+'\n')
print('PASS 24 guest statuses, 12 native Resource Manager reads, both baseline/candidate create/replace/rename/cross-volume move/return-copy; resource payloads, maps, application bytes, data forks and metadata preserved; only system-reserved bytes may differ; mutation checks passed')
