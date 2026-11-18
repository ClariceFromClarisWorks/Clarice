"""Explicit physical-SE acceptance client using Python's established Telnet engine.
Run with macOS Python 3.9 (telnetlib); explicit trusted-LAN SE only.
"""
import telnetlib,time,re,json,argparse,os
from pathlib import Path
class Client:
 def __init__(self,port=2323,password=None,host="192.168.1.252"):
  self.t=telnetlib.Telnet(host,port,8);self.t.set_option_negotiation_callback(self.option);self.seq=0
  password=os.environ.get('CLARICE_TEST_PASSWORD') if password is None else password
  self.greeting=self.t.read_until(b'Password: ' if password else b' > ',20)
  if password:
   if not self.greeting.endswith(b'Password: '):raise RuntimeError('Password prompt missing')
   self.t.write(password.encode()+b'\r\n');answer=self.t.read_until(b' > ',120)
   if password.encode() in answer:raise RuntimeError('Password echoed')
   if not answer.endswith(b' > '):raise RuntimeError('Authentication did not reach prompt')
   self.greeting+=answer
  if b'Clarice remote shell' not in self.greeting:raise RuntimeError(repr(self.greeting))
 def option(self,sock,cmd,opt):
  if cmd==telnetlib.WILL:sock.sendall(telnetlib.IAC+(telnetlib.DO if opt in (telnetlib.ECHO,telnetlib.SGA,telnetlib.BINARY) else telnetlib.DONT)+opt)
  elif cmd==telnetlib.DO:sock.sendall(telnetlib.IAC+(telnetlib.WILL if opt in (telnetlib.SGA,telnetlib.BINARY) else telnetlib.WONT)+opt)
 def command(self,source,timeout=30):
  self.seq+=1;tag=f'REMOTE_END_{self.seq}'
  self.t.write((source+"; printf '\\n"+tag+":%d\\n' $?\r\n").encode())
  data=b'';end=time.monotonic()+timeout
  pattern=re.compile(rb'\r?\n'+tag.encode()+rb':([0-9]+)\r?\n')
  while time.monotonic()<end:
   data+=self.t.read_until(b' > ',max(.1,end-time.monotonic()))
   match=pattern.search(data)
   if match and data.endswith(b' > '):return int(match[1]),data.decode('latin1')
  raise TimeoutError(repr(data))
 def close(self):self.t.close()
