"""Actual vi incremental display versus its own forced redraw over Snow Telnet."""
import sys,time,json,pyte
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'xvi'))
from client import XviClient
c=XviClient(port=23250,password='');screen=pyte.Screen(80,24);stream=pyte.Stream(screen);rows=[]
def drain():
 data=c.drain(quiet=.6,timeout=15);stream.feed(data.decode('latin1'));return data
def send(data):
 dataout=b''
 for pos in range(0,len(data),16):
  c.t.write(data[pos:pos+16]);dataout+=drain()
 return dataout
try:
 rc,out=c.command('cp /tmp/vi-lines /tmp/vi-refresh');assert rc==0,out
 c.t.write(b'vi-fast /tmp/vi-refresh\r\n');time.sleep(1);drain();assert screen.display[0].startswith('0000 ')
 cases=[b'ljhk',b'20j',b'20k',b'\x1b[B'*5,b'\x1b[A'*5,b'15l',b'10h',b'G',b'gg',b':set tabstop=4\r',b':set tabstop=8\r',b'iINSERT\tHERE',b'\x1b',b'jk',b'u',b'jk',b'yy',b'p',b'jk',b'dd',b'jk',b'u',b'2w',b'b',b'$aTAIL',b'\x1b',b'jk',b'3x',b'.',b'u',b'jk',b'/0005\r',b'n',b'N',b'\x06',b'\x02',b'G',b'0',b'i'+b'L'*120,b'\x1b',b'60h',b'60l',b'gg',b':%s/abc/XYZ/g\r',b'jk',b'oqueued change',b'\x1b',b'jk'*35,b'xjk',b'ujk',b'rZjk',b'.jk',b'"ayy',b'"apjk',b':set tabstop=3\r',b'jk']
 in_insert=False
 for i,keys in enumerate(cases):
  send(keys)
  # Compare only normal mode: Ctrl-L is literal input in insert mode.
  if keys.startswith(b'i') or keys.startswith(b'$a') or keys.startswith(b'o'):in_insert=True
  if keys==b'\x1b':in_insert=False
  if in_insert:continue
  before=tuple(screen.display[:23]);cursor=(screen.cursor.x,screen.cursor.y)
  send(b'\x0c');after=tuple(screen.display[:23]);newcursor=(screen.cursor.x,screen.cursor.y)
  assert before==after,(i,keys,before,after)
  assert cursor==newcursor,(i,keys,cursor,newcursor)
  rows.append(dict(case=i,keys=repr(keys),document_and_cursor_match=True))
 send(b':wq\r');rc,out=c.command('status runtime');assert rc==0 and 'remaining=0' in out;rows.append(dict(metrics=out))
finally:
 c.close();Path('outputs/vi-response').mkdir(parents=True,exist_ok=True);Path('outputs/vi-response/refresh-guest.json').write_text(json.dumps(rows,indent=2)+'\n')
print('PASS',len(rows)-1,'incremental/forced redraw comparisons; motion, counts, edits, undo, paste, repeat, settings, long lines, scrolling; zero tracked survivors')
