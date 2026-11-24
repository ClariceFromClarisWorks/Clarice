"""Disposable physical checks for the concurrently integrated filesystem fixes."""
import json,re
from pathlib import Path
from serial_client import SerialClient
c=SerialClient(Path('work/se-performance/password').read_text().strip());rows=[]
out=Path('outputs/se-performance-20261118/integration1841-hardware.json')
def check(command,expected=0,contains=None):
 status,text=c.command(command);rows.append(dict(command=command,status=status,text=text));out.write_text(json.dumps(rows,indent=2)+'\n')
 assert status==expected,text
 if contains is not None:assert contains in text,text
 return text
try:
 check('mkdir /tmp/sf1841') # Existing fixtures fail; never erase or reuse them.
 check("printf 'NEW\\n' > /tmp/sf1841/source; printf 'OLD\\n' > /tmp/sf1841/dest")
 check('hfsattrib --locked on /tmp/sf1841/source')
 try:
  check('mv /tmp/sf1841/source /tmp/sf1841/dest',1)
  check('cat /tmp/sf1841/source /tmp/sf1841/dest',contains='\r\nNEW\r\nOLD\r\n')
 finally:check('hfsattrib --locked off /tmp/sf1841/source')
 check("mkdir ''",1)
 check('mkdir -p /tmp/sf1841/a/b; cd /tmp/sf1841/a/b')
 check('mv /tmp/sf1841/a /tmp/sf1841/c; pwd',contains='/tmp/sf1841/c/b')
 check('cd /')
 check('nano /tmp/fe18',2,contains='local input ownership')
 check('telnetd --help',contains='arrows/Home/End')
 c.send(b"printf 'AC'\x1b[D\x1b[DB\r")
 raw=c.until(b' > ',30).decode('latin1');rows.append(dict(test='serial cursor insertion',text=raw));out.write_text(json.dumps(rows,indent=2)+'\n')
 assert '\r\nABC' in raw,raw
 check('cat /tmp/fe18',contains='\r\nSE EDIT ORIGINAL\r\n')
 check('hfsattrib /tmp/fe18',contains="type='TEXT' creator='ttxt'")
 check('cat /tmp/up18log')
 check('status runtime',contains='remaining=0')
 c.send(b'exit\r');print('PASS physical combined filesystem, remote diagnostics/cursor and preserved nano file')
finally:c.close()
