"""Validate imports/ABI and stage optional test packages, without altering release catalog."""
from pathlib import Path
import importlib.util,re,struct,json,hashlib,zlib,shutil
r=Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location('pack',r/'scripts/pack-command.py');pack=importlib.util.module_from_spec(spec);spec.loader.exec_module(pack)
exports=set(re.findall(r'CLX_IMPORT\("([^"]+)"',(r/'src/clx-imports.inc').read_text()))
rows=[]
for name in ['diff','base64','xxd','patch','gzip','gunzip','tar']:
 data,imports=pack.pack(r/f'work/busybox-additions/bb{name}.o.gdb',(1,10))
 missing=set(x.decode() for x in imports)-exports
 assert not missing,(name,sorted(missing))
 image,entry,relocs,nimports=struct.unpack_from('>4I',data,8)
 assert image<=256*1024 and relocs<=8192 and nimports<=128
 assert data==(r/f'work/busybox-additions/bb{name}.clx').read_bytes()
 rows.append(dict(name=name,package_bytes=len(data),image_bytes=image,relocations=relocs,imports=nimports,sha256=hashlib.sha256(data).hexdigest(),crc32=f'{zlib.crc32(data):08x}'))
(r/'outputs/busybox-additions/packages.json').write_text(json.dumps(rows,indent=2)+'\n')
print(json.dumps(rows,indent=2))
