#!/usr/bin/env python3
"""Focused local-only checks for the development HTTP server."""
import functools, http.server, importlib.util, tempfile, threading, urllib.request
from pathlib import Path
spec=importlib.util.spec_from_file_location('dev_http',Path(__file__).resolve().parents[2]/'scripts/dev-http.py')
module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
with tempfile.TemporaryDirectory() as directory:
 root=Path(directory);(root/'public').mkdir();(root/'public'/'binary').write_bytes(bytes(range(256)));(root/'private').write_text('private');(root/'public'/'escape').symlink_to(root/'private')
 server=http.server.ThreadingHTTPServer(('127.0.0.1',0),functools.partial(module.Handler,directory=str(root/'public')));server.peer='127.0.0.1'
 thread=threading.Thread(target=server.serve_forever);thread.start()
 base='http://127.0.0.1:'+str(server.server_port)
 def request(path,code,method='GET'):
  try:r=urllib.request.urlopen(urllib.request.Request(base+path,method=method))
  except urllib.error.HTTPError as e:r=e
  with r:
   assert r.status==code,(path,r.status,code)
   return r.read(),r.headers
 try:
  data,headers=request('/binary',200);assert data==bytes(range(256)) and headers['Content-Length']=='256'
  request('/',404);request('/escape',404);request('/../private',404);request('/binary',405,'HEAD')
  server.peer='192.0.2.1';request('/binary',403)
 finally:server.shutdown();thread.join();server.server_close()
print('PASS: binary Content-Length, no listing, symlink/traversal escape refused, unsupported method, unexpected peer')
