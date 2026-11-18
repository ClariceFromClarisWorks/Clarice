"""Actual M68000 correctness and dynamic instruction counts, NOT cycle timings.
Uses the existing Unicorn environment. No Macintosh/HFS execution is simulated.
"""
from pathlib import Path
import json,random,struct
from unicorn import Uc,UC_ARCH_M68K,UC_MODE_BIG_ENDIAN,UC_HOOK_CODE
from unicorn.m68k_const import *
r=Path('work/ls-asm');out=Path('outputs/ls-asm');rng=random.Random(68000)
regs=[UC_M68K_REG_D3,UC_M68K_REG_D4,UC_M68K_REG_D5,UC_M68K_REG_D6,UC_M68K_REG_D7,UC_M68K_REG_A2,UC_M68K_REG_A3,UC_M68K_REG_A4,UC_M68K_REG_A5,UC_M68K_REG_A6]
values=list(range(65537))+[10**n+d for n in range(1,10) for d in (-1,0,1)]+[0x7fffffff,0x80000000,0xffffffff]+[rng.randrange(2**32) for _ in range(10000)]
rows={};machines={}
for name in ['baseline','candidate','specialized']:
 u=Uc(UC_ARCH_M68K,UC_MODE_BIG_ENDIAN);u.ctl_set_cpu_model(UC_CPU_M68K_M68000);u.mem_map(0x10000,0x40000);u.mem_write(0x10000,(r/(name+'.raw')).read_bytes());machines[name]=u
 def run(value,count=False):
  u.reg_write(UC_M68K_REG_SR,0x2000);u.reg_write(UC_M68K_REG_A7,0x40000)
  for reg in regs:u.reg_write(reg,0x12345678)
  u.mem_write(0x40000,struct.pack('>III',0x48000,0x30001,value));u.mem_write(0x30000,b'!'*12)
  stats={'instructions':0,'divu':0}
  def trace(uc,pc,size,data):
   stats['instructions']+=1
   if int.from_bytes(uc.mem_read(pc,2),'big')&0xf1c0==0x80c0:stats['divu']+=1
  hook=u.hook_add(UC_HOOK_CODE,trace) if count else None
  # Invalidate blocks translated before tracing was enabled.
  if count:u.ctl_remove_cache(0x10000,0x11000)
  u.emu_start(0x10000,0x48000,count=10000)
  if hook:
   u.hook_del(hook)
   assert stats['instructions'] > 0, "instruction tracing did not run"
  assert bytes(u.mem_read(0x30000,12))==b'!'+f'{value:10d}'.encode()+b'!',(name,value)
  assert u.reg_read(UC_M68K_REG_A7)==0x40004
  assert all(u.reg_read(reg)==0x12345678 for reg in regs)
  return stats
 for v in values:run(v)
 samples=[0,1,9,10,100,1024,8192,65535,65536,99999,100000,1048576,9999999,0x7fffffff,0xffffffff]
 rows[name]={'tested':len(values),'linked_bytes':(r/(name+'.raw')).stat().st_size,'samples':{str(v):run(v,True) for v in samples}}
 print(name,'PASS',len(values),'exact outputs, odd destination, guards, preserved registers and stack')
(out/'cpu-results.json').write_text(json.dumps(rows,indent=2)+'\n')
print('value baseline_instructions candidate_instructions specialized_instructions baseline_DIVU candidate_DIVU specialized_DIVU')
for v in samples:
 a=rows['baseline']['samples'][str(v)];b=rows['candidate']['samples'][str(v)]
 c=rows['specialized']['samples'][str(v)]
 print(v,a['instructions'],b['instructions'],c['instructions'],a['divu'],b['divu'],c['divu'])
