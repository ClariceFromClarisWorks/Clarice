#!/usr/bin/env python3
"""Disposable local-network fixtures; no file serving, uploads or shell access."""
import argparse,http.server,time
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--bind',required=True);p.add_argument('--port',type=int,default=8094);a=p.parse_args()
data=bytes(range(256))*16
class Handler(http.server.BaseHTTPRequestHandler):
 def do_GET(self):
  if self.client_address[0] not in ('127.0.0.1',a.bind):self.send_error(403);return
  path=self.path
  if path in ('/wget-before.clx','/mktemp-before.clx','/compare.sh','/mktemp-after.clx','/finish.sh'):
   fixture=Path('work/http-upstream')/path[1:]
   if not fixture.is_file():self.send_error(404);return
   payload=fixture.read_bytes();self.send_response(200);self.send_header('Content-Length',str(len(payload)));self.end_headers();self.wfile.write(payload);return
  if path=='/length':
   self.send_response(200);self.send_header('Content-Length',str(len(data)));self.end_headers();self.wfile.write(data);return
  if path=='/404':self.send_error(404);return
  self.protocol_version='HTTP/1.1';self.send_response(200);self.send_header('Transfer-Encoding','chunked')
  if path=='/ambiguous':self.send_header('Content-Length','4096')
  self.end_headers()
  try:
   if path=='/malformed':self.wfile.write(b'xyz\r\n');return
   if path=='/truncated':self.wfile.write(b'100\r\npartial');return
   if path=='/slow':
    for i in range(600):
     self.wfile.write(b'100\r\n'+data[:256]+b'\r\n');self.wfile.flush();time.sleep(.1)
   else:
    for i in range(0,len(data),113):
     block=data[i:i+113];self.wfile.write(('%x;fixture=yes\r\n'%len(block)).encode()+block+b'\r\n')
   self.wfile.write(b'0\r\nX-Fixture: complete\r\n\r\n')
  except (BrokenPipeError,ConnectionResetError):pass
http.server.ThreadingHTTPServer((a.bind,a.port),Handler).serve_forever()
