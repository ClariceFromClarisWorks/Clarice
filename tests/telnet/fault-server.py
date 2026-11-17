"""Four finite, peer-restricted malformed-input/recovery fixtures. No shell."""
import argparse, json, socket
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--bind',required=True);p.add_argument('--peer',required=True)
p.add_argument('--port',type=int,default=2325);a=p.parse_args()
cases=[('malformed',bytes([255,250,24,255,42])),
       ('oversized',bytes([255,250,42])+b'x'*16385),
       ('truncated',bytes([255,251])),('recovery',b'RECOVERY_OK\r\n')]
with socket.socket() as s:
 s.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1);s.bind((a.bind,a.port));s.listen(1);s.settimeout(180)
 for name,data in cases:
  while True:
   c,peer=s.accept()
   if peer[0]==a.peer:break
   c.close()
  with c:
   c.settimeout(5)
   try:c.sendall(data);c.shutdown(socket.SHUT_WR)
   except (BrokenPipeError,ConnectionResetError):pass
   try:
    while c.recv(512):pass
   except (TimeoutError,ConnectionResetError):pass
   print(json.dumps(dict(case=name,bytes=len(data))),flush=True)
