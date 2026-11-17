"""Inspect the actual MacBinary forks/resource map without loading resources."""
from pathlib import Path
import struct,json,re,sys,hashlib
root=Path(__file__).resolve().parents[2];p=Path(sys.argv[1]) if len(sys.argv)>1 else root/'work/build/Clarice.bin'
b=p.read_bytes();u16=lambda s,o:struct.unpack_from('>H',s,o)[0];u32=lambda s,o:struct.unpack_from('>I',s,o)[0]
df=u32(b,83);rf=u32(b,87);off=128+((df+127)//128)*128;r=b[off:off+rf];data,mp,dl,ml=struct.unpack_from('>4I',r);types=mp+u16(r,mp+24);rows=[]
for i in range(u16(r,types)+1):
 at=types+2+8*i;typ=r[at:at+4].decode('mac_roman');count=u16(r,at+4)+1;refs=types+u16(r,at+6)
 for j in range(count):
  q=refs+j*12;ident=struct.unpack_from('>h',r,q)[0];loc=data+int.from_bytes(r[q+5:q+8],'big');size=u32(r,loc);rows.append(dict(type=typ,id=ident,bytes=size,attributes=r[q+4],sha256=hashlib.sha256(r[loc+4:loc+4+size]).hexdigest()))
legacy=dict((name,int(ident)) for name,ident in re.findall(r'\("([^"\\]+)",(\d+)\)',(root/'scripts/command-resource.py').read_text()))
commands=[(name,legacy[image]) for name,image in re.findall(r'CLARICE_COMMAND\("([^"\\]+)", "([^"\\]+)"\)',(root/'src/standard-commands.inc').read_text())]
sizes={x['id']:x['bytes'] for x in rows if x['type']=='CLX1'}
for x in rows:
 if x['type']=='CLX1':x['commands']=[n for n,i in commands if int(i)==x['id']]
clx=sum(sizes.values());code=sum(x['bytes'] for x in rows if x['type']=='CODE');report=dict(package_bytes=len(b),data_fork=df,resource_fork=rf,resource_payload=sum(x['bytes'] for x in rows),resource_map_bytes=ml,clx_payload=clx,code_payload=code,other_payload=sum(x['bytes'] for x in rows if x['type'] not in ['CLX1','CODE']),installed_command_data=sum(sizes.get(int(i),0) for _,i in commands),resources=rows)
print(json.dumps(report,indent=2))
