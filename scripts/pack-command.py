#!/usr/bin/env python3
"""Pack a big-endian m68000 ELF relocatable object into the CLX1 command ABI."""
from pathlib import Path
import struct,sys
u=lambda b,o:struct.unpack_from('>I',b,o)[0]
def pack(source):
 b=Path(source).read_bytes()
 if b[:7]!=b'\x7fELF\x01\x02\x01' or struct.unpack_from('>HH',b,16)!=(1,4):raise ValueError('requires ELF32 big-endian m68k ET_REL')
 if u(b,36)!=0x1000000:raise ValueError('requires baseline m68000 object')
 shoff=u(b,32);shsize,n,strings=struct.unpack_from('>HHH',b,46)
 if shsize!=40 or shoff+n*40>len(b):raise ValueError('invalid sections')
 sections=[struct.unpack_from('>10I',b,shoff+i*40) for i in range(n)]
 image=bytearray();offsets={}
 for i,s in enumerate(sections):
  if not s[2]&2:continue
  if s[1] not in (1,8):raise ValueError('unsupported allocatable section')
  align=max(s[8],4);image.extend(b'\0'*((-len(image))%align));offsets[i]=len(image)
  data=b'\0'*s[5] if s[1]==8 else b[s[4]:s[4]+s[5]]
  if len(data)!=s[5]:raise ValueError('truncated section')
  image.extend(data)
 image.extend(b'\0'*((-len(image))%4));imports=[];relocs=[];entry=None
 for s in sections:
  if s[1]!=2:continue
  strings_section=sections[s[6]];names=b[strings_section[4]:strings_section[4]+strings_section[5]]
  symbols=[struct.unpack_from('>IIIBBH',b,o) for o in range(s[4],s[4]+s[5],16)]
  for sym in symbols:
   name=names[sym[0]:].split(b'\0',1)[0]
   if name==b'clarice_main' and sym[5] in offsets:entry=offsets[sym[5]]+sym[1]
 for r in sections:
  if r[1] not in (4,9) or r[7] not in offsets:continue
  if r[1]!=4:raise ValueError('REL without explicit addend unsupported')
  sy=sections[r[6]];st=sections[sy[6]];names=b[st[4]:st[4]+st[5]]
  for pos in range(r[4],r[4]+r[5],12):
   place,info,addend=struct.unpack_from('>IIi',b,pos);typ=info&255;idx=info>>8
   sym=struct.unpack_from('>IIIBBH',b,sy[4]+idx*16);name=names[sym[0]:].split(b'\0',1)[0]
   off=offsets[r[7]]+place
   if typ not in (1,4,5,6):raise ValueError(f'unsupported relocation {typ}: {name!r}')
   size={1:4,4:4,5:2,6:1}[typ]
   if off+size>len(image):raise ValueError('relocation outside image')
   if sym[5]==0:
    if typ!=1 or not name or len(name)>63:raise ValueError('unsupported external relocation')
    if name not in imports:imports.append(name)
    struct.pack_into('>I',image,off,addend&0xffffffff);relocs.append((off,2,imports.index(name)))
   else:
    if sym[5] in offsets:value=offsets[sym[5]]+sym[1]+addend
    elif sym[5]==0xfff1:value=sym[1]+addend
    else:raise ValueError('unresolved/common symbol')
    if typ==1:
     struct.pack_into('>I',image,off,value&0xffffffff)
     if sym[5]!=0xfff1:relocs.append((off,1,value))
    else:
     value-=off
     if not -(1<<(size*8-1))<=value<(1<<(size*8-1)):raise ValueError('PC relative relocation overflow')
     image[off:off+size]=value.to_bytes(size,'big',signed=True)
 if entry is None or entry&1:raise ValueError('missing/alignment error in clarice_main')
 relocs.sort();last=-4
 for off,kind,target in relocs:
  if off&1 or off<last+4:raise ValueError('overlapping/odd relocation')
  last=off
 payload=bytes(image)+b''.join(struct.pack('>III',*r) for r in relocs)+b''.join(s.ljust(64,b'\0') for s in imports)
 checksum=2166136261
 for c in payload:checksum=((checksum^c)*16777619)&0xffffffff
 header=struct.pack('>4s7I',b'CLX1',1,len(image),entry,len(relocs),len(imports),checksum,0)
 return header+payload,imports
if __name__=='__main__':
 data,imports=pack(sys.argv[1]);Path(sys.argv[2]).write_bytes(data)
 print(f'{sys.argv[2]}: {len(data)} bytes, imports '+', '.join(s.decode() for s in imports))
