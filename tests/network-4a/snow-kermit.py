"""Native Kermit guest probe + actual modem PTY, controlled over localhost.
The test-only CLX selects native transport instead of Telnet's raw-wire mode.
"""
from pathlib import Path
import sys,subprocess,time,json,argparse,signal
p=argparse.ArgumentParser();p.add_argument('--port',type=int,required=True);p.add_argument('--device',required=True);a=p.parse_args()
sys.path.insert(0,'tests/remote-sessions');from client import Client
w=Path('work/network-4a');o=Path('outputs/network-4a');rows=[];c=Client(port=a.port,password='')
def transfer(name,verb,path,guest,cancel=False):
 with (o/(name+'-peer.txt')).open('w') as log:
  peer=subprocess.Popen(['work/kermit/kermit','-d',a.device,verb,str(path)],stdout=log,stderr=subprocess.STDOUT)
  try:
   if cancel:
    c.t.write((guest+'\r\n').encode());time.sleep(.4);c.t.get_socket().sendall(bytes([255,244]));text=c.t.read_until(b' > ',30).decode('latin1');assert text.endswith(' > '),text
    check,st=c.command('test \"$?\" -eq 130');assert check==0,st;status=130
   else:status,text=c.command(guest,45)
   rc=peer.wait(timeout=15)
   _,heap=c.command('netheap2; status runtime',30)
   rows.append(dict(test=name,peer_status=rc,status=status,guest=text,heap=heap));(o/'kermit-snow.json').write_text(json.dumps(rows,indent=2)+'\n')
   assert rc!=0 if cancel else rc==0,(name,rc,text)
   assert status==(130 if cancel else 0),text
   assert 'remaining=0' in heap,heap
  finally:
   if peer.poll() is None:peer.send_signal(signal.SIGINT);peer.wait(timeout=12)
 print('PASS',name,flush=True)
try:
 assert c.command('rm -f /tmp/net4a/krecv /tmp/net4a/kcancel /tmp/net4a/kempty')[0]==0
 source=w/'server/binary.dat';large=w/'server/large.dat'
 transfer('kermit-receive','send',source,'kermit-native receive /tmp/net4a/krecv')
 (w/'kroundtrip.dat').unlink(missing_ok=True)
 transfer('kermit-send','receive',w/'kroundtrip.dat','kermit-native send /tmp/net4a/krecv')
 assert (w/'kroundtrip.dat').read_bytes()==source.read_bytes()
 transfer('kermit-cancel','send',large,'kermit-native receive /tmp/net4a/kcancel',True)
 assert c.command('test ! -e /tmp/net4a/kcancel')[0]==0
 transfer('kermit-recovery','send',source,'kermit-native receive /tmp/net4a/kcancel')
 assert c.command('cmp /tmp/net4a/krecv /tmp/net4a/kcancel')[0]==0
 transfer('kermit-empty','send',w/'server/empty.dat','kermit-native receive /tmp/net4a/kempty')
 assert c.command('test -z "$(find /tmp/net4a -name \'.krm-*\')"')[0]==0
finally:c.close()
print('PASS Kermit native binary roundtrip, cancellation130, cleanup/recovery, empty file')
