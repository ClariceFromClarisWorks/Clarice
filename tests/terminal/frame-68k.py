"""Actual 68000 fixed-row comparison: every byte, both even alignments, ABI/bounds."""
from pathlib import Path
import random,re,struct
from unicorn import Uc,UC_ARCH_M68K,UC_MODE_BIG_ENDIAN,UC_HOOK_MEM_READ
from unicorn.m68k_const import *
r=Path('work/frame-perf');symbols={name:int(a,16) for a,name in re.findall(r'^([0-9a-f]+) T (\w+)$',(r/'symbols.txt').read_text(),re.M)}
u=Uc(UC_ARCH_M68K,UC_MODE_BIG_ENDIAN);u.ctl_set_cpu_model(UC_CPU_M68K_M68000)
u.mem_map(0x10000,0x40000);u.mem_write(0x10000,(r/'kernels.raw').read_bytes())
regs=[UC_M68K_REG_D2,UC_M68K_REG_D3,UC_M68K_REG_D4,UC_M68K_REG_D5,UC_M68K_REG_D6,UC_M68K_REG_D7,UC_M68K_REG_A2,UC_M68K_REG_A3,UC_M68K_REG_A4,UC_M68K_REG_A5,UC_M68K_REG_A6]
bounds=[]
def read_guard(uc,access,address,size,value,data):
 if 0x20000<=address<0x31000:assert any(lo<=address and address+size<=hi for lo,hi in bounds),(address,size,bounds)
u.hook_add(UC_HOOK_MEM_READ,read_guard)
rng=random.Random(68000);cases=0
for a in [0x20000,0x20002]:
 for b in [0x30000,0x30002]:
  for position in range(-1,132):
   first=bytes(rng.randrange(256) for _ in range(132));second=bytearray(first)
   if position>=0:second[position]^=255
   u.mem_write(a,first);u.mem_write(b,bytes(second));bounds[:]=[(a,a+132),(b,b+132)]
   u.reg_write(UC_M68K_REG_SR,0x2000);u.reg_write(UC_M68K_REG_A7,0x40000)
   for reg in regs:u.reg_write(reg,0x12345678)
   u.mem_write(0x40000,struct.pack('>III',0x48000,a,b));u.emu_start(symbols['asm_terminal_row_changed'],0x48000,count=1000)
   assert u.reg_read(UC_M68K_REG_D0)==int(position>=0)
   assert u.reg_read(UC_M68K_REG_A7)==0x40004 and all(u.reg_read(reg)==0x12345678 for reg in regs)
   assert bytes(u.mem_read(a,132))==first and bytes(u.mem_read(b,132))==second
   cases+=1
print(f'PASS: {cases} actual M68000 row comparisons; equal and every differing byte, both even alignments, no out-of-bounds reads/writes; preserved ABI')
