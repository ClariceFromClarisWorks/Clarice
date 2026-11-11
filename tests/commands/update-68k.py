"""Execute independently built CLX fixtures on Unicorn's 68000 CPU.
Not a Plus/System 6 test. cli_write is a host test stub, not Macintosh I/O.
"""
from pathlib import Path
import struct
from unicorn import Uc,UC_ARCH_M68K,UC_MODE_BIG_ENDIAN,UC_HOOK_CODE
from unicorn.m68k_const import UC_M68K_REG_A7,UC_M68K_REG_SR,UC_M68K_REG_D0,UC_CPU_M68K_M68000
for version in (1,2):
 b=Path(f'work/task2/probe{version}.clx').read_bytes()
 magic,fmt,size,entry,nr,ni,checksum,reserved=struct.unpack_from('>4s7I',b)
 offset=48 if fmt==2 else 32
 assert magic==b'CLX1' and not reserved
 for repeat in range(3):
  cpu=Uc(UC_ARCH_M68K,UC_MODE_BIG_ENDIAN);cpu.ctl_set_cpu_model(UC_CPU_M68K_M68000)
  cpu.mem_map(0x10000,0x80000);code=bytearray(b[offset:offset+size]);names=b[offset+size+nr*12:]
  for i in range(nr):
   at,kind,target=struct.unpack_from('>III',b,offset+size+12*i)
   if kind==1:value=0x10000+target
   else:
    assert names[target*64:(target+1)*64].split(b'\0')[0]==b'cli_write'
    value=0x30000+struct.unpack_from('>I',code,at)[0]
   struct.pack_into('>I',code,at,value)
  cpu.mem_write(0x10000,bytes(code));cpu.mem_write(0x30000,b'\x4e\x75')
  cpu.reg_write(UC_M68K_REG_SR,0x2000);cpu.reg_write(UC_M68K_REG_A7,0x70000)
  cpu.mem_write(0x70000,struct.pack('>III',0x80000,1,0))
  output=[]
  def write_hook(cpu,address,size,user):
   if address!=0x30000:return
   sp=cpu.reg_read(UC_M68K_REG_A7)
   fd,p,n=struct.unpack('>III',cpu.mem_read(sp+4,12));assert fd==1
   output.append(bytes(cpu.mem_read(p,n)));cpu.reg_write(UC_M68K_REG_D0,n)
  cpu.hook_add(UC_HOOK_CODE,write_hook)
  cpu.emu_start(0x10000+entry,0x80000,count=10000)
  assert cpu.reg_read(UC_M68K_REG_D0)==0
  assert b''.join(output)==f'independent command v{version}\n'.encode()
print('PASS: independent legacy/v2 commands executed three times each on a 68000 CPU model; output/status correct')
