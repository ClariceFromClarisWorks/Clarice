"""Matched interleaved Snow/Telnet comparison; real ANSI display + key oracle."""
import sys,json,time,pyte
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'xvi'))
from client import XviClient
c=XviClient(port=23240,password=Path(sys.argv[1]).read_text().strip());rows=[]
def screen_wait(screen,stream,predicate,timeout=90):
 start=time.monotonic();raw=b''
 while time.monotonic()-start<timeout:
  data=c.t.read_very_eager()
  if data:raw+=data;stream.feed(data.decode('latin1'))
  if predicate():return raw
  time.sleep(.01)
 raise TimeoutError(repr(raw)+'\n'+'\n'.join(screen.display))
try:
 rc,text=c.command('test -f /tmp/vi-8k',60)
 if rc:
  rc,text=c.command("awk 'BEGIN { for(i=0;i<128;i++) printf \"%063d\\n\",i }' > /tmp/vi-8k",90);assert rc==0,text
 rc,text=c.command('cat /tmp/vi-8k',60);assert rc==0,text
 expected=''.join(f'{i:063d}\r\n' for i in range(128));assert expected in text
 rc,text=c.command('wc -c /tmp/vi-8k',60);assert rc==0 and '8192' in text,text
 Path('outputs/vi/fixture.json').write_text(json.dumps(dict(command='cat and wc -c /tmp/vi-8k',exact_128_numbered_lines=True,text=text),indent=2)+'\n')
 for run in range(3):
  for path in ['', '/tmp/vi-8k']:
   for editor in (['vi','xvi'] if run%2==0 else ['xvi','vi']):
    screen=pyte.Screen(80,24);stream=pyte.Stream(screen);command=editor+(' '+path if path else '')
    start=time.monotonic();c.t.write(command.encode()+b'\r\n')
    def ready():
     expected=f'{22:063d}' if path else '~'
     return screen.display[22].startswith(expected) and screen.cursor.x==0 and screen.cursor.y==0
    raw=screen_wait(screen,stream,ready);ready_at=time.monotonic();startup_bytes=len(raw)
    c.t.write(b'iX');raw+=screen_wait(screen,stream,lambda:screen.display[0].startswith('X') and screen.cursor.x==1 and screen.cursor.y==0)
    input_at=time.monotonic();c.t.write(b'\x1b:q!\r');raw+=c.t.read_until(b' > ',60);end=time.monotonic();assert raw.endswith(b' > ')
    rc,metrics=c.command('status runtime',60);assert rc==0 and 'remaining=0' in metrics
    row=dict(editor=editor,command=command,run=run+1,host_submit_to_full_screen_seconds=ready_at-start,host_submit_to_key_response_seconds=input_at-start,host_key_response_seconds=input_at-ready_at,host_complete_open_key_quit_seconds=end-start,startup_bytes=startup_bytes,total_bytes=len(raw),metrics=metrics)
    rows.append(row);Path('outputs/vi/measurements.json').write_text(json.dumps(rows,indent=2)+'\n');print(editor,path,run+1,round(ready_at-start,3),round(end-start,3),flush=True)
finally:c.close()
