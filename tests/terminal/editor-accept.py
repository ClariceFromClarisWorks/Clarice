"""Final local editor save/cancel/order checks; disposable isolated Snow only."""
import sys,time,subprocess,json,os
from pathlib import Path
sys.path.insert(0,'tests/remote-sessions');from client import Client
pid=sys.argv[1];control=sys.argv[2];rows=[]
app=os.environ.get("VI_APP","vi-final")
def keys(s,delay=.04):
 p=Path('work/editor-snow/accept.keys');p.write_bytes(s.encode('latin1'));subprocess.run(['work/editor-snow/editor-keys',pid,str(p),str(delay)],check=True,stdout=subprocess.DEVNULL)
def esc():subprocess.run([control,pid,'chord','59','33'],check=True,stdout=subprocess.DEVNULL)
def cmd(s):
 c=Client(port=23250,password='');rc,out=c.command(s,60);c.close();time.sleep(.5);rows.append(dict(command=s,status=rc,text=out));assert rc==0,(s,rc,out);return out
# Atomic fixture copy, two movement forms must land on precisely the same line.
original=Path('work/editor-snow/http/lines.txt').read_text()
for name,motion in [('count','20j'),('arrows',chr(31)*20),('burst','jk'*60)]:
 path='/tmp/vi-accept-'+name;cmd('cp /tmp/vi-lines '+path)
 keys(app+' '+path+'\r');time.sleep(2);keys(motion,.005 if name=='burst' else .04);keys('iMARK');esc();keys(':wq\r');time.sleep(1)
 lines=original.splitlines(True);line=0 if name=='burst' else 20;lines[line]='MARK'+lines[line]
 assert ''.join(lines).replace('\n','\r\n') in cmd('cat '+path)
 cmd('status runtime')
# Ctrl-C belongs to vi; Command-period cancels the invocation and restores shell.
keys(app+' /tmp/vi-lines\r');time.sleep(2)
subprocess.run([control,pid,'chord','59','8'],check=True,stdout=subprocess.DEVNULL)
keys('ljhk:q\r');time.sleep(1);cmd('status runtime')
keys(app+' /tmp/vi-lines\r');time.sleep(2)
subprocess.run([control,pid,'chord','55','47'],check=True,stdout=subprocess.DEVNULL);time.sleep(1)
keys('echo $? > /tmp/editor-cancel-status\r');time.sleep(.7)
assert '\r\n130\r\n' in cmd('cat /tmp/editor-cancel-status')
cmd('status runtime');assert '\r\nRECOVERED\r\n' in cmd('echo RECOVERED')
Path(os.environ.get('EDITOR_ACCEPT_OUTPUT','outputs/editor-response/normal-acceptance.json')).write_text(json.dumps(rows,indent=2)+'\n')
print('PASS normal builds: 20j/20 arrows exact line; 120-key burst; saves; Ctrl-C ownership; Command-period 130; shell recovery')
