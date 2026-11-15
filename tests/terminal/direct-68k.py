"""Production direct composition on M68000: dirty pixels, memory guards."""
from pathlib import Path
import random,re,struct,subprocess
from unicorn import Uc,UC_ARCH_M68K,UC_MODE_BIG_ENDIAN,UC_HOOK_MEM_READ,UC_HOOK_MEM_WRITE,UC_MEM_WRITE
from unicorn.m68k_const import *
r=Path('work/direct-perf');r.mkdir(parents=True,exist_ok=True)
s=Path('src/native-glyphs.inc').read_text();part=s[s.index('extern void asm_ascii_groups_at'):s.index('static int native_glyph_row')]
(r/'test.c').write_text('#define STRIDE 64\nstatic unsigned char *prompt_pixels;\nextern void asm_ascii_cells(unsigned char *,const unsigned char *,const unsigned char *,long,long,long);\n'+part+'\nvoid test(unsigned char*p,const char*s,int n,int phase){prompt_pixels=p;native_glyph_compose_at(s,n,phase);}\n')
image='ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c'
subprocess.run(['docker','run','--rm','--pull','never','--platform','linux/amd64','--network','none','-v',str(Path.cwd())+':/src',image,'sh','-ec','m68k-apple-macos-gcc -m68000 -Os -c /src/work/direct-perf/test.c -o /src/work/direct-perf/test.o; m68k-apple-macos-gcc -m68000 -c /src/src/kernels.S -o /src/work/direct-perf/kernels.o; m68k-apple-macos-ld -Ttext=0x10000 -e test /src/work/direct-perf/test.o /src/work/direct-perf/kernels.o -o /src/work/direct-perf/test.elf; m68k-apple-macos-objcopy -O binary /src/work/direct-perf/test.elf /src/work/direct-perf/test.raw'],check=True)
u=Uc(UC_ARCH_M68K,UC_MODE_BIG_ENDIAN);u.ctl_set_cpu_model(UC_CPU_M68K_M68000);u.mem_map(0x10000,0x50000);u.mem_write(0x10000,(r/'test.raw').read_bytes())
bounds=[]
def guard(uc,access,address,size,value,data):
 assert any(a<=address and address+size<=b for a,b in bounds),(access,address,size)
 if access==UC_MEM_WRITE:assert 0x20010<=address and address+size<=0x20010+704,'write outside row staging area'
 if size>1:assert address%2==0,'odd word/long access'
u.hook_add(UC_HOOK_MEM_READ|UC_HOOK_MEM_WRITE,guard,begin=0x20000,end=0x31000)
rng=random.Random(68000);cases=0
for phase in range(16):
 for count in range(1,86):
  if phase+count*6>512:continue
  old=bytearray(rng.randrange(256) for _ in range(4128));chars=bytes(32+(i*13+count*17)%95 for i in range(count))
  glyph=bytes(rng.randrange(64)<<2 for _ in range(95*32));old[16+704:16+704+len(glyph)]=glyph
  u.mem_write(0x20000,bytes(old));u.mem_write(0x30001,chars)
  bounds[:]=[(0x20010+y*64,0x20010+(y+1)*64) for y in range(11)]+[(0x20010+704,0x20010+704+len(glyph)),(0x30001,0x30001+count)]
  u.reg_write(UC_M68K_REG_SR,0x2000);u.reg_write(UC_M68K_REG_A7,0x40000);u.mem_write(0x40000,struct.pack('>5I',0x48000,0x20010,0x30001,count,phase))
  u.emu_start(0x10000,0x48000,count=1000000)
  got=bytes(u.mem_read(0x20000,4128));assert got[:16]==old[:16] and got[720:]==old[720:],'buffer/glyph guards'
  for y in range(11):
   for x in range(count*6):
    pos=16+y*64+(phase+x)//8
    assert ((got[pos]>>(7-(phase+x)%8))&1)==((glyph[(chars[x//6]-32)*32+y]>>(7-x%6))&1),(phase,count,y,x)
  cases+=1
print(f'PASS: {cases} actual M68000 direct-composition cases; every phase/tail, dirty storage, byte-identical glyph/outer guards, bounded and even multibyte accesses')
