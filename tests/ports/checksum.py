from pathlib import Path
import random,struct
from unicorn import Uc,UC_ARCH_M68K,UC_MODE_BIG_ENDIAN
from unicorn.m68k_const import *
u=Uc(UC_ARCH_M68K,UC_MODE_BIG_ENDIAN);u.ctl_set_cpu_model(UC_CPU_M68K_M68000);u.mem_map(0x10000,0x50000)
u.mem_write(0x10000,Path('work/checksum/checksum.raw').read_bytes());rng=random.Random(68000)
regs=[UC_M68K_REG_D2,UC_M68K_REG_D3,UC_M68K_REG_D4,UC_M68K_REG_D5,UC_M68K_REG_D6,UC_M68K_REG_D7,UC_M68K_REG_A2,UC_M68K_REG_A3,UC_M68K_REG_A4,UC_M68K_REG_A5,UC_M68K_REG_A6]
for case,n in enumerate(list(range(1025))+[65535,65536,65537]+[rng.randrange(4096) for _ in range(1000)]):
 data=rng.randbytes(n);seed=rng.randrange(2**32);expected=seed
 for c in data:expected=((expected^c)*16777619)&0xffffffff
 u.mem_write(0x20000,b'!'+data+b'!');u.mem_write(0x50000,struct.pack('>IIII',0x58000,seed,0x20001,n))
 u.reg_write(UC_M68K_REG_SR,0x2000);u.reg_write(UC_M68K_REG_A7,0x50000)
 for r in regs:u.reg_write(r,0x12345678)
 u.emu_start(0x10000,0x58000,count=2000000)
 assert u.reg_read(UC_M68K_REG_D0)==expected,(case,n)
 assert u.reg_read(UC_M68K_REG_A7)==0x50004
 assert all(u.reg_read(r)==0x12345678 for r in regs)
 assert bytes(u.mem_read(0x20000,n+2))==b'!'+data+b'!'
print(f'PASS: {case+1} actual 68000 checksum cases; odd addresses, empty/large buffers, guards, registers and stack')
