"""Real Telnet edit/save/error checks; screen oracle before input."""
import sys,json,time,pyte
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'xvi'))
from client import XviClient
c=XviClient(port=23241,password=Path(sys.argv[1]).read_text().strip());rows=[]
def cmd(source,expected=0,contains=None):
 rc,text=c.command(source,90);rows.append(dict(command=source,status=rc,text=text));assert rc==expected,(source,rc,text)
 if contains is not None:assert contains in text,(contains,text)
 return text
def edit(source,keys):
 screen=pyte.Screen(80,24);stream=pyte.Stream(screen);c.t.write(source.encode()+b'\r\n');start=time.monotonic();raw=b''
 while time.monotonic()-start<60:
  data=c.t.read_very_eager();raw+=data;stream.feed(data.decode('latin1'))
  if screen.cursor.y==0 and screen.cursor.x==0 and (screen.display[22].startswith('~') or screen.display[22].startswith(f'{22:063d}')):break
  time.sleep(.01)
 else:raise TimeoutError(repr(raw))

 for part in __import__('re').split(b'(\x1b(?!\[))',keys):
  if part:
   c.t.write(part);raw+=c.drain(quiet=.4,timeout=10)
  if part==b'\x1b':time.sleep(.15)

 if not raw.endswith(b' > '):raw+=c.t.read_until(b' > ',60)
 assert raw.endswith(b' > '),repr(raw)
 rows.append(dict(command=source,transcript=raw.decode('latin1')));print(source,flush=True);cmd('status runtime',contains='remaining=0')
try:
 cmd('bbvi --version',contains='BusyBox 1.37.0');cmd('bbvi --help',contains='Usage:');cmd('bbvi --bad-option',2,contains='unsupported option')
 cmd("printf 'alpha beta\\nsecond line\\n' > /tmp/bbvi-edit")
 edit('bbvi /tmp/bbvi-edit',b'/beta\rcwBETA\x1b:wq\r');cmd('cat /tmp/bbvi-edit',contains='alpha BETA')
 edit('bbvi /tmp/bbvi-edit',b':%s/line/ROW/\r:wq\r');cmd('cat /tmp/bbvi-edit',contains='second ROW')
 edit('bbvi /tmp/bbvi-edit',b'ix\x1bu:wq\r');cmd('cat /tmp/bbvi-edit',contains='\r\nalpha BETA\r\nsecond ROW\r\n')
 edit('bbvi /tmp/bbvi-edit',b'\x1b[BiARROW \x1b:wq\r');cmd('cat /tmp/bbvi-edit',contains='ARROW second ROW')
 edit('bbvi -R /tmp/bbvi-edit',b'iBAD\x1b:w\r:q!\r');assert 'BAD' not in cmd('cat /tmp/bbvi-edit')
 cmd('hfsattr --type TEXT --creator BB01 /tmp/bbvi-edit')
 edit('bbvi /tmp/bbvi-edit',b'A retained\x1b:wq\r');cmd('hfsattr /tmp/bbvi-edit',contains='BB01')
 cmd('hfsattr --locked on /tmp/bbvi-edit')
 edit('bbvi /tmp/bbvi-edit',b'A LOST\x1b:w!\r:q!\r');assert 'LOST' not in cmd('cat /tmp/bbvi-edit')
 cmd('hfsattr --locked off /tmp/bbvi-edit')
 cmd('bbvi --help > /tmp/bbvi-help');cmd('cat /tmp/bbvi-help',contains='Usage:')
 cmd('bbvi --bad-option > /tmp/bbvi-out 2> /tmp/bbvi-err',2);cmd('test ! -s /tmp/bbvi-out');cmd('cat /tmp/bbvi-err',contains='unsupported option')
 cmd('bbvi < /tmp/bbvi-edit',2,contains='terminal required')
 edit('bbvi /tmp/bbvi-edit',b':!echo no\r:q!\r')
 cmd('cp /tmp/vi-8k /tmp/bbvi-reference');edit('bbvi /tmp/vi-8k',b':wq\r');cmd('cmp /tmp/vi-8k /tmp/bbvi-reference')
 cmd('test ! -f /etc/clarice/update.incomplete');cmd('cat /etc/clarice/installed.catalog',contains='bbvi');cmd('status runtime',contains='remaining=0')
finally:
 c.close();Path('outputs/busybox/regression.json').write_text(json.dumps(rows,indent=2)+'\n')
print('PASS',len(rows),'observations')
