"""Disposable HTTP/FTP workload peers. Serve only generated benchmark fixtures."""
from pathlib import Path
from http.server import BaseHTTPRequestHandler,ThreadingHTTPServer
import threading,time,sys
from pyftpdlib.authorizers import DummyAuthorizer
from pyftpdlib.handlers import FTPHandler
from pyftpdlib.servers import FTPServer
root=Path('work/network-4d/server').resolve();bind='192.168.1.112'
class HTTP(BaseHTTPRequestHandler):
 def do_GET(self):
  mode,size=self.path.strip('/').split('/');data=(root/size).read_bytes()
  if mode=='idle':time.sleep(.75)
  if mode=='stall':time.sleep(40)
  self.send_response(200);self.send_header('Content-Length',str(len(data)))
  if mode=='header':self.send_header('X-Padding','x'*4000)
  self.end_headers()
  try:
   for at in range(0,len(data),2048):
    self.wfile.write(data[at:at+2048]);self.wfile.flush()
    if mode=='slow':time.sleep(.05)
  except (BrokenPipeError,ConnectionResetError):pass
 def log_message(self,*a):pass
threading.Thread(target=ThreadingHTTPServer((bind,18085),HTTP).serve_forever,daemon=True).start()
a=DummyAuthorizer();a.add_anonymous(str(root),perm='elradfmwMT')
FTPHandler.authorizer=a;FTPHandler.passive_ports=range(22380,22390)
print('HTTP18085 FTP22368 ready',flush=True);FTPServer((bind,22368),FTPHandler).serve_forever()
