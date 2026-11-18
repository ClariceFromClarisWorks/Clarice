"""Safe disposable guest config/heap acceptance. No physical targets."""
import os,sys,time,json,subprocess,telnetlib,re
from pathlib import Path
from client import Client
pid=sys.argv[1];password=os.environ['CLARICE_TEST_PASSWORD'];rows=[]
def local(command):
 p=Path('/tmp/remote-auth-command.txt');p.write_text(command+'\n')
 subprocess.run(['/tmp/deploy-pairkeys',pid,str(p),'0.04'],check=True);time.sleep(2)
c=Client();assert c.command('cp /etc/clarice/telnet.conf /tmp/telnet-current.conf; printf BAD > /etc/clarice/telnet.conf')[0]==0;c.close()
local('remote off; telnetd; echo AUTH_CONFIG_STATUS:$?')
t=telnetlib.Telnet('127.0.0.1',23231,8)
try: data=t.read_until(b'Password: ',3)
except EOFError:data=b''
t.close();assert b'Password:' not in data and b' > ' not in data;rows.append({'test':'malformed config refuses startup','passed':True})
local('cp /tmp/telnet-current.conf /etc/clarice/telnet.conf; telnetd')
c=Client();assert c.command('cmp /tmp/telnet-current.conf /etc/clarice/telnet.conf')[0]==0;c.close();rows.append({'test':'config recovery and password','passed':True})
# Stop/re-enable cycles must release both native and application storage.
local('remote off; for i in 1 2 3; do telnetd; remote off; status runtime; done > /tmp/auth-memory.txt; telnetd')
c=Client();status,text=c.command('cat /tmp/auth-memory.txt');assert status==0
heaps=re.findall(r'heap=(\d+) largest=(\d+)',text);assert len(heaps)==3,heaps
# Native heap has the pre-existing reversible two-byte alignment variation.
assert heaps[0]==heaps[-1] and len({v[1] for v in heaps})==1 and max(int(v[0]) for v in heaps)-min(int(v[0]) for v in heaps)<=2,heaps
rows.append({'test':'three enable/stop cycles','heap_and_largest':heaps});c.close()
# Ctrl+C before authentication must never turn into shell input.
t=telnetlib.Telnet('127.0.0.1',23231,8);t.set_option_negotiation_callback(lambda s,c,o:Client.option(None,s,c,o));assert t.read_until(b'Password: ',20).endswith(b'Password: ')
t.write(password[:4].encode()+b'\x03');data=t.read_all();t.close();assert password[:4].encode() not in data and b' > ' not in data
rows.append({'test':'pre-auth cancellation','passed':True});time.sleep(1)
c=Client();assert c.command('echo RECOVERED')[0]==0;c.close()
Path('outputs/remote-sessions/auth-recovery.json').write_text(json.dumps(rows,indent=2)+'\n');print('PASS malformed config, recovery, three native heap cycles and pre-auth cancellation')
