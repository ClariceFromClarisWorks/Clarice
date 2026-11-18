"""Authenticated Snow acceptance; password supplied privately via environment.
Only loopback port 23231 is used. Never record credential or verifier contents.
"""
import os,time,json,telnetlib
from pathlib import Path
from client import Client
password=os.environ['CLARICE_TEST_PASSWORD']
rows=[]
for attempt in ('wrong','empty','overflow'):
 t=telnetlib.Telnet('127.0.0.1',23231,8)
 # Reuse the actual Telnet negotiation callback without opening another socket.
 t.set_option_negotiation_callback(lambda s,c,o:Client.option(None,s,c,o))
 greeting=t.read_until(b'Password: ',20);assert greeting.endswith(b'Password: ')
 supplied={'wrong':b'deliberately-wrong','empty':b'','overflow':b'x'*140}[attempt]
 t.write(supplied+b'\r\necho AUTH_BYPASS\r\n')
 answer=t.read_until(b'Authentication failed.',60)
 assert b'Authentication failed.' in answer and b'AUTH_BYPASS' not in answer and b' > ' not in answer
 if supplied:assert supplied not in answer
 t.close();time.sleep(6)
 rows.append({'test':attempt,'rejected':True,'no_echo_or_shell':True})
for i in range(3):
 start=time.monotonic();c=Client(password=password);elapsed=time.monotonic()-start
 status,text=c.command('pwd; status runtime');assert status==0 and 'Runtime: heap=' in text
 rows.append({'test':'correct password/reconnect','host_seconds':elapsed,'runtime':text})
 c.t.write(b'exit\r\n');c.t.read_all();c.close();time.sleep(1)
Path('outputs/remote-sessions/auth-snow.json').write_text(json.dumps(rows,indent=2)+'\n')
print('PASS wrong, empty, overlong password rejection; no echoed credentials or pre-auth command; three authenticated reconnects')
