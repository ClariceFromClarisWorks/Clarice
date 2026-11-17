"""Focused guest reconnect fixture: run against a disposable Snow helper.
No physical-machine addresses or tokens are stored in this script.
"""
import argparse,json,socket,sys,time
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
from host import Wire,capture
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--bind',required=True);p.add_argument('--peer',required=True)
p.add_argument('--port',type=int,required=True);p.add_argument('--token-file',type=Path,required=True)
p.add_argument('--output',type=Path,required=True);args=p.parse_args()
from host import local_address
local_address(args.bind);local_address(args.peer)
out=args.output;out.mkdir(parents=True,exist_ok=True);token=args.token_file.read_text().strip()
log=[]
def note(**kw):log.append(kw);print(json.dumps(kw),flush=True);(out/'snow.json').write_text(json.dumps(log,indent=2)+'\n')
def listen():
 s=socket.socket();s.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1);s.bind((args.bind,args.port));s.listen();s.settimeout(120);return s
def accept(server):
 at=time.monotonic()
 while time.monotonic()-at<120:
  c,peer=server.accept();assert peer[0]==args.peer,peer
  w=Wire(c,5)
  try:hello=w.line()
  except (ConnectionError,TimeoutError):
   note(event='stale-NAT-candidate-rejected');c.close();continue
  assert hello=='HIL2 '+token
  w=Wire(c,5)
  try:
   c.sendall(b'PING\n');r=w.line();assert r.startswith('PONG ')
  except (ConnectionError,TimeoutError):
   note(event='unresponsive-NAT-candidate-rejected');c.close();continue
  w=Wire(c,30);note(event='connected',seconds=round(time.monotonic()-at,3));return c,w
 raise TimeoutError('No valid reconnect')
def cmd(w,s):w.reset();w.sock.sendall(s.encode()+b'\n');return w.line()
def ping(w):
 s=cmd(w,'PING');assert s.startswith('PONG '),s;note(event='ping',reply=s);return s
s=listen();print('LISTENING',flush=True)
c,w=accept(s);ping(w);note(**capture(w,out/'snow-connected.png'))
# A partial input line belongs only to its original session.
c.sendall(b'KEY 120 7 0');c.close();c,w=accept(s)
assert cmd(w,'')=='ERR command';ping(w);note(**capture(w,out/'snow-partial-discard.png'))
# Disconnect while a finite button-down plan is in progress.
assert cmd(w,'DRAG 200 52 210 62 180')=='QUEUED';time.sleep(.15);c.close();c,w=accept(s)
r=ping(w);assert 'active=0 down=0' in r
# Break a screenshot mid-frame; its allocation must be released.
c.sendall(b'GRAB\n');w.reset();assert w.line().startswith('FRAME ');c.close();c,w=accept(s);ping(w)
# Host/listener disappears entirely; guest must keep yielding and retry.
heaps=[]
for i in range(8):
 c.close();c,w=accept(s);r=ping(w);heaps.append(int(r.split()[2]));assert 'active=0 down=0' in r
assert len(set(heaps))==1,heaps
note(event='cycles-pass',heaps=heaps)
c.close();s.close();note(event='listener-off');time.sleep(10);s=listen();c,w=accept(s);ping(w)
note(**capture(w,out/'snow-final.png'));assert cmd(w,'QUIT')=='BYE';c.close();s.close();note(event='quit-pass')
