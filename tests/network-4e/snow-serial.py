"""Normal optional Seriald lifecycle on a disposable Snow modem PTY only."""
import os,sys,select,time,tty,json,subprocess,re,argparse
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--pid',required=True);p.add_argument('--pty',required=True);a=p.parse_args()
sys.path.insert(0,'tests/remote-sessions');from client import Client
ctl='/Users/williamprestwich/Documents/ChatGPT/Clarice/work/snow-performance/control'
rows=[];out=Path('outputs/network-4e/serial-snow.json')
def record(name,**kw):
 rows.append(dict(test=name,**kw));out.write_text(json.dumps(rows,indent=2)+'\n');print(name,kw,flush=True)
def keys(s):
 f=Path('work/network-4e/local.keys');f.write_bytes(s.encode());subprocess.run([ctl,a.pid,'activate'],check=True);subprocess.run(['work/network-4c/keys',a.pid,str(f),'.15'],check=True)
def remote(s):
 c=Client(port=23265,password='');rc,text=c.command(s,90);c.close();time.sleep(.8);assert not rc,text;record(s,text=text);return text
fd=os.open(a.pty,os.O_RDWR|os.O_NOCTTY|os.O_NONBLOCK);tty.setraw(fd)
def receive(mark=b' > ',timeout=20):
 end=time.monotonic()+timeout;b=b''
 while time.monotonic()<end:
  if select.select([fd],[],[],.1)[0]:b+=os.read(fd,65536)
  if mark in b:return b
 raise TimeoutError(repr(b))
try:
 remote('netheap2; status runtime')
 for cycle in range(3):
  keys('sh /tmp/serial-on.sh\r');time.sleep(.7)
  os.write(fd,b'\r');banner=receive(b'Password: ');os.write(fd,b'fixture\r');login=receive();assert b'fixture' not in login
  os.write(fd,b'echo SERIAL-OK\r');reply=receive();assert b'\r\nSERIAL-OK\r\n' in reply
  os.write(fd,b'read x\r');time.sleep(.25);os.write(fd,b'\x03');cancel=receive()
  os.write(fd,b'echo $?\r');status=receive();assert b'\r\n130\r\n' in status,status
  os.write(fd,b'echo RECOVERED\r');recovered=receive();assert b'\r\nRECOVERED\r\n' in recovered
  os.write(fd,b'exit\r');time.sleep(.5)
  record('normal Seriald login/read cancel130/recovery/logout',cycle=cycle,banner=banner.decode('latin1'),reply=reply.decode('latin1'),cancel=cancel.decode('latin1'),status=status.decode('latin1'))
  keys('sh /tmp/serial-off.sh\r');time.sleep(.6)
  remote('cat /tmp/stopped-heap; netheap2; status runtime; remote status')
 remote('/tmp/serial-start; status runtime; netheap2')
finally:os.close(fd)
print('PASS normal Seriald repeated start/login/command/cancel/logout/stop, Telnet recovery and native fault probe')
