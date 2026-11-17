"""Actual 68000 output kernels and fixed-width decimal field versus host oracles.
Link decimal.o / kernels.S.obj at 0x10000; extract .raw and nm to work/output-perf.
Uses the existing isolated work/date-test-env Unicorn environment.
"""
from pathlib import Path
import random,re,struct
from unicorn import Uc,UC_ARCH_M68K,UC_MODE_BIG_ENDIAN,UC_HOOK_MEM_READ
from unicorn.m68k_const import *
root=Path('work/output-perf')
symbols={name:int(address,16) for address,name in re.findall(r'^([0-9a-f]+) T (\w+)$',(root/'symbols.txt').read_text(),re.M)}
rng=random.Random(68000)
regs=[UC_M68K_REG_D3,UC_M68K_REG_D4,UC_M68K_REG_D5,UC_M68K_REG_D6,UC_M68K_REG_D7,UC_M68K_REG_A2,UC_M68K_REG_A3,UC_M68K_REG_A4,UC_M68K_REG_A5,UC_M68K_REG_A6]
def machine(image):
 u=Uc(UC_ARCH_M68K,UC_MODE_BIG_ENDIAN);u.ctl_set_cpu_model(UC_CPU_M68K_M68000)
 u.mem_map(0x10000,0x40000);u.mem_write(0x10000,(root/image).read_bytes());return u
def call(u,name,*args):
 u.reg_write(UC_M68K_REG_SR,0x2000);u.reg_write(UC_M68K_REG_A7,0x40000)
 for r in regs:u.reg_write(r,0x12345678)
 u.mem_write(0x40000,struct.pack('>'+'I'*(len(args)+1),0x48000,*args))
 u.emu_start(symbols[name],0x48000,count=10000)
 assert u.reg_read(UC_M68K_REG_A7)==0x40004
 assert all(u.reg_read(r)==0x12345678 for r in regs)
 return u.reg_read(UC_M68K_REG_D0)
u=machine('decimal.raw')
values=list(range(65537))+[2**32-1,2**31,2**31-1]+[rng.randrange(2**32) for _ in range(10000)]
for v in values:
 u.mem_write(0x30000,b'!'*12);call(u,'format_decimal_field',0x30001,v)
 assert bytes(u.mem_read(0x30000,12))==b'!'+f'{v:10d}'.encode()+b'!',v
print(f'PASS: {len(values)} actual M68000 decimal fields, exact padding/digits, odd destination, guards and ABI')
u=machine('kernels.raw');bounds=[0,0]
def read_guard(uc,access,address,size,value,data):
 if 0x20000<=address<0x21000:assert bounds[0]<=address and address+size<=bounds[1],(address,size,bounds)
u.hook_add(UC_HOOK_MEM_READ,read_guard)
cases=0
for n in range(133):
 for position in range(n+1):
  data=bytearray(rng.randrange(32,256) for _ in range(n))
  if position<n:data[position]=rng.randrange(32)
  src=0x20001+(cases&1);dst=0x30001+((cases>>1)&1)
  if data:u.mem_write(src,bytes(data))
  bounds[:]=[src,src+n];u.mem_write(0x30000,b'!'*136)
  got=call(u,'asm_copy_printable',dst,src,n)
  assert got==position and bytes(u.mem_read(0x30000,136))==b'!'*(dst-0x30000)+bytes(data[:position])+b'!'*(136-(dst-0x30000)-position)
  cases+=1
for offset in range(0,16,2):
 u.mem_write(0x30000,b'!'*164);call(u,'asm_clear_terminal_row',0x30002+offset)
 assert bytes(u.mem_read(0x30000,164))==b'!'*(2+offset)+b'\0'*132+b'!'*(30-offset)
print(f'PASS: {cases} bounded printable-copy cases, all lengths/control positions, high bytes, odd addresses, no overread; 8 row-clear alignments, guards and ABI')
