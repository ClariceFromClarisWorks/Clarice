"""Experimental pre-shifted planes on M68000: dirty pixels, memory guards."""
from pathlib import Path
import random,re,struct,subprocess
from unicorn import Uc,UC_ARCH_M68K,UC_MODE_BIG_ENDIAN,UC_HOOK_MEM_READ,UC_HOOK_MEM_WRITE,UC_MEM_WRITE
from unicorn.m68k_const import *
r=Path('work/planes');r.mkdir(parents=True,exist_ok=True)
s=Path('src/native-glyphs.inc').read_text();part=s[s.index('extern void asm_ascii_groups_at'):s.index('static int native_glyph_row')]
fill=Path('src/glyph-planes.inc').read_text();fill=fill[fill.index('static void glyph_planes_fill'):fill.index('static int glyph_planes_prepare')]
part=part.replace('asm_ascii_groups_at(p,(const unsigned char*)s,glyphs,full,bits)','asm_ascii_planes_at(p,(const unsigned char*)s,planes,full)')
(r/'test.c').write_text('#include <stdint.h>\n#define STRIDE 64\nstatic unsigned char *prompt_pixels,*planes;\nextern void asm_ascii_planes_at(unsigned char*,const unsigned char*,const unsigned char*,long);\nextern void asm_ascii_cells(unsigned char *,const unsigned char *,const unsigned char *,long,long,long);\n'+fill+part+'\nvoid test(unsigned char*p,const char*s,int n,int phase,unsigned char*q){prompt_pixels=p;planes=q;glyph_planes_fill(q,p+704,phase&7);native_glyph_compose_at(s,n,phase);}\n')
image='ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c'
subprocess.run(['docker','run','--rm','--pull','never','--platform','linux/amd64','--network','none','-v',str(Path.cwd())+':/src',image,'sh','-ec','m68k-apple-macos-gcc -m68000 -Os -c /src/work/planes/test.c -o /src/work/planes/test.o; m68k-apple-macos-gcc -m68000 -c /src/src/kernels.S -o /src/work/planes/kernels.o; m68k-apple-macos-gcc -m68000 -c /src/src/glyph-planes.S -o /src/work/planes/planes.o; m68k-apple-macos-ld -Ttext=0x10000 -e test /src/work/planes/test.o /src/work/planes/kernels.o /src/work/planes/planes.o -o /src/work/planes/test.elf; m68k-apple-macos-objcopy -O binary /src/work/planes/test.elf /src/work/planes/test.raw; m68k-apple-macos-nm /src/work/planes/test.elf > /src/work/planes/symbols.txt'],check=True)
u=Uc(UC_ARCH_M68K,UC_MODE_BIG_ENDIAN);u.ctl_set_cpu_model(UC_CPU_M68K_M68000);u.mem_map(0x10000,0x50000);u.mem_write(0x10000,(r/'test.raw').read_bytes())
entry=int(next(x.split()[0] for x in (r/'symbols.txt').read_text().splitlines() if x.endswith(' T test')),16)
bounds=[]
def guard(uc,access,address,size,value,data):
 assert any(a<=address and address+size<=b for a,b in bounds),(access,address,size)
 if access==UC_MEM_WRITE and not 0x3f000<=address<0x40020:assert (0x20010<=address and address+size<=0x20010+704) or (0x50010<=address and address+size<=0x50010+24320),'write outside row/table storage'
 if size>1:assert address%2==0,'odd word/long access'
u.hook_add(UC_HOOK_MEM_READ|UC_HOOK_MEM_WRITE,guard,begin=0x20000,end=0x56000)
rng=random.Random(68000);cases=0
for phase in range(16):
 for count in range(1,86):
  if phase+count*6>512:continue
  old=bytearray(rng.randrange(256) for _ in range(4128));chars=bytes(32+(i*13+count*17)%95 for i in range(count))
  glyph=bytes(rng.randrange(64)<<2 for _ in range(95*32));old[16+704:16+704+len(glyph)]=glyph
  u.mem_write(0x50000,bytes([0xa5])*24352);u.mem_write(0x20000,bytes(old));u.mem_write(0x30001,chars)
  bounds[:]=[(0x20010+y*64,0x20010+(y+1)*64) for y in range(11)]+[(0x20010+704,0x20010+704+len(glyph)),(0x30001,0x30001+count),(0x50010,0x50010+24320),(0x3f000,0x40020)]
  u.reg_write(UC_M68K_REG_SR,0x2000);u.reg_write(UC_M68K_REG_A7,0x40000);u.mem_write(0x40000,struct.pack('>6I',0x48000,0x20010,0x30001,count,phase,0x50010))
  u.emu_start(entry,0x48000,count=1000000)
  assert bytes(u.mem_read(0x50000,16))==bytes([0xa5])*16 and bytes(u.mem_read(0x50010+24320,16))==bytes([0xa5])*16,'table guards'
  got=bytes(u.mem_read(0x20000,4128));assert got[:16]==old[:16] and got[720:]==old[720:],'buffer/glyph guards'
  for y in range(11):
   for x in range(count*6):
    pos=16+y*64+(phase+x)//8
    assert ((got[pos]>>(7-(phase+x)%8))&1)==((glyph[(chars[x//6]-32)*32+y]>>(7-x%6))&1),(phase,count,y,x)
  cases+=1
print(f'PASS: {cases} actual M68000 pre-shifted-plane cases; every phase/tail, dirty storage, byte-identical glyph/outer guards, bounded and even multibyte accesses')
