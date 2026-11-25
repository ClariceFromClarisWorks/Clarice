"""Explicit disposable Snow guest only. Normal utilities plus labelled probes."""
import sys,json,time,re,argparse
from pathlib import Path
sys.path.insert(0,'tests/remote-sessions');from client import Client
p=argparse.ArgumentParser();p.add_argument('--port',type=int,required=True);a=p.parse_args();c=Client(port=a.port,password='');out=Path('outputs/network-4b/snow.json');rows=[];host='192.168.1.112'
def run(label,cmd,expected=0,contains=None):
 start=time.monotonic();rc,text=c.command(cmd,120);ok=rc==expected and (contains is None or contains in text);rows.append(dict(test=label,command=cmd,status=rc,passed=ok,text=text,host_seconds=time.monotonic()-start));out.write_text(json.dumps(rows,indent=2)+'\n');assert ok,text;print('PASS',label,flush=True);return text
try:
 run('fixture','mkdir -p /tmp/net4b-final; cd /tmp/net4b-final')
 run('HTTP binary',f'wget -O binary http://{host}:21864/binary.dat')
 run('FTP binary roundtrip',f'ftp -P 21264 put {host} binary roundtrip4b.dat && ftp -P 21264 get {host} roundtrip4b.dat returned && cmp binary returned')
 assert Path('work/network-4b/server/roundtrip4b.dat').read_bytes()==Path('work/network-4b/server/binary.dat').read_bytes()
 run('FTP unknown-size orderly EOF',f'ftp -P 21265 get {host} normal unknown && cmp binary unknown')
 run('FTP empty',f'ftp -P 21264 get {host} empty.dat empty')
 run('two live native streams',f'tcp-probe {host} 22364 3',contains='system-delta=0')
 run('native echo probe',f'tcp-probe {host} 22364 0',contains='owned=0')
 run('three failed native releases then cleanup',f'tcp-probe {host} 22364 1',1,'system-delta=0')
 # The cancellation probe returns130 without setting the shell cancellation flag.
 c.t.write(f'tcp-probe {host} 22365 2\r\n'.encode())
 reply=c.t.read_until(b' > ',60).decode('latin1');assert 'system-delta=0' in reply,reply
 rows.append(dict(test='pending receive, three failed aborts, cancellation',text=reply))
 run('probe cancellation status','test "$?" -eq 130')
 run('native receive timeout and cleanup',f'tcp-probe {host} 22365 0',1,'owned=0')
 run('warm heap','netheap2; status runtime',contains='remaining=0')
 before=run('heap before repeated transfers','netheap2')
 for i in range(10):run('repeat '+str(i),f'ftp -P 21264 get {host} binary.dat repeat && cmp binary repeat && rm repeat && status runtime',contains='remaining=0')
 after=run('heap after repeated transfers','netheap2')
 def heaps(s):return re.search(r'app=(\d+) largest=(\d+) system=(\d+) largest=(\d+)',s).groups()
 assert heaps(before)==heaps(after),(heaps(before),heaps(after))
 for mode in ['Telnet IP','remote Ctrl+C']:
  c.t.write(f'ftp -P 21266 get {host} large.dat cancelled\r\n'.encode());time.sleep(.35);start=time.monotonic()
  if mode=='Telnet IP':c.t.get_socket().sendall(bytes([255,244]))
  else:c.t.write(b'\x03')
  reply=c.t.read_until(b' > ',60).decode('latin1');assert reply.endswith(' > '),reply
  rows.append(dict(test=mode+' prompt recovery',text=reply,host_seconds=time.monotonic()-start));out.write_text(json.dumps(rows,indent=2)+'\n')
  run(mode+' status130 and staging cleanup','test "$?" -eq 130 && test ! -e cancelled && test -z "$(find . -name \'.ftp-*\')" && status runtime',contains='remaining=0')
 run('HTTP recovery',f'wget -O recovered http://{host}:21864/binary.dat && cmp binary recovered')
 run('final cleanup','netheap2; status runtime',contains='remaining=0')
finally:c.close()
