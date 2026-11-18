"""Explicit USB serial test client; restores termios and never logs passwords."""
import fcntl,struct
import os,termios,tty,select,time,re
class SerialClient:
 def __init__(self,password,device='/dev/cu.usbmodem1401'):
  if not re.fullmatch(r'/dev/cu\.usbmodem[0-9]+',device):raise ValueError('USB modem required')
  self.fd=os.open(device,os.O_RDWR|os.O_NOCTTY|os.O_NONBLOCK);self.old=termios.tcgetattr(self.fd);self.pending=b'';self.seq=0
  try:
   tty.setraw(self.fd);a=termios.tcgetattr(self.fd);a[4]=a[5]=termios.B57600;a[2]|=termios.CLOCAL|termios.CREAD;termios.tcsetattr(self.fd,termios.TCSANOW,a)
   fcntl.ioctl(self.fd,termios.TIOCMBIS,struct.pack("I",termios.TIOCM_DTR|termios.TIOCM_RTS))
   time.sleep(1)
   started=time.monotonic();self.send(b'\r');self.until(b'Password: ',65);self.send(password.encode()+b'\r');answer=self.until(b' > ',120)
   if password.encode() in answer:raise RuntimeError('Password echoed')
   self.login_seconds=time.monotonic()-started
  except BaseException:self.close();raise
 def send(self,data):
  for v in data:
   if os.write(self.fd,bytes([v]))!=1:raise OSError('short serial write')
   time.sleep(.002)
 def until(self,needle,timeout=60):
  end=time.monotonic()+timeout
  while True:
   at=self.pending.find(needle)
   if at>=0:
    at+=len(needle);result=self.pending[:at];self.pending=self.pending[at:];return result
   if time.monotonic()>=end:raise TimeoutError(repr(self.pending))
   if select.select([self.fd],[],[],.1)[0]:self.pending+=os.read(self.fd,4096)
 def command(self,source,timeout=120):
  self.seq+=1;tag=f'SE_END_{self.seq}'
  self.send((source+"; printf '\\n"+tag+":%d\\n' $?\r").encode())
  data=b'';end=time.monotonic()+timeout
  while time.monotonic()<end:
   data+=self.until(b' > ',max(.1,end-time.monotonic()))
   m=re.search(rb'\r?\n'+tag.encode()+rb':([0-9]+)\r?\n',data)
   if m:return int(m[1]),data.decode('latin1')
  raise TimeoutError('No command completion marker')
 def close(self):
  if self.fd is not None:
   try:termios.tcsetattr(self.fd,termios.TCSANOW,self.old)
   finally:os.close(self.fd);self.fd=None
