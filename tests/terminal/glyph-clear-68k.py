"""Production C tail clearing plus actual M68000 glyph compositor."""
from pathlib import Path
import subprocess
root=Path.cwd();out=root/'work/clear-perf';out.mkdir(parents=True,exist_ok=True)
s=(root/'src/native-glyphs.inc').read_text();clear='static void native_glyph_clear'+s.split('static void native_glyph_clear',1)[1].split('static int native_glyph_row',1)[0]
(out/'wrapper.c').write_text('#define STRIDE 64\nstatic unsigned char *prompt_pixels;\n'+clear+'\nextern void asm_ascii_cells(unsigned char *,const unsigned char *,const unsigned char *,long,long,long);\nvoid test_native_row(unsigned char *p,const unsigned char *s,const unsigned char *g,long n,long h,long w){prompt_pixels=p;native_glyph_clear(n);asm_ascii_cells(p,s,g,n,h,w); }\n')
subprocess.run(['docker','run','--rm','--pull','never','--platform','linux/amd64','--network','none','-v',str(root)+':/src','ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c','sh','-ec',"m68k-apple-macos-gcc -m68000 -Os -c /src/work/clear-perf/wrapper.c -o /src/work/clear-perf/wrapper.o; m68k-apple-macos-gcc -m68000 -c /src/src/kernels.S -o /src/work/clear-perf/kernels.o; m68k-apple-macos-ld -Ttext=0x10000 -e test_native_row /src/work/clear-perf/wrapper.o /src/work/clear-perf/kernels.o -o /src/work/clear-perf/kernels.elf; m68k-apple-macos-objcopy -O binary /src/work/clear-perf/kernels.elf /src/work/clear-perf/kernels.raw; m68k-apple-macos-nm /src/work/clear-perf/kernels.elf > /src/work/clear-perf/symbols.txt"],check=True)
"""Real 68000 shared cell compositor: original and ASCII-based entry points."""
from pathlib import Path
import random,re,struct
from unicorn import Uc,UC_ARCH_M68K,UC_MODE_BIG_ENDIAN,UC_HOOK_MEM_READ
from unicorn.m68k_const import *
r=Path('work/clear-perf');symbols={name:int(a,16) for a,name in re.findall(r'^([0-9a-f]+) T (\w+)$',(r/'symbols.txt').read_text(),re.M)}
u=Uc(UC_ARCH_M68K,UC_MODE_BIG_ENDIAN);u.ctl_set_cpu_model(UC_CPU_M68K_M68000);u.mem_map(0x10000,0x60000);u.mem_write(0x10000,(r/'kernels.raw').read_bytes())
regs=[UC_M68K_REG_D2,UC_M68K_REG_D3,UC_M68K_REG_D4,UC_M68K_REG_D5,UC_M68K_REG_D6,UC_M68K_REG_D7,UC_M68K_REG_A2,UC_M68K_REG_A3,UC_M68K_REG_A4,UC_M68K_REG_A5,UC_M68K_REG_A6]
rng=random.Random(68000);bounds=[]
def guard(uc,access,address,size,value,data):
 if 0x20000<=address<0x26000:assert any(a<=address and address+size<=b for a,b in bounds),(address,size,bounds)
u.hook_add(UC_HOOK_MEM_READ,guard)

for count in range(1,86):
 glyph=bytes(rng.randrange(64)<<2 for _ in range(95*32));chars=bytes(32+(i*13+count*17)%95 for i in range(count))
 u.mem_write(0x21000,glyph);u.mem_write(0x20001,chars);bounds[:]=[(0x20001,0x20001+count),(0x21000,0x21000+len(glyph))]
 old=bytearray(rng.randrange(256) for _ in range(4096));expected=bytearray(old);u.mem_write(0x30000,bytes(old))
 for y in range(11):
  tail=count%4;offset=16+y*64+(count//4)*3
  for b in range((tail*6+7)//8):expected[offset+b]=0
  for c,ch in enumerate(chars):
   for x in range(6):
    bit=c*6+x;pos=16+y*64+bit//8;mask=128>>(bit&7)
    expected[pos]=(expected[pos]&~mask)|(mask if glyph[(ch-32)*32+y]&(128>>x) else 0)
 u.reg_write(UC_M68K_REG_SR,0x2000);u.reg_write(UC_M68K_REG_A7,0x40000)
 u.mem_write(0x40000,struct.pack('>7I',0x48000,0x30010,0x20001,0x21000,count,11,6))
 u.emu_start(symbols['test_native_row'],0x48000,count=1000000)
 assert bytes(u.mem_read(0x30000,4096))==expected,count
print('PASS: 85 actual M68000 C-clear/compositor cases with arbitrary previous bytes; all short/full/tail lengths and untouched guards')
