"""Observe real ANSI screen contents and key response with pyte 0.8.2.
Host timings include network/scheduling. This does not estimate physical SE time.
"""
import argparse,json,time,pyte
from pathlib import Path
from client import XviClient
p=argparse.ArgumentParser();p.add_argument('--password-file',type=Path,required=True);a=p.parse_args()
c=XviClient(port=23239,password=a.password_file.read_text().strip());rows=[]
def await_screen(screen,stream,predicate,timeout=90):
 start=time.monotonic();raw=b''
 while time.monotonic()-start<timeout:
  data=c.t.read_very_eager()
  if data:raw+=data;stream.feed(data.decode('latin1'))
  if predicate():return raw
  time.sleep(.01)
 raise TimeoutError('\n'.join(screen.display))
try:
 for path in ['', '/tmp/xvi-8k']:
  for run in range(3):
   screen=pyte.Screen(80,24);stream=pyte.Stream(screen)
   command='xvi'+(' '+path if path else '')
   start=time.monotonic();c.t.write(command.encode()+b'\r\n')
   def ready():
    expected=f'{22:063d}' if path else '~'
    return screen.display[22].startswith(expected) and screen.cursor.x==0 and screen.cursor.y==0
   raw=await_screen(screen,stream,ready);ready_at=time.monotonic()
   c.t.write(b'iX');raw+=await_screen(screen,stream,lambda:screen.display[0].startswith('X') and screen.cursor.x==1 and screen.cursor.y==0)
   input_at=time.monotonic();c.t.write(b'\x1b:q!\r');raw+=c.t.read_until(b' > ',60);end=time.monotonic()
   assert raw.endswith(b' > ')
   rc,metrics=c.command('status runtime',60);assert rc==0 and 'remaining=0' in metrics
   rows.append(dict(command=command,run=run+1,host_submit_to_full_screen_seconds=ready_at-start,host_key_response_seconds=input_at-ready_at,host_complete_open_key_quit_seconds=end-start,bytes=len(raw),metrics=metrics))
   Path('outputs/xvi/screen-measurements.json').write_text(json.dumps(rows,indent=2)+'\n')
finally:c.close()
print('PASS',len(rows),'full-screen and keyboard-response observations')
