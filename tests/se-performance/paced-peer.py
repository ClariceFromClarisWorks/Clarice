"""Run a peer against a private PTY bridged to USB serial at 2 ms/byte.
Bounded 8192-byte queues; no byte logging (authentication stays private).
Usage: paced-peer.py COMMAND ... {PTY} ...
"""
import os,pty,tty,termios,fcntl,struct,select,subprocess,sys,time
assert '{PTY}' in sys.argv[1:]
fd=os.open('/dev/cu.usbmodem1401',os.O_RDWR|os.O_NOCTTY|os.O_NONBLOCK)
old=termios.tcgetattr(fd);master,slave=pty.openpty();child=None
try:
 tty.setraw(fd);a=termios.tcgetattr(fd);a[4]=a[5]=termios.B57600;a[2]|=termios.CLOCAL|termios.CREAD;termios.tcsetattr(fd,termios.TCSANOW,a)
 fcntl.ioctl(fd,termios.TIOCMBIS,struct.pack('I',termios.TIOCM_DTR|termios.TIOCM_RTS));tty.setraw(slave);os.set_blocking(master,False)
 time.sleep(1);child=subprocess.Popen([os.ttyname(slave) if s=='{PTY}' else s for s in sys.argv[1:]])
 outgoing=bytearray();incoming=bytearray();next_byte=0;end=time.monotonic()+600
 while child.poll() is None:
  if time.monotonic()>end:raise TimeoutError('peer exceeded 600 seconds')
  readers=([master] if len(outgoing)<4096 else [])+([fd] if len(incoming)<4096 else [])
  ready,_,_=select.select(readers,[],[],.001)
  if master in ready:outgoing.extend(os.read(master,4096))
  if fd in ready:incoming.extend(os.read(fd,4096))
  if len(outgoing)>8192 or len(incoming)>8192:raise RuntimeError('bridge queue overflow')
  if outgoing and time.monotonic()>=next_byte:
   n=os.write(fd,outgoing[:1]);del outgoing[:n];next_byte=time.monotonic()+.002
  if incoming:
   try:n=os.write(master,incoming);del incoming[:n]
   except BlockingIOError:pass
 raise SystemExit(child.returncode)
finally:
 if child is not None and child.poll() is None:child.terminate();child.wait(timeout=5)
 termios.tcsetattr(fd,termios.TCSANOW,old)
 for f in [fd,master,slave]:os.close(f)
