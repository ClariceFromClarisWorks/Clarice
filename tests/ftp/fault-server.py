"""Disposable local FTP EOF/reset fixture. No filesystem access or credentials.
Reject SIZE so the client must distinguish FIN from RST, even with final 226.
Usage: python3 fault-server.py BIND PORT
"""
import socket,struct,sys,threading,time
bind,port=sys.argv[1],int(sys.argv[2])
def serve(c):
 data=None
 try:
  c.settimeout(30);f=c.makefile('rb');c.sendall(b'220 fixture\r\n')
  while True:
   line=f.readline(1024)
   if not line:break
   verb,_,arg=line.decode('ascii').strip().partition(' ')
   if verb=='USER':reply=b'230 anonymous\r\n'
   elif verb=='TYPE':reply=b'200 binary\r\n'
   elif verb=='SIZE':reply=b'502 SIZE unavailable\r\n'
   elif verb=='EPSV':
    data=socket.socket();data.bind((bind,0));data.listen(1);data.settimeout(10)
    reply=f'229 (|||{data.getsockname()[1]}|)\r\n'.encode()
   elif verb=='RETR':
    c.sendall(b'150 data\r\n');d,_=data.accept();data.close();data=None
    d.sendall(bytes(range(256))*32);time.sleep(.25)
    if arg=='reset':d.setsockopt(socket.SOL_SOCKET,socket.SO_LINGER,struct.pack('ii',1,0))
    else:d.shutdown(socket.SHUT_WR)
    d.close();reply=b'226 transfer complete\r\n'
   elif verb=='QUIT':c.sendall(b'221 bye\r\n');break
   else:reply=b'502 unsupported\r\n'
   c.sendall(reply)
 except (OSError,UnicodeError):pass
 finally:
  if data:data.close()
  c.close()
s=socket.socket();s.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1);s.bind((bind,port));s.listen(4)
while True:
 c,_=s.accept();threading.Thread(target=serve,args=(c,),daemon=True).start()
