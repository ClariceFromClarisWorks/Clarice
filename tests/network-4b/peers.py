"""Disposable local echo/stall/HTTP peers; no hardware endpoints contacted."""
import socketserver,threading,time,http.server
from pathlib import Path
class Echo(socketserver.BaseRequestHandler):
 def handle(self):
  self.request.settimeout(20)
  try:
   while True:
    d=self.request.recv(4096)
    if not d:return
    self.request.sendall(d)
  except (OSError,TimeoutError):pass
class Stall(socketserver.BaseRequestHandler):
 def handle(self):time.sleep(30)
class Server(socketserver.ThreadingTCPServer):allow_reuse_address=True;daemon_threads=True
for port,handler in [(22364,Echo),(22365,Stall)]:
 server=Server(('192.168.1.112',port),handler);threading.Thread(target=server.serve_forever,daemon=True).start()
class HTTP(http.server.SimpleHTTPRequestHandler):
 def __init__(self,*a,**kw):super().__init__(*a,directory='work/network-4b/server',**kw)
http.server.ThreadingHTTPServer(('192.168.1.112',21864),HTTP).serve_forever()
