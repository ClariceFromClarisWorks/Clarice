#!/usr/bin/env python3
"""Create clean application media using an existing Apple Driver43 device header.
Only the descriptor, partition map and driver partition are retained. No HFS
blocks or unallocated data from the template are copied. Populate via hfsutils.
"""
import argparse,struct,hashlib,json
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('template',type=Path);p.add_argument('output',type=Path);a=p.parse_args()
with a.template.open('rb') as f:
    dd=f.read(512);assert dd[:2]==b'ER' and struct.unpack_from('>H',dd,2)[0]==512
    size=struct.unpack_from('>I',dd,4)[0]*512
    f.seek(512);first=f.read(512);count=struct.unpack_from('>I',first,4)[0];assert count==3
    f.seek(512);entries=[f.read(512) for _ in range(count)]
    types=[e[48:80].split(b'\0')[0] for e in entries]
    assert types==[b'Apple_partition_map',b'Apple_Driver43',b'Apple_HFS']
    start,blocks=struct.unpack_from('>II',entries[1],8);assert start>=count+1 and (start+blocks)*512<=size
    f.seek(start*512);driver=f.read(blocks*512)
    hfs_start,hfs_blocks=struct.unpack_from('>II',entries[2],8);assert hfs_start>=start+blocks and (hfs_start+hfs_blocks)*512<=size
    e=bytearray(entries[2]);e[16:48]=b'Clarice'.ljust(32,b'\0');entries[2]=bytes(e)
a.output.parent.mkdir(parents=True,exist_ok=True)
with a.output.open('wb') as f:
    f.truncate(size);f.write(dd)
    for i,e in enumerate(entries,1):f.seek(i*512);f.write(e)
    f.seek(start*512);f.write(driver)
print(json.dumps({'bytes':size,'driver_type':'Apple_Driver43','driver_sha256':hashlib.sha256(driver).hexdigest(),'hfs_start':hfs_start,'hfs_blocks':hfs_blocks,'template_hfs_bytes_copied':0},indent=2))
