"""Run against an explicitly selected disposable Snow guest and FTP fixtures."""
from pathlib import Path
import argparse,json,sys,time,re
p=argparse.ArgumentParser();p.add_argument('--port',type=int,required=True);p.add_argument('--pid',required=True);p.add_argument('--host',required=True);a=p.parse_args()
sys.path.insert(0,'tests/remote-sessions');from client import Client
c=Client(port=a.port,password='');rows=[];out=Path('outputs/network-4a');root=Path('work/network-4a/server');h=a.host
def run(label,cmd,expected=0,contains=None):
 t=time.monotonic();rc,text=c.command(cmd,120);ok=rc==expected and (contains is None or contains in text)
 rows.append(dict(test=label,command=cmd,status=rc,passed=ok,text=text,host_seconds=time.monotonic()-t));(out/'ftp-snow.json').write_text(json.dumps(rows,indent=2)+'\n');assert ok,text;print('PASS',label,flush=True);return text
try:
 run('fixture','cd /tmp/net4a')
 run('binary put',f'ftp -P 21264 put {h} binary roundtrip.dat')
 assert (root/'roundtrip.dat').read_bytes()==(root/'binary.dat').read_bytes()
 run('known empty EOF',f'ftp -P 21264 get {h} empty.dat empty')
 run('unknown EOF',f'ftp -P 21265 get {h} normal unknown2; cmp binary unknown2')
 run('native TCP failure after data despite final226',f'ftp-error -P 21265 get {h} normal bad',1,'-23012')
 run('failure retains no target or staging','test ! -e bad && test -z "$(find . -name \'.ftp-*\')"')
 run('warm heap','netheap2; status runtime',contains='remaining=0')
 before=run('heap before repeated transfers','netheap2')
 for i in range(10):
  run('repeat '+str(i),f'ftp -P 21264 get {h} binary.dat repeat && cmp binary repeat && rm repeat && status runtime',contains='remaining=0')
 after=run('heap after repeated transfers','netheap2')
 def heaps(s):return tuple(map(int,re.search(r'app=(\d+) largest=(\d+) system=(\d+) largest=(\d+)',s).groups()))
 assert heaps(before)==heaps(after),(heaps(before),heaps(after))
 for mode in ['Telnet IP','remote Ctrl+C']:
  c.t.write(f'ftp -P 21266 get {h} large.dat cancelled\r\n'.encode());time.sleep(.35)
  t=time.monotonic()
  if mode=='Telnet IP':c.t.get_socket().sendall(bytes([255,244]))
  else:
   c.t.write(b'\x03')
  reply=c.t.read_until(b' > ',60).decode('latin1');assert reply.endswith(' > '),reply
  rows.append(dict(test=mode+' prompt recovery',text=reply,host_seconds=time.monotonic()-t))
  run(mode+' status130 and clean staging','test "$?" -eq 130 && test ! -e cancelled && test -z "$(find . -name \'.ftp-*\')" && status runtime',contains='remaining=0')
 run('recovery download',f'ftp -P 21264 get {h} binary.dat recovered && cmp binary recovered')
 run('final heap','netheap2; status runtime',contains='remaining=0')
finally:c.close()
print('PASS guest binary, empty/unknown EOF, injected error, repeated heaps, cancellation/recovery')
