"""Execute production in-place alignment on M68000; compare every buffer byte."""
from pathlib import Path
import random, re, struct, subprocess
from unicorn import Uc, UC_ARCH_M68K, UC_MODE_BIG_ENDIAN, UC_HOOK_MEM_READ, UC_HOOK_MEM_WRITE, UC_HOOK_CODE
from unicorn.m68k_const import *
r=Path('work/align-perf');r.mkdir(parents=True,exist_ok=True)
r.joinpath('align.c').write_text('#define STRIDE 64\ntypedef struct {short top,left,bottom,right;} Rect;\nstatic unsigned char *prompt_pixels;\n'+Path('src/native-align.inc').read_text()+'\nint test_align(unsigned char *p,Rect*r,int phase){prompt_pixels=p;return align_native_row(r,phase);}\n')
image='ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c'
subprocess.run(['docker','run','--rm','--pull','never','--platform','linux/amd64','--network','none','-v',str(Path.cwd())+':/src',image,'sh','-ec','m68k-apple-macos-gcc -m68000 -Os -c /src/work/align-perf/align.c -o /src/work/align-perf/align.o; m68k-apple-macos-gcc -m68000 -c /src/src/kernels.S -o /src/work/align-perf/kernels.o; m68k-apple-macos-ld -Ttext=0x10000 -e test_align /src/work/align-perf/align.o /src/work/align-perf/kernels.o -o /src/work/align-perf/align.elf; m68k-apple-macos-objcopy -O binary /src/work/align-perf/align.elf /src/work/align-perf/align.raw; m68k-apple-macos-objdump -d /src/work/align-perf/align.elf > /src/work/align-perf/align.dis'],check=True)
u=Uc(UC_ARCH_M68K,UC_MODE_BIG_ENDIAN);u.ctl_set_cpu_model(UC_CPU_M68K_M68000);u.mem_map(0x10000,0x50000);u.mem_write(0x10000,(r/'align.raw').read_bytes())
bounds=[]
def guard(uc,access,address,size,value,data):
 assert any(a<=address and address+size<=b for a,b in bounds),(access,address,size,bounds)
u.hook_add(UC_HOOK_MEM_READ|UC_HOOK_MEM_WRITE,guard,begin=0x20000,end=0x2101f)
saved=[UC_M68K_REG_D2,UC_M68K_REG_D3,UC_M68K_REG_D4,UC_M68K_REG_D5,UC_M68K_REG_D6,UC_M68K_REG_D7,UC_M68K_REG_A2,UC_M68K_REG_A3,UC_M68K_REG_A4,UC_M68K_REG_A5,UC_M68K_REG_A6]
# Check the assembly boundary itself. Retro68's compiled C wrapper uses D2 as
# scratch, so comparing it across the wrapper would test the wrong convention.
dis=(r/'align.dis').read_text()
entry=int(re.search(r'^([0-9a-f]+) <asm_align_row_bits>:',dis,re.M)[1],16)
returned=int(re.search(r'jsr [^\n]*<asm_align_row_bits>\n\s*([0-9a-f]+):',dis)[1],16)
entry_registers=[]
def enter(uc,address,size,data):entry_registers[:]=[uc.reg_read(reg) for reg in saved]
def leave(uc,address,size,data):
 assert [uc.reg_read(reg) for reg in saved]==entry_registers,'assembly callee-saved registers'
u.hook_add(UC_HOOK_CODE,enter,begin=entry,end=entry)
u.hook_add(UC_HOOK_CODE,leave,begin=returned,end=returned)
rng=random.Random(68000);cases=[]
for phase in range(16):
 for left in list(range(32))+[128,255,480,496,505,511]:
  for width in [1,2,6,12,18,24,30,51,64,80,464,480,510,512-left]:
   if left+width<=512:cases.append((left,left+width,0,11,phase))
 for count in range(1,86):cases.append((0,count*6,0,11,phase))
 for top,bottom in [(0,1),(1,3),(10,11),(11,12),(63,64)]:cases.append((3,481,top,bottom,phase))
cases += [(-1,20,0,11,7),(0,0,0,11,7),(2,1,0,11,7),(0,20,-1,11,7),(0,20,0,65,7),(0,20,11,11,7)]
def bit(b,y,x):return (b[16+y*64+x//8]>>(7-x%8))&1
for left,right,top,bottom,phase in cases:
 old=bytes(rng.randrange(256) for _ in range(4128));expected=bytearray(old)
 shift=(phase-left)&15
 valid=shift and left>=0 and right>left and right+shift<=512 and top>=0 and bottom<=64 and bottom>top
 if valid:
  first=left//16;last=(right+shift-1)//16
  for y in range(top,bottom):
   # Independent bit-level expected shift, including whole boundary words.
   for x in range(first*16,(last+1)*16):
    pos=16+y*64+x//8;mask=128>>(x%8)
    value=bit(old,y,x-shift) if x-shift>=first*16 else 0
    expected[pos]=(expected[pos]&~mask)|(mask if value else 0)
 bounds[:]=[(0x20010+y*64+first*2,0x20010+y*64+(last+1)*2) for y in range(top,bottom)] if valid else []
 for i,reg in enumerate(saved):u.reg_write(reg,0x12340000+i)
 u.mem_write(0x20000,old);u.mem_write(0x30000,struct.pack('>4h',top,left,bottom,right))
 u.reg_write(UC_M68K_REG_SR,0x2000);u.reg_write(UC_M68K_REG_A7,0x40000);u.mem_write(0x40000,struct.pack('>4I',0x48000,0x20010,0x30000,phase))
 u.emu_start(0x10000,0x48000,count=1000000)
 assert all(u.reg_read(reg)==0x12340000+i for i,reg in enumerate(saved) if reg!=UC_M68K_REG_D2),"C wrapper callee-saved registers"
 assert bytes(u.mem_read(0x20000,4128))==expected,(left,right,top,bottom,phase)
 actual=struct.unpack('>4h',u.mem_read(0x30000,8));assert actual==(top,left+(shift if valid else 0),bottom,right+(shift if valid else 0)),actual
 assert u.reg_read(UC_M68K_REG_D0)==bool(valid)
 if valid:
  for y in range(top,bottom):
   for x in range(left,right):assert bit(expected,y,x+shift)==bit(old,y,x)
print(f'PASS: {len(cases)} actual M68000 alignment cases; offsets 0..15, all cell tails, dirty buffers, overlap, bounds and fallback; full-buffer read/write guards and ABI registers')
