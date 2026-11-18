import os
from client import Client
import socket,time,json,subprocess,sys
from pathlib import Path
pid=sys.argv[1];out=Path(os.environ.get('CLARICE_REMOTE_OUTPUT', 'outputs/remote-sessions'));rows=[]
c=Client()
try:
 # A second TCP connection must not acquire another shell or run commands.
 s=socket.create_connection(('127.0.0.1',23231),3);s.settimeout(2);s.sendall(b'echo SECOND_CLIENT\r\n')
 try:data=s.recv(1024)
 except (socket.timeout,ConnectionResetError):data=b''
 s.close();assert b'Clarice remote shell' not in data and b' > ' not in data;rows.append(dict(test='second client rejected',received=data.decode('latin1')))
 assert c.command('echo FIRST_CLIENT_STILL_WORKS')[0]==0
 # Raw Ctrl+C, not only IAC IP, cancels execution and preserves the session.
 c.t.write(b'while true; do :; done\r\n');time.sleep(.2);c.t.write(b'\x03')
 data=c.t.read_until(b' > ',10);assert b'^C' in data,data
 assert c.command('echo AFTER_ETX')[0]==0;rows.append(dict(test='ETX cancellation',passed=True))
 # Local Command-period cancels local editing, not the remote command.
 c.t.write(b'while true; do :; done\r\n');time.sleep(.3)
 control='/Users/williamprestwich/Documents/ChatGPT/Clarice/work/snow-performance/control'
 subprocess.run([control,pid,'activate'],check=True);time.sleep(.5)
 subprocess.run([control,pid,'chord','55','47'],check=True)
 time.sleep(.2);data=c.t.read_very_eager();assert b' > ' not in data,data
 c.t.get_socket().sendall(bytes([255,244]));data+=c.t.read_until(b' > ',10)
 assert b'^C' in data;assert c.command('echo REMOTE_SURVIVED_LOCAL_CANCEL')[0]==0
 rows.append(dict(test='local cancel independent',passed=True))
 # Disconnect during running output must unwind, then accept a new client.
 c.t.write(b'while true; do echo output; done\r\n');time.sleep(.3)
finally:c.close()
time.sleep(.8);c=Client();assert c.command('echo AFTER_DISCONNECT')[0]==0;c.close();rows.append(dict(test='disconnect during output and reconnect',passed=True))
# A parent exit closes only the network session; the listener is reusable.
time.sleep(.3);c=Client();assert c.command('(exit 3); echo PARENT_ALIVE')[0]==0
c.t.write(b'exit\r\n');time.sleep(.3);c.close();time.sleep(.5)
c=Client();assert c.command('echo AFTER_LOGOUT')[0]==0;c.close();rows.append(dict(test='subshell exit and logout',passed=True))
(out/'lifecycle.json').write_text(json.dumps(rows,indent=2)+'\n');print('PASS',len(rows),'guest lifecycle checks')
