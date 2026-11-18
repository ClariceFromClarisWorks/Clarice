import argparse,json,time,telnetlib
from pathlib import Path
from client import XviClient
p=argparse.ArgumentParser();p.add_argument('--password-file',type=Path,required=True);a=p.parse_args();password=a.password_file.read_text().strip();rows=[];out=Path('outputs/xvi/lifecycle.json')
def save():out.write_text(json.dumps(rows,indent=2)+'\n')
def connect():return XviClient(port=23239,password=password)
c=connect()
def cmd(s,expected=0):
 rc,text=c.command(s,90);rows.append(dict(command=s,status=rc,text=text));save();assert rc==expected,(s,rc,text);return text
def edit(s,k):
 r=c.edit(s,k);rows.append(r);save();cmd('status runtime');return r
try:
 cmd('cp -p /mnt/GlyphFull/Clarice /tmp/xvi-forks')
 cmd('hfsattr /mnt/GlyphFull/Clarice /tmp/xvi-forks')
 edit('xvi /tmp/xvi-forks',b'iFork preservation\x1b:wq\r')
 cmd('hfsattr /mnt/GlyphFull/Clarice /tmp/xvi-forks')
 cmd('cat /tmp/xvi-forks')
 edit('xvi /tmp/xvi-arrows',b'ione\rtwo\x1b\x1b[A0iX\x1b:wq\r')
 text=cmd('cat /tmp/xvi-arrows');assert 'Xone\r\ntwo' in text
 # Disconnect while editing unsaved text, reconnect and prove shell ownership.
 c.t.write(b'xvi /tmp/xvi-disconnect\r\n');data=c.t.read_until(b'\x1b[?25h',60)+c.drain(quiet=1,timeout=60);c.t.write(b'iunsaved');data+=c.drain(quiet=1);c.close();rows.append(dict(test='disconnect during edit',transcript=data.decode('latin1')));save();time.sleep(1)
 c=connect();cmd('status runtime');cmd('test ! -f /tmp/xvi-disconnect')
 edit('xvi /tmp/xvi-after-disconnect',b'irecovered\x1b:wq\r');cmd('cat /tmp/xvi-after-disconnect')
 c.t.write(b'xvi\r\n');data=c.t.read_until(b'\x1b[?25h',60)+c.drain(quiet=1,timeout=60);c.t.write(b'\x03')
 ended=False;start=time.monotonic()
 while time.monotonic()-start<30:
  try:data+=c.t.read_very_eager()
  except EOFError:ended=True;break
  time.sleep(.1)
 rows.append(dict(test='Ctrl-C raw editor',connection_closed=ended,transcript=data.decode('latin1')));save();c.close();assert ended
 c=connect();cmd('status runtime');cmd('echo TELNET-RECOVERED')
finally:c.close();save()
print('PASS',len(rows),'observations; Ctrl-C closes the session (reported limitation)')
