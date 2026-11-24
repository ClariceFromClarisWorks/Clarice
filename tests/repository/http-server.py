"""The LAN listener accepts changing client addresses; explicit restrictions remain optional."""
import functools
import http.server
import importlib.util
from pathlib import Path
import tempfile
import threading
import urllib.request
import urllib.error
spec=importlib.util.spec_from_file_location('dev_http',Path(__file__).resolve().parents[2]/'scripts/dev-http.py')
module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
with tempfile.TemporaryDirectory() as tmp:
    Path(tmp,'index.txt').write_text('fixture')
    with http.server.ThreadingHTTPServer(('127.0.0.1',0),functools.partial(module.Handler,directory=tmp)) as server:
        server.peer=None
        thread=threading.Thread(target=server.serve_forever);thread.start()
        base=f'http://127.0.0.1:{server.server_port}/'
        try:
            assert urllib.request.urlopen(base+'index.txt').read()==b'fixture'
            server.peer=['192.0.2.1']
            try: urllib.request.urlopen(base+'index.txt')
            except urllib.error.HTTPError as e: assert e.code==403
            else: raise AssertionError('explicit restriction ignored')
            server.peer=None
            assert urllib.request.urlopen(base+'index.txt').status==200
            try: urllib.request.urlopen(base)
            except urllib.error.HTTPError as e: assert e.code==404
            else: raise AssertionError('directory listing exposed')
        finally:
            server.shutdown();thread.join()
print('PASS unrestricted LAN mode, optional allowlist, no directory listing')
