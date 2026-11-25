"""Matched normal-build Snow acceptance over the existing loopback Telnet bridge."""
import sys,json,time
from pathlib import Path
sys.path.insert(0,'tests/remote-sessions');from client import Client
label=sys.argv[1];c=Client(port=23259,password='');rows=[]
try:
 rc,out=c.command('cat /tmp/load-result',90);rows.append(dict(test='loader-pressure-and-timing',status=rc,output=out));print(out,flush=True)
 if label=='after':
  for n in [8192,32768]:
   for run in range(2 if n==8192 else 1):
    rc,out=c.command('time sh /mnt/Sys6Unix/Perf/'+str(n)+'.sh; result=$?; status runtime; test \"$result\" = 0',900)
    assert rc==0 and 'remaining=0' in out;rows.append(dict(test='script',bytes=n,run=run,status=rc,output=out));print(out,flush=True)
  for source,expected in [("f() { for x in \"$@\"; do printf '<%s>' \"$x\"; done; }; f '' 'a b'; echo",'<><a b>'),("printf 'one\\ntwo\\n' | while read x; do echo \"$x\"; done > /tmp/perf-flow; cat /tmp/perf-flow",'two'),("echo $((2+3*4)); case 'a*' in a\\*) echo literal;; esac",'literal')]:
   rc,out=c.command(source,60);assert rc==0 and expected in out;rows.append(dict(test='shell',output=out))
  c.t.write(b'while :; do :; done\r\n');time.sleep(1);c.t.write(b'\x03');reply=c.t.read_until(b' > ',30)
  rc,out=c.command('echo cancel=$?; echo recovered; status runtime',60)
  assert 'cancel=130' in out and 'recovered' in out and 'remaining=0' in out;rows.append(dict(test='cancel-recovery',output=reply.decode('latin1')+out))
finally:c.close()
Path('outputs/userland-perf/guest-'+label+'.json').write_text(json.dumps(rows,indent=2)+'\n')
print('PASS',label,flush=True)
