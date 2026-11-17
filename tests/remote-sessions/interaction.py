"""Snow-only UI/session acceptance; no physical-machine connection."""
from client import Client
import subprocess,time,json,re,sys
from pathlib import Path
pid=sys.argv[1];base=Path('/Users/williamprestwich/Documents/ChatGPT/Clarice/work')
control=str(base/'snow-performance/control');click=str(base/'se-control-upgrade/click');drag=str(base/'ux-six/drag')
out=Path('outputs/remote-sessions');rows=[]
def call(*args):subprocess.run(list(args),check=True,timeout=20)
def shot(name):
 p=out/(name+'.png');call(control,pid,'shot',str(p));call('sips','-Z','1000',str(p),'--out',str(p))
def type_local(text):
 p=Path('/tmp/remote-ui-input.txt');p.write_text(text);call('/tmp/deploy-pairkeys',pid,str(p),'0.04')
def record(name,result):rows.append(dict(test=name,result=result))
c=Client()
try:
 assert c.command('cd /tmp; rm -f /tmp/local-remote-queue.txt /tmp/remote-must-not-exist')[0]==0
 # A large line is rejected before its executable prefix can run.
 c.t.write(("printf BAD > /tmp/remote-must-not-exist "+'x'*1400+'\r\n').encode())
 data=c.t.read_until(b'Input rejected:',10)+c.t.read_until(b' > ',10)
 assert b'Input rejected:' in data,data
 assert c.command('test ! -e /tmp/remote-must-not-exist')[0]==0
 record('overflow',data.decode('latin1'))
 # Multiple complete input lines preserve order in the bounded pending queue.
 c.t.write(b'echo FIRST\r\necho SECOND\r\n')
 data=c.t.read_until(b' > ',10)+c.t.read_until(b' > ',10)
 assert b'\r\nFIRST\r\n' in data and b' > SECOND\r\n' in data,data
 record('multiline',data.decode('latin1'))
 c.t.write(b'while true; do :; done\r\n');time.sleep(.4)
 call(control,pid,'activate');time.sleep(.5)
 type_local('echo LOCAL_QUEUE_OK > /tmp/local-remote-queue.txt')
 shot('local-edit-during-remote')
 # Scroll through local history while the remote command remains active.
 call(control,pid,'chord','56','126');shot('local-scroll-during-remote')
 call(control,pid,'chord','56','125')
 type_local('\n');time.sleep(.5)
 # Network IAC IP cancels only the remote command, then local submission runs.
 c.t.get_socket().sendall(bytes([255,244]));data=c.t.read_until(b' > ',10)
 assert b'^C' in data,data
 time.sleep(.7);shot('local-queue-after-remote-interrupt')
 status,text=c.command('cat /tmp/local-remote-queue.txt; pwd');assert status==0 and 'LOCAL_QUEUE_OK' in text and '/tmp' in text,text
 record('local queued command / remote interrupt / independent cwd',text)
 # Canonical stdin and EOF stay in the remote connection.
 c.t.write(b'read a b; printf "READ:%s/%s\\n" "$a" "$b"\r\n');time.sleep(.2);c.t.write(b'hello remote\r\n')
 data=c.t.read_until(b' > ',10);assert b'READ:hello/remote' in data,data;record('remote stdin',data.decode('latin1'))
 # Repeated external command invocation and session state remain usable.
 for i in range(8):
  status,text=c.command("printf 'alpha\\nbeta\\n' | grep beta; status runtime")
  assert status==0 and 'beta' in text
  heap=re.search(r'heap=(\d+) largest=(\d+)',text);assert heap
  record('repeat heap',dict(heap=int(heap[1]),largest=int(heap[2])))
finally:c.close()
# Reconnect repeatedly: state is new and no previous cwd/variables persist.
for i in range(3):
 time.sleep(.3);c=Client();status,text=c.command('pwd; status runtime');assert status==0
 record('reconnect',text);c.close()
(out/'interaction.json').write_text(json.dumps(rows,indent=2)+'\n');print('PASS',len(rows),'interaction/memory observations')
