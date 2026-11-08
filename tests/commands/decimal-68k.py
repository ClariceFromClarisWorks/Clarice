"""Compare compiled baseline 68000 decimal routine to Python integers.
Link decimal.o at 0x10000 and extract work/ls-opt2/decimal.raw first.
"""
import random,struct
from pathlib import Path
from unicorn import Uc,UC_ARCH_M68K,UC_MODE_BIG_ENDIAN
from unicorn.m68k_const import *
u=Uc(UC_ARCH_M68K,UC_MODE_BIG_ENDIAN);u.ctl_set_cpu_model(UC_CPU_M68K_M68000);u.mem_map(0x10000,0x40000)
u.mem_write(0x10000,Path('work/ls-opt2/decimal.raw').read_bytes())
rng=random.Random(68000)
values=list(range(65537))+[2**32-1,2**31,2**31-1]+[rng.randrange(2**32) for _ in range(10000)]

# Retro68 Macintosh ABI has caller-clobbered D0-D2/A0-A1.
regs=[UC_M68K_REG_D3,UC_M68K_REG_D4,UC_M68K_REG_D5,UC_M68K_REG_D6,UC_M68K_REG_D7,UC_M68K_REG_A2,UC_M68K_REG_A3,UC_M68K_REG_A4,UC_M68K_REG_A5,UC_M68K_REG_A6]
for value in values:
 u.reg_write(UC_M68K_REG_SR,0x2000);u.reg_write(UC_M68K_REG_A7,0x40000)
 for r in regs:u.reg_write(r,0x12345678)
 u.mem_write(0x40000,struct.pack('>III',0x48000,0x30001,value));u.mem_write(0x30000,b'!'*13)
 u.emu_start(0x10000,0x48000,count=10000)
 result=bytes(u.mem_read(0x30000,13));expected=str(value).encode()+b'\0'
 assert result==b'!'+expected+b'!'*(12-len(expected)),(value,result)
 assert u.reg_read(UC_M68K_REG_A7)==0x40004
 assert all(u.reg_read(r)==0x12345678 for r in regs)
print(f'PASS: {len(values)} actual M68000 decimal conversions; exact bytes, odd destination, guards, preserved registers and stack')
