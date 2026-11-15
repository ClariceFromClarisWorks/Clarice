#!/usr/bin/env python3
"""Prepare an explicit private HTTP update source. Does not publish or install."""
import argparse, binascii
from pathlib import Path
p=argparse.ArgumentParser()
p.add_argument('application',type=Path)
p.add_argument('--build',type=int,required=True)
p.add_argument('--version',required=True)
p.add_argument('--directory',type=Path,required=True)
a=p.parse_args()
if not 0<a.build<=0xffffffff or not 0<len(a.version)<64 or any(not 33<=ord(c)<=126 for c in a.version):p.error('Invalid build/version')
b=a.application.read_bytes()
if len(b)<128 or b[65:73]!=b'APPLCLRC' or binascii.crc_hqx(b[:124],0)!=int.from_bytes(b[124:126],'big'):p.error('Invalid application MacBinary header')
name=f'Clarice-{a.build}.bin'
manifest=(f'CLARICE-UPDATE 2\nbuild={a.build}\nversion={a.version}\nsystem=1536\ncpu=68000\npackage={name}\nbytes={len(b)}\ncrc32={binascii.crc32(b):08x}\n').encode('ascii')
a.directory.mkdir(parents=True,exist_ok=True)
# Refuse accidental reuse of a published build identifier or source index.
for path,data in ((a.directory/name,b),(a.directory/'update.txt',manifest)):
 with path.open('xb') as f:f.write(data)
print((a.directory/'update.txt').resolve())
