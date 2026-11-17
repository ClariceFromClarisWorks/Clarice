"""Disposable Snow cancellation fixture: throttle only /slow.clx."""
import argparse,functools,http.server,time
p=argparse.ArgumentParser();p.add_argument('directory');p.add_argument('--bind',required=True);p.add_argument('--port',type=int,default=8084);a=p.parse_args()
class Handler(http.server.SimpleHTTPRequestHandler):
    def do_GET(self):
        if self.client_address[0]!=a.bind:
            self.send_error(403);return
        if self.path!='/slow.clx': return super().do_GET()
        data=(__import__('pathlib').Path(a.directory)/'slow.clx').read_bytes()
        self.send_response(200);self.send_header('Content-Length',str(len(data)));self.end_headers()
        try:
            for i in range(0,len(data),512):
                self.wfile.write(data[i:i+512]);self.wfile.flush();time.sleep(.15)
        except (BrokenPipeError,ConnectionResetError):pass
http.server.ThreadingHTTPServer((a.bind,a.port),functools.partial(Handler,directory=a.directory)).serve_forever()
