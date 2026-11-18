"""Physical-SE command correctness, cancellation and settled memory evidence.
Password comes from the environment; never record authentication traffic.
Requires configured authenticated Telnet server. Run separately from local timing.
"""
import argparse,json,os,re,time
from pathlib import Path
from client import Client
p=argparse.ArgumentParser();p.add_argument('--output',type=Path,required=True)
a=p.parse_args();assert os.environ.get('CLARICE_TEST_PASSWORD')
rows=[]
def record(**kw):
 rows.append(kw);a.output.parent.mkdir(parents=True,exist_ok=True)
 a.output.write_text(json.dumps(rows,indent=2)+'\n')
start=time.monotonic();c=Client();record(test='authenticated login',host_seconds=time.monotonic()-start)
try:
 for command,expected in [('pwd','/'),("printf 'alpha\\nbeta\\n' | grep beta",'beta'),('command -v nano','nano')]:
  start=time.monotonic();status,text=c.command(command,120)
  record(command=command,status=status,text=text,host_seconds=time.monotonic()-start)
  assert status==0 and expected in text
 c.t.write(b'while true; do :; done\r\n');time.sleep(1)
 start=time.monotonic();c.t.write(b'\x03');text=c.t.read_until(b' > ',30)
 assert text.endswith(b' > ')
 status,text=c.command('echo $?')
 record(test='Ctrl+C recovery',text=text,host_seconds=time.monotonic()-start)
 assert re.search(r'\r?\n130\r?\n',text)
 for i in range(10):
  status,text=c.command("printf 'alpha\\nbeta\\n' | grep beta; status runtime",120)
  match=re.search(r'heap=(\d+) largest=(\d+)',text)
  record(test='repeated pipeline cleanup',sample=i,status=status,text=text,
         heap_largest=[int(x) for x in match.groups()] if match else None)
  assert status==0 and match and 'remaining=0' in text
finally:c.close()
for i in range(3):
 start=time.monotonic();c=Client()
 try:
  status,text=c.command('pwd; status runtime',60)
  record(test='reconnect',sample=i,status=status,text=text,host_seconds=time.monotonic()-start)
  assert status==0 and 'remaining=0' in text
 finally:c.close()
print('PASS physical remote command checks; inspect recorded memory trend separately')
