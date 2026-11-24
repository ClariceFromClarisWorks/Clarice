#!/usr/bin/env python3
"""Read-only HTTP development server for the trusted local network.
Serve only an explicit staging directory; HTTP is unauthenticated and unencrypted.
"""
import argparse
import functools
import http.server
from pathlib import Path

class Handler(http.server.SimpleHTTPRequestHandler):
    def do_GET(self):
        if self.server.peer and self.client_address[0] not in self.server.peer:
            self.send_error(403, 'Unexpected peer')
            return
        root = Path(self.directory).resolve()
        target = Path(self.translate_path(self.path)).resolve()
        if not target.is_relative_to(root) or not target.is_file():
            self.send_error(404)
            return
        super().do_GET()

    def do_HEAD(self):
        self.send_error(405)

if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--bind', required=True, help='MacBook trusted-LAN IPv4 address')
    p.add_argument('--peer', action='append', help='Optional client allowlist; omit to allow changing LAN addresses')
    p.add_argument('--port', type=int, default=8080)
    p.add_argument('--directory', type=Path, required=True)
    a = p.parse_args()
    root = a.directory.resolve(strict=True)
    if not root.is_dir():
        p.error('directory must be a staging directory')
    with http.server.ThreadingHTTPServer((a.bind, a.port), functools.partial(Handler, directory=str(root))) as server:
        server.peer = a.peer
        print(f'Serving {root} on {a.bind}:{a.port} for {a.peer}', flush=True)
        try:
            server.serve_forever()
        except KeyboardInterrupt:
            pass
