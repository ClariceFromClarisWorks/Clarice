"""Real 68000 shared cell compositor: original and ASCII-based entry points."""
from pathlib import Path
import random,re,struct
from unicorn import Uc,UC_ARCH_M68K,UC_MODE_BIG_ENDIAN,UC_HOOK_MEM_READ
from unicorn.m68k_const import *
r=Path('work/jump-perf');symbols={name:int(a,16) for a,name in re.findall(r'^([0-9a-f]+) T (\w+)$',(r/'symbols.txt').read_text(),re.M)}
u=Uc(UC_ARCH_M68K,UC_MODE_BIG_ENDIAN);u.ctl_set_cpu_model(UC_CPU_M68K_M68000);u.mem_map(0x10000,0x60000);u.mem_write(0x10000,(r/'kernels.raw').read_bytes())
regs=[UC_M68K_REG_D2,UC_M68K_REG_D3,UC_M68K_REG_D4,UC_M68K_REG_D5,UC_M68K_REG_D6,UC_M68K_REG_D7,UC_M68K_REG_A2,UC_M68K_REG_A3,UC_M68K_REG_A4,UC_M68K_REG_A5,UC_M68K_REG_A6]
rng=random.Random(68000);bounds=[]
def guard(uc,access,address,size,value,data):
 if 0x20000<=address<0x26000:assert any(a<=address and address+size<=b for a,b in bounds),(address,size,bounds)
u.hook_add(UC_HOOK_MEM_READ,guard)
cases=0
for name,bias,limit in [('asm_cells',0,256),('asm_ascii_cells',32,95)]:
 for width in range(1,9):
  for height in [1,11,32]:
   for count in (range(86) if width==6 and height==11 else sorted({0,1,7,min(85,512//width)})):
    glyph=bytearray(rng.randrange(256)&(255<<(8-width)&255) for _ in range(limit*32));chars=bytes(bias+(i*13+count*17)%limit for i in range(count))
    u.mem_write(0x21000,bytes(glyph));u.mem_write(0x20001,chars or b'!');bounds[:]=[(0x20001,0x20001+count),(0x21000,0x21000+len(glyph))]
    expected=bytearray(4096);expected[:16]=b'!'*16;expected[-16:]=b'!'*16;u.mem_write(0x30000,bytes(expected))
    for c,ch in enumerate(chars):
     for y in range(height):
      for x in range(width):
       if glyph[(ch-bias)*32+y]&(128>>x):
        bit=c*width+x;expected[16+y*64+bit//8]|=128>>(bit&7)
    u.reg_write(UC_M68K_REG_SR,0x2000);u.reg_write(UC_M68K_REG_A7,0x40000)
    for reg in regs:u.reg_write(reg,0x12345678)
    u.mem_write(0x40000,struct.pack('>7I',0x48000,0x30010,0x20001,0x21000,count,height,width));u.emu_start(symbols[name],0x48000,count=1000000)
    assert bytes(u.mem_read(0x30000,4096))==expected,(name,width,height,count)
    assert all(u.reg_read(reg)==0x12345678 for reg in regs) and u.reg_read(UC_M68K_REG_A7)==0x40004
    cases+=1
print(f'PASS: {cases} M68000 compositor cases, both entry points, widths 1..8, heights 1/11/32, ASCII/high-byte indexing, empty/full rows, guards and ABI')
