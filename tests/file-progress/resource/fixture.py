from pathlib import Path
import json,hashlib
w=Path('work/file-progress/resource');w.mkdir(parents=True,exist_ok=True)
items=[('STR ',128,'Greeting',0,bytes([len(b'Clarice resource-fork validation')])+b'Clarice resource-fork validation'),('BLOB',129,'Boundary payload',0,bytes(range(256))*257),('TEST',-123,'Binary sentinel',0x20,bytes([0,255,128,13,10,0,42]))]
# Generate genuine resource format with upstream Retro68 Rez, not a custom map.
r=[];expected=[]
for typ,rid,name,attr,data in items:
 r.append(f'data \'{typ}\' ({rid}, "{name}"'+(', purgeable' if attr else '')+') {\n'+ '\n'.join('$"'+data[i:i+32].hex()+'"' for i in range(0,len(data),32))+'\n};')
 h=2166136261
 for x in data:h=((h^x)*16777619)&0xffffffff
 expected.append(dict(type=typ,id=rid,name=name,attributes=attr,length=len(data),fnv=h,sha256=hashlib.sha256(data).hexdigest()))
(w/'fixture.r').write_text('\n'.join(r));(w/'data').write_bytes(b'Clarice valid resource file\r'+bytes(range(256))*33)
(w/'expected.json').write_text(json.dumps(expected,indent=2)+'\n')
(w/'expected.h').write_text('\n'.join(' {0x'+x['type'].encode().hex()+f"UL,{x['id']},{x['attributes']},{x['length']}L,{x['fnv']}UL,\"{x['name']}\"}}," for x in expected))
