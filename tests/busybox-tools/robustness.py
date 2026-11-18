"""Real guest empty streams, heap-pressure recovery and locked-file failures."""
from pathlib import Path
import sys,json,time,re
r=Path(__file__).resolve().parents[2];sys.path.insert(0,str(r/'tests/xvi'));from client import XviClient
c=XviClient(port=23243,password=Path(sys.argv[1]).read_text().strip());rows=[];out=r/'outputs/busybox-tools/robustness.json'
def cmd(s,expected=0,timeout=240):
 rc,t=c.command(s,timeout);rows.append(dict(command=s,status=rc,text=t));out.write_text(json.dumps(rows,indent=2)+'\n')
 if expected is not None:assert rc==expected,(s,rc,t)
 return rc,t
try:
 cmd("printf '' > /tmp/bbt-empty")
 for name,args,rc in [('wc','',0),('tr','a-z A-Z',0),('cmp','/tmp/bbt-empty',0),('sort','',0),('sed','p',0),('grep','alpha',1),('cat','',0),('cut','-c 1',0),('head','',0),('tail','',0),('uniq','',0),('tee','',0),('cksum','',0)]:
  cmd('bb'+name+' '+args+' < /tmp/bbt-empty > /tmp/bbt-out',rc)
  _,s=cmd('status runtime');assert 'remaining=0' in s
 cmd('bbtail -c 100 < /tmp/bbt-binary.dat > /tmp/bbt-candidate');cmd('tail -c 100 < /tmp/bbt-binary.dat > /tmp/bbt-reference');cmd('cmp /tmp/bbt-candidate /tmp/bbt-reference')
 _,t=cmd("printf 'alpha\\nbeta\\n' | bbgrep alpha | bbwc -l");assert '\r\n1\r\n' in t,t
 cmd('cp /tmp/bbt-small.txt /tmp/bbt-locked');cmd('hfsattr --locked on /tmp/bbt-locked')
 rc,_=cmd('bbtee /tmp/bbt-locked < /tmp/bbt-ordinary.txt > /tmp/bbt-out',None);assert rc!=0
 cmd('cmp /tmp/bbt-locked /tmp/bbt-small.txt');cmd('hfsattr --locked off /tmp/bbt-locked')
 cmd('wget -O /tmp/bbt-pressure http://192.168.1.112:8097/sort-pressure.txt')
 for name in ['bbsort','sort']:
  rc,t=cmd('time '+name+' /tmp/bbt-pressure > /tmp/bbt-pressure-out; pressure_status=$?; status runtime; printf "PRESSURE_STATUS=%s\\n" "$pressure_status"',timeout=600)
  assert 'remaining=0' in t,t
  print(name,re.findall(r'real [0-9.]+ s|PRESSURE_STATUS=[0-9]+',t),flush=True)
  if name=='sort':assert 'PRESSURE_STATUS=0\r\n' in t,t
 cmd('sort -c /tmp/bbt-pressure-out')
 _,t=cmd('wc -c < /tmp/bbt-pressure-out');assert '\r\n262144\r\n' in t,t
 _,t=cmd('bbwc -c < /tmp/bbt-small.txt');assert '\r\n512\r\n' in t,t
 _,t=cmd('status runtime');assert 'remaining=0' in t,t
finally:c.close()
print('PASS',len(rows),'robustness observations')
