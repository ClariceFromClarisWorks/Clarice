#!/usr/bin/env python3
"""Focused functional Snow tests. Requires running Clarice at its own prompt."""
import argparse,subprocess,time,json,signal
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--pid',required=True);p.add_argument('--device',required=True);p.add_argument('--resume',action='store_true');a=p.parse_args()
r=Path(__file__).resolve().parents[2];out=r/'outputs/serial-3';work=r/'work/serial-3';results=[]
def keys(s):
 f=work/'fixture.keys';f.write_bytes(s.encode('ascii'));subprocess.run([str(r/'work/serial-2/keys'),a.pid,str(f),'.09'],check=True,stdout=subprocess.DEVNULL)
def metrics(name):keys('echo $? > /tmp/'+name+'code\rstatus runtime > /tmp/'+name+'\r');time.sleep(.5)
def transfer(name,action,path,guest,cancel=False):
 log=(out/(name+'-peer.log')).open('w');t=time.monotonic()
 peer=subprocess.Popen([str(r/'work/kermit/kermit'),'-d',a.device,action,str(path)],stdout=log,stderr=log)
 try:
  keys(guest+'\r')
  if cancel:time.sleep(2);keys('\x03')
  rc=peer.wait(timeout=30);results.append(dict(test=name,peer_exit=rc,seconds_including_typing=time.monotonic()-t))
  (out/'snow-results.json').write_text(json.dumps({'checks':results,'complete':False},indent=2)+'\n')
  assert (rc!=0 if cancel else rc==0),name
 finally:
  if peer.poll() is None:peer.send_signal(signal.SIGINT);peer.wait(timeout=12)
  log.close()
 time.sleep(.5);metrics(name)
if not a.resume:
 metrics('received')
 transfer('send','receive',work/'from-guest.dat','kermit send /tmp/send.dat')
 assert (work/'from-guest.dat').read_bytes()==(work/'binary.dat').read_bytes()
transfer('roundtrip','receive',work/'roundtrip.dat','kermit send /tmp/recv.dat')
assert (work/'roundtrip.dat').read_bytes()==(work/'binary.dat').read_bytes()
(work/'large.dat').write_bytes(bytes(range(256))*512)
transfer('cancel','send',work/'large.dat','kermit receive /tmp/cancel.dat',True)
transfer('recovery','send',work/'binary.dat','kermit receive /tmp/cancel.dat')
transfer('recovered','receive',work/'recovered.dat','kermit send /tmp/cancel.dat')
assert (work/'recovered.dat').read_bytes()==(work/'binary.dat').read_bytes()
transfer('empty','send',work/'empty.dat','kermit receive /tmp/empty.dat')
keys('kermit receive /tmp/recv.dat\r');time.sleep(1);metrics('refused')
keys('status runtime\r')
(out/'snow-results.json').write_text(json.dumps({'checks':results,'binary_comparisons':True},indent=2)+'\n')
print('PASS: guest transfers, roundtrip bytes, cancel/recovery, empty; inspect guest statuses and disk cleanup separately')
