"""Peer-filter wire checks. Configure the guest's allowed peer separately.
A deny pass requires actual EOF/reset with zero bytes, never merely a timeout.
Passwords are read from a private file and never recorded. No guest files changed.
"""
import argparse,json,socket,time
from pathlib import Path
from client import Client
p=argparse.ArgumentParser()
p.add_argument('mode',choices=['allow','deny'])
p.add_argument('--host',default='127.0.0.1')
p.add_argument('--port',type=int,default=23291)
p.add_argument('--password-file',type=Path)
p.add_argument('--output',type=Path,required=True)
p.add_argument('--samples',type=int,default=3)
a=p.parse_args();rows=[]
try:
 for i in range(a.samples):
  start=time.monotonic()
  if a.mode=='deny':
   received=b'';outcome=None
   with socket.create_connection((a.host,a.port),10) as s:
    s.settimeout(15)
    try:
     s.sendall(b"printf 'PEER_REJECTION_EXECUTED'\r\n")
     while True:
      b=s.recv(4096)
      if not b:outcome='eof';break
      received+=b
      if len(received)>16384:raise AssertionError('Unexpected output exceeds bound')
    except (ConnectionResetError,BrokenPipeError):outcome='reset'
   row=dict(sample=i,mode=a.mode,outcome=outcome,received_hex=received.hex(),host_seconds=time.monotonic()-start)
   rows.append(row);assert outcome in ('eof','reset') and not received,row
  else:
   if not a.password_file:raise ValueError('--password-file required for allowed authentication')
   c=Client(host=a.host,port=a.port,password=a.password_file.read_text().strip())
   try:
    status,text=c.command("printf 'ALLOWED_PEER_OK\\n'; printf 'alpha\\nbeta\\n' | grep beta && status runtime",120)
    row=dict(sample=i,mode=a.mode,status=status,text=text,host_seconds=time.monotonic()-start)
    rows.append(row);assert status==0 and 'ALLOWED_PEER_OK' in text and '\r\nbeta\r\n' in text and 'remaining=0' in text,row
    c.t.write(b'exit\r\n')
   finally:c.close()
  a.output.write_text(json.dumps(rows,indent=2)+'\n')
  print('PASS',a.mode,i,flush=True);time.sleep(1)
finally:
 a.output.write_text(json.dumps(rows,indent=2)+'\n')
