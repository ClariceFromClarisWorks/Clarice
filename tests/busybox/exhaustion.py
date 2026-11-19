"""Real guest exhaustion, reconnect and allocation cleanup (disposable data)."""
import sys,time,json
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'xvi'))
from client import XviClient
password=Path(sys.argv[1]).read_text().strip();rows=[]
def connect():
 for attempt in range(5):
  try:return XviClient(port=23241,password=password)
  except (EOFError,OSError,RuntimeError):
   if attempt==4:raise
   time.sleep(1)
c=connect()
try:
 for cmd in ['dd if=/dev/zero of=/tmp/bbvi-large bs=4096 count=128','wc -c /tmp/bbvi-large']:
  rc,text=c.command(cmd,90);rows.append(dict(command=cmd,status=rc,text=text));assert rc==0
 assert '524288' in text
 for i in range(3):
  c.t.write(b'vi /tmp/bbvi-large\r\n');deadline=time.monotonic()+60;data=b''
  while time.monotonic()<deadline:
   try:data+=c.t.read_very_eager()
   except EOFError:break
   time.sleep(.05)
  else:raise AssertionError('exhaustion did not end editor session')
  c.close();time.sleep(1);c=connect()
  rc,text=c.command('status runtime',60);rows.append(dict(attempt=i+1,transcript=data.decode('latin1'),metrics=text));assert rc==0 and 'remaining=0' in text
  rc,text=c.command('vi --help',60);assert rc==0
 rc,text=c.command('rm /tmp/bbvi-large',60);assert rc==0
finally:c.close();Path('outputs/busybox-completion/exhaustion.json').write_text(json.dumps(rows,indent=2)+'\n')
print('PASS 512 KiB exhaustion repeated three times, reconnect, zero tracked survivors, subsequent command')
