"""Run the actual linked 68000 date formatter against Python datetime.
Build created.elf/raw at 0x10000 from created.o; use its nm entry address.
Requires unicorn in an isolated host test environment.
"""
import datetime,random,struct,sys
from pathlib import Path
from unicorn import Uc, UC_ARCH_M68K, UC_MODE_BIG_ENDIAN
from unicorn.m68k_const import UC_M68K_REG_A7, UC_M68K_REG_SR, UC_CPU_M68K_M68000
entry=int(sys.argv[1],16)
u=Uc(UC_ARCH_M68K,UC_MODE_BIG_ENDIAN);u.ctl_set_cpu_model(UC_CPU_M68K_M68000);u.mem_map(0x10000,0x40000)
u.mem_write(0x10000,Path('work/modules/created.raw').read_bytes())
epoch=datetime.datetime(1904,1,1);rng=random.Random(68000)
values=[0,1,59,60,86399,86400,2**32-1]+[rng.randrange(2**32) for _ in range(10000)]+list(range(0,2**32,86400))
for seconds in values:
 u.reg_write(UC_M68K_REG_SR,0x2000);u.reg_write(UC_M68K_REG_A7,0x40000)
 u.mem_write(0x40000,struct.pack('>III',0x48000,0x30000,seconds))
 u.emu_start(entry,0x48000,count=10000)
 actual=bytes(u.mem_read(0x30000,16)).decode()
 expected=(epoch+datetime.timedelta(seconds=seconds)).strftime('%Y-%m-%d %H:%M')
 assert actual==expected,(seconds,actual,expected)
print(f'PASS: actual 68000 formatter, {len(values)} dates match Python datetime')
