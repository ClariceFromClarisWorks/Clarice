"""Isolated Snow Telnet tests; existing authenticated service, BINARY both ways."""
import sys,telnetlib,time,json,argparse
from pathlib import Path
import importlib.util
spec=importlib.util.spec_from_file_location('clarice_remote_client',Path(__file__).resolve().parents[1]/'remote-sessions/client.py')
module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
Client=module.Client
class XviClient(Client):
 def option(self,sock,cmd,opt):
  if cmd==telnetlib.WILL:sock.sendall(telnetlib.IAC+(telnetlib.DO if opt in (telnetlib.ECHO,telnetlib.SGA,telnetlib.BINARY) else telnetlib.DONT)+opt)
  elif cmd==telnetlib.DO:sock.sendall(telnetlib.IAC+(telnetlib.WILL if opt in (telnetlib.SGA,telnetlib.BINARY) else telnetlib.WONT)+opt)
 def drain(self,quiet=.3,timeout=30):
  data=b'';start=last=time.monotonic()
  while time.monotonic()-start<timeout:
   new=self.t.read_very_eager()
   if new:data+=new;last=time.monotonic()
   elif time.monotonic()-last>quiet:break
   time.sleep(.025)
  return data
 def edit(self,command,keys):
  start=time.monotonic();self.t.write(command.encode()+b'\r\n');data=self.t.read_until(b'\x1b[?25h',60)
  if b'\x1b[?25h' not in data:raise RuntimeError(repr(data))
  data+=self.drain(quiet=1,timeout=60);ready=time.monotonic()-start
  self.t.write(keys);data+=self.t.read_until(b' > ',60)
  if not data.endswith(b' > '):raise RuntimeError(repr(data))
  return dict(command=command,host_seconds_to_quiet_screen=ready,transcript=data.decode('latin1'))
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--port',type=int,default=23239);p.add_argument('--password-file',type=Path,required=True);p.add_argument('--output',type=Path,default=Path('outputs/xvi/guest-smoke.json'));a=p.parse_args()
 c=XviClient(port=a.port,password=a.password_file.read_text().strip());rows=[]
 try:
  for cmd in ['xvi --help','xvi --version','xvi --bad-option','appinstall check /bin/xvi','status runtime']:
   rc,text=c.command(cmd,60);rows.append(dict(command=cmd,status=rc,text=text))
  rows.append(c.edit('xvi /tmp/xvi-new.txt',b'iHello from xvi\x1b:wq\r'))
  for cmd in ['echo $?','cat /tmp/xvi-new.txt','status runtime']:
   rc,text=c.command(cmd,60);rows.append(dict(command=cmd,status=rc,text=text))
 finally:
  c.close();a.output.write_text(json.dumps(rows,indent=2)+'\n')
 print('Recorded',len(rows),'guest observations')
