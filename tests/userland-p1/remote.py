from pathlib import Path
import sys,time,json
sys.path.insert(0,'tests/remote-sessions');from client import Client
c=Client(port=23259,password='');rows=[]
try:
 c.t.write(b'''read value; printf 'consumed=<%s>\\n' "$value"\r\ntouch /tmp/P1-BAD\r\nprintf 'QUEUE-DONE\\n'\r\n''')
 out=c.t.read_until(b'\r\nQUEUE-DONE\r\n',30)+c.t.read_until(b' > ',10)
 assert b'consumed=<touch /tmp/P1-BAD>' in out,out
 rows.append(dict(test='queued-read',output=out.decode('latin1')))
 rc,out=c.command('test ! -e /tmp/P1-BAD');assert rc==0,out
 rows.append(dict(test='not-executed',status=rc,output=out))
 c.t.write(b'while :; do :; done\r\n');time.sleep(1);c.t.write(b'\x03');out=c.t.read_until(b' > ',30)
 rc,result=c.command('echo $?');assert rc==0 and '\r\n130\r\n' in result,(out,result)
 rows.append(dict(test='cancel',output=out.decode('latin1')+result))
 rc,out=c.command('echo recovered; status runtime');assert rc==0 and 'remaining=0' in out,out
 rows.append(dict(test='recovered',output=out))
finally:c.close()
Path('outputs/userland-p1/guest-remote.json').write_text(json.dumps(rows,indent=2)+'\n')
print('PASS consumed read line not dispatched; following queued command once; remote Ctrl+C 130 and recovery')
