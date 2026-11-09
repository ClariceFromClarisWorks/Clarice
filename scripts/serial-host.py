#!/usr/bin/env python3
"""Local PTY peer for Snow's modem port; no network listener or shell execution."""
import argparse,os,select,time,tty,json
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('pty');p.add_argument('--log',default='work/serial-host.jsonl');p.add_argument('--seconds',type=int,default=600);a=p.parse_args()
fd=os.open(a.pty,os.O_RDWR|os.O_NOCTTY|os.O_NONBLOCK);tty.setraw(fd)
Path(a.log).parent.mkdir(parents=True,exist_ok=True)
log=open(a.log,'a',buffering=1);pending=bytearray();line=bytearray();deadline=time.monotonic()+a.seconds
try:
 while time.monotonic()<deadline:
  ready,write,_=select.select([fd],[fd] if pending else [],[],0.25)
  if write:
   n=os.write(fd,pending[:256]);log.write(json.dumps({'direction':'host-to-guest','hex':bytes(pending[:n]).hex()})+'\n');del pending[:n]
  if ready:
   try:data=os.read(fd,256)
   except BlockingIOError:continue
   if not data:break
   log.write(json.dumps({'direction':'guest-to-host','hex':data.hex()})+'\n')
   for c in data:
    if c in (10,13):
     if not line:continue
     text=line.decode('ascii',errors='replace');line.clear()
     if text=='CLARICE-PING':reply='CLARICE-PONG\r\n'
     elif text=='help':reply='Host test peer: help, echo TEXT, ping, quit\r\n'
     elif text=='ping':reply='pong\r\n'
     elif text.startswith('echo '):reply=text[5:]+'\r\n'
     elif text=='quit':reply='Goodbye from host test peer\r\n'
     else:reply='Unknown test command; type help\r\n'
     pending.extend(reply.encode('ascii','replace'))
    elif c in (8,127):
     if line:line.pop()
    elif len(line)<255:line.append(c)
   if len(pending)>4096:raise RuntimeError('host output queue exceeded4096 bytes')
finally:
 os.close(fd);log.close()
