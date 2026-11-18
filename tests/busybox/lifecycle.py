"""Fatal disconnect must release the invocation; no unsaved-recovery claim."""
import sys,time,json,pyte
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'xvi'))
from client import XviClient
password=Path(sys.argv[1]).read_text().strip();rows=[]
c=XviClient(port=23241,password=password)
screen=pyte.Screen(80,24);stream=pyte.Stream(screen);data=b''
def seen(predicate):
 global data
 start=time.monotonic()
 while time.monotonic()-start<30:
  chunk=c.t.read_very_eager()
  if chunk:data+=chunk;stream.feed(chunk.decode('latin1'))
  if predicate():return
  time.sleep(.01)
 raise TimeoutError('\n'.join(screen.display))
c.t.write(b'bbvi /tmp/vi-8k\r\n')
seen(lambda:screen.display[22].startswith(f'{22:063d}') and screen.cursor.x==0 and screen.cursor.y==0)
c.t.write(b'iUNSAVED');seen(lambda:screen.display[0].startswith('UNSAVED') and screen.cursor.x==7)
c.close();rows.append(dict(action='disconnect after visible unsaved insertion',first_screen_row=screen.display[0],transcript=data.decode('latin1')))

for attempt in range(1,5):
 time.sleep(1)
 try:c=XviClient(port=23241,password=password);rows.append(dict(reconnect_attempt=attempt,result='authenticated'));break
 except (EOFError,RuntimeError,TimeoutError,OSError) as e:rows.append(dict(reconnect_attempt=attempt,error=type(e).__name__+': '+str(e)))
else:
 Path('outputs/busybox/lifecycle.json').write_text(json.dumps(rows,indent=2)+'\n');raise RuntimeError('no reconnect after disconnect')
try:
 rc,text=c.command('status runtime',60);rows.append(dict(command='status runtime',status=rc,text=text));assert rc==0 and 'remaining=0' in text
 rc,text=c.command('bbvi --help',60);rows.append(dict(command='bbvi --help after reconnect',status=rc,text=text));assert rc==0
 screen=pyte.Screen(80,24);stream=pyte.Stream(screen);data=b''
 c.t.write(b'bbvi /tmp/vi-8k\r\n');seen(lambda:screen.display[22].startswith(f'{22:063d}') and screen.cursor.x==0 and screen.cursor.y==0)
 c.t.write(b':q!\r');reply=c.t.read_until(b' > ',60);assert reply.endswith(b' > ')
 rows.append(dict(command='bbvi /tmp/vi-8k after reconnect; discard quit',transcript=(data+reply).decode('latin1')))
 rc,text=c.command('cmp /tmp/vi-8k /tmp/vi-reference',60);rows.append(dict(command='cmp /tmp/vi-8k /tmp/vi-reference',status=rc,text=text));assert rc==0
finally:c.close();Path('outputs/busybox/lifecycle.json').write_text(json.dumps(rows,indent=2)+'\n')
print('PASS disconnect cleanup, bounded reconnect, subsequent edit, original unchanged')
