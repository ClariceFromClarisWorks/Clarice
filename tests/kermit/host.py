#!/usr/bin/env python3
"""Exercise the actual adapter/engine across a local PTY relay (no guest)."""
import os,pty,subprocess,tempfile,selectors,time,json,hashlib,signal
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
EXE=Path(os.environ.get('KERMIT_TEST_EXE',str(ROOT/'work/kermit/kermit')))
results=[]
def test(label,data,interrupt=False,damage=False):
 with tempfile.TemporaryDirectory() as d:
  d=Path(d);source=d/'source';dest=d/'dest';source.write_bytes(data)
  a,sa=pty.openpty();b,sb=pty.openpty()
  logs=[open(d/'send.log','wb'),open(d/'receive.log','wb')]
  env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0'}
  receiver=subprocess.Popen([EXE,'-d',os.ttyname(sb),'receive',dest],stdout=logs[1],stderr=logs[1],env=env)
  time.sleep(.05)
  sender=subprocess.Popen([EXE,'-d',os.ttyname(sa),'send',source],stdout=logs[0],stderr=logs[0],env=env)
  sel=selectors.DefaultSelector();sel.register(a,selectors.EVENT_READ,b);sel.register(b,selectors.EVENT_READ,a)
  start=time.monotonic();count=0;changed=False;cancelled=False;maximum_short=False;last_byte=b''
  try:
   while time.monotonic()-start<35 and (sender.poll() is None or receiver.poll() is None):
    for key,_ in sel.select(.02):
     try:chunk=os.read(key.fd,4096)
     except OSError:continue
     if os.environ.get("WIRE_DEBUG"): print("SEND" if key.fd==a else "RECV",repr(chunk),flush=True)
     if key.fd==a:
      count+=len(chunk);maximum_short |= b'\x01~' in last_byte+chunk;last_byte=chunk[-1:]
     if damage and not changed and b'D' in chunk and key.fd==a:
      chunk=bytearray(chunk);chunk[-3]^=1;chunk=bytes(chunk);changed=True
     if interrupt and count>200 and not cancelled:
      receiver.send_signal(signal.SIGINT);sender.send_signal(signal.SIGINT);cancelled=True
     if not cancelled:os.write(key.data,chunk)
   if sender.poll() is None:sender.kill()
   if receiver.poll() is None:receiver.kill()
   codes=[sender.wait(),receiver.wait()]
   if interrupt:assert codes==[130,130] and not dest.exists(),codes
   else:assert codes==[0,0] and dest.read_bytes()==data,codes
   assert not list(d.glob('*.clarice-*'))
   assert not damage or changed
   results.append(dict(test=label,pass_=True,bytes=len(data),seconds=time.monotonic()-start,codes=codes,corruption_injected=changed,maximum_short=maximum_short))
  except BaseException:
   for l in logs:l.flush()
   print("FAILED",label);print((d/'send.log').read_text());print((d/'receive.log').read_text());raise
  finally:
   for p in (sender,receiver):
    if p.poll() is None:p.kill();p.wait()
   for fd in (a,sa,b,sb):os.close(fd)
   for l in logs:l.close()
for label,data in [('empty',b''),('all-bytes',bytes(range(256))*16),('repeat',b'\0'*4097),('boundary',bytes(range(256))*2+b'!')]:test(label,data)
test('checksum-retry',bytes(range(256))*8,damage=True)
test('cancel',bytes(range(256))*512,interrupt=True)
test('recovery',bytes(range(256))*8)
print(json.dumps(results,indent=2))
