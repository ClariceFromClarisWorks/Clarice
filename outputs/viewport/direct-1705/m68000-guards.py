"""Exact 1705 pointer compositor, separate glyph/stage storage, actual M68000."""
from pathlib import Path
import random,struct,subprocess
from unicorn import Uc,UC_ARCH_M68K,UC_MODE_BIG_ENDIAN,UC_HOOK_MEM_READ,UC_HOOK_MEM_WRITE,UC_MEM_WRITE
from unicorn.m68k_const import *
r=Path('work/direct-stage-test');src=Path('work/direct-terminal/src')
s=(src/'native-glyphs.inc').read_text();part=s[s.index('extern void asm_ascii_groups_at'):s.index('static int native_glyph_row_to')]
f=(src/'glyph-planes.inc').read_text();fill=f[f.index('static void glyph_planes_fill'):f.index('static int glyph_planes_prepare')]
head='''#include <stdint.h>
#define STRIDE 64
#define TEXT_LEFT 4
static unsigned char *prompt_pixels,*glyph_planes_data;
static int planes_enabled,requested;
static struct{struct{struct{int left;}bounds;}portBits;}window,*win=&window;
static int glyph_planes_begin(int phase){(void)phase;return 0;}
static int glyph_planes_prepare(const unsigned char*g,int phase){(void)g;return phase==requested;}
extern void asm_ascii_planes_unrolled(unsigned char*,const unsigned char*,const unsigned char*,long);
extern void asm_ascii_planes_at(unsigned char*,const unsigned char*,const unsigned char*,long);
extern void asm_ascii_cells(unsigned char*,const unsigned char*,const unsigned char*,long,long,long);
'''
wrapper='''
void test(unsigned char*out,unsigned char*font,const char*s,int n,int phase,unsigned char*table,int variant){
 prompt_pixels=font;glyph_planes_data=table;requested=phase&7;planes_enabled=variant;
 glyph_planes_fill(table,font+704,requested);native_glyph_compose_at(out,s,n,phase);
}
'''
(r/'test.c').write_text(head+fill+part+wrapper)
image='ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c'
cmd='''m68k-apple-macos-gcc -m68000 -Os -c /src/work/direct-stage-test/test.c -o /src/work/direct-stage-test/test.o
for s in kernels glyph-planes planes-unrolled; do m68k-apple-macos-gcc -m68000 -c /src/work/direct-terminal/src/$s.S -o /src/work/direct-stage-test/$s.o; done
m68k-apple-macos-ld -Ttext=0x10000 -e test /src/work/direct-stage-test/test.o /src/work/direct-stage-test/kernels.o /src/work/direct-stage-test/glyph-planes.o /src/work/direct-stage-test/planes-unrolled.o -o /src/work/direct-stage-test/test.elf
m68k-apple-macos-objcopy -O binary /src/work/direct-stage-test/test.elf /src/work/direct-stage-test/test.raw
m68k-apple-macos-nm /src/work/direct-stage-test/test.elf > /src/work/direct-stage-test/symbols.txt'''
subprocess.run(['docker','run','--rm','--pull','never','--platform','linux/amd64','--network','none','-v',str(Path.cwd())+':/src',image,'sh','-ec',cmd],check=True)
u=Uc(UC_ARCH_M68K,UC_MODE_BIG_ENDIAN);u.ctl_set_cpu_model(UC_CPU_M68K_M68000);u.mem_map(0x10000,0x50000);u.mem_write(0x10000,(r/'test.raw').read_bytes())
entry=int(next(x.split()[0] for x in (r/'symbols.txt').read_text().splitlines() if x.endswith(' T test')),16)
bounds=[]
def guard(uc,access,address,size,value,data):
 assert any(a<=address and address+size<=b for a,b in bounds),(access,hex(address),size)
 if access==UC_MEM_WRITE and not 0x3f000<=address<0x40040:
  assert (0x20010<=address and address+size<=0x20010+704) or (0x50010<=address and address+size<=0x50010+24320),'write outside stage/table'
 if size>1:assert address%2==0,'odd word/long access'
u.hook_add(UC_HOOK_MEM_READ|UC_HOOK_MEM_WRITE,guard,begin=0x20000,end=0x56000)
rng=random.Random(68000);cases=0;plane_cases=0
regs=[UC_M68K_REG_D3,UC_M68K_REG_D4,UC_M68K_REG_D5,UC_M68K_REG_D6,UC_M68K_REG_D7,UC_M68K_REG_A2,UC_M68K_REG_A3,UC_M68K_REG_A4,UC_M68K_REG_A5,UC_M68K_REG_A6]
for offset in (0,2,14):
 for phase in range(16):
  for count in range(1,86):
   if offset*8+phase+count*6>512:continue
   # Every phase/tail on the selected unrolled engine; sampled full rows also
   # exercise the unchanged no-table fallback through this exact interface.
   for variant in ((0,2) if count in (1,4,80) else (2,)):
    old=bytes(rng.randrange(256) for _ in range(736));font=bytearray(rng.randrange(256) for _ in range(4128))
    glyph=bytes(rng.randrange(64)<<2 for _ in range(95*32));font[16+704:16+704+len(glyph)]=glyph
    chars=bytes(32+(i*13+count*17)%95 for i in range(count))
    u.mem_write(0x20000,old);u.mem_write(0x22000,bytes(font));u.mem_write(0x30001,chars);u.mem_write(0x50000,bytes([0xa5])*24352)
    bounds[:]=[(0x20010+y*64,0x20010+(y+1)*64) for y in range(11)]+[(0x22010+704,0x22010+704+len(glyph)),(0x30001,0x30001+count),(0x50010,0x50010+24320),(0x3f000,0x40040)]
    u.reg_write(UC_M68K_REG_SR,0x2000);u.reg_write(UC_M68K_REG_A7,0x40000)
    saved={reg:0x17170000+i*0x101 for i,reg in enumerate(regs)}
    for reg,v in saved.items():u.reg_write(reg,v)
    u.mem_write(0x40000,struct.pack('>8I',0x48000,0x20010+offset,0x22010,0x30001,count,phase,0x50010,variant))
    u.emu_start(entry,0x48000,count=1000000)
    for reg,v in saved.items():assert u.reg_read(reg)==v,('callee-saved register',reg,hex(v),hex(u.reg_read(reg)),offset,phase,count,variant)
    got=bytes(u.mem_read(0x20000,736));assert got[:16]==old[:16] and got[720:]==old[720:],'stage guards'
    assert bytes(u.mem_read(0x22000,4128))==font,'glyph/source buffer changed'
    assert bytes(u.mem_read(0x50000,16))==bytes([0xa5])*16 and bytes(u.mem_read(0x50010+24320,16))==bytes([0xa5])*16,'table guards'
    for y in range(11):
     for x in range(count*6):
      bit=offset*8+phase+x;pos=16+y*64+bit//8
      assert ((got[pos]>>(7-bit%8))&1)==((glyph[(chars[x//6]-32)*32+y]>>(7-x%6))&1),(offset,phase,count,y,x,variant)
    cases+=1;plane_cases+=variant==2
print(f'PASS {cases} actual M68000 separated-output cases ({plane_cases} selected-unrolled mode); phase0..15, tails1..85, row offsets0/2/14, dirty stage, read-only glyph source, per-scanline bounds, even multibyte accesses and callee-saved registers')
