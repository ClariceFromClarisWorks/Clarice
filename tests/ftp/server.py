"""Disposable Snow fixture only; never point root at personal files.
Requires pyftpdlib 2.2.0. Bind explicitly to a trusted local interface.
"""
import argparse
from pyftpdlib.authorizers import DummyAuthorizer
from pyftpdlib.handlers import FTPHandler, ThrottledDTPHandler
from pyftpdlib.servers import FTPServer
p=argparse.ArgumentParser();p.add_argument('root');p.add_argument('--bind',default='127.0.0.1');p.add_argument('--port',type=int,default=2121);p.add_argument('--slow',action='store_true');p.add_argument('--pasv',action='store_true');a=p.parse_args()
auth=DummyAuthorizer();auth.add_anonymous(a.root,perm='elradfmwMT')
class Handler(FTPHandler):
    def ftp_EPSV(self,line):
        if a.pasv:self.respond('502 EPSV disabled for fixture')
        else:super().ftp_EPSV(line)
Handler.authorizer=auth
if a.slow:
    class Slow(ThrottledDTPHandler):
        read_limit=4096
        write_limit=4096
    Handler.dtp_handler=Slow
server=FTPServer((a.bind,a.port),Handler);server.max_cons=8;server.max_cons_per_ip=8
server.serve_forever()
