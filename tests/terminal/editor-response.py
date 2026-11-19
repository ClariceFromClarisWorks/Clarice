"""Real local vi input via existing CGEvent/Snow keys, results via Telnet.
Run from repo root: python3 tests/terminal/editor-response.py PID vertical old [LABEL]
Requires the isolated fixture, editor-keys binary, vi-lines and diagnostic Core.
"""
import sys,subprocess,time,json,re,os
from pathlib import Path
sys.path.insert(0,'tests/remote-sessions')
from client import Client
PID=int(sys.argv[1]);case=sys.argv[2];mode=sys.argv[3]
label=('-'+sys.argv[4]) if len(sys.argv)>4 else ''
def keys(s,delay=.04):
 p=Path('work/editor-snow/case.keys');p.write_bytes(s.encode('latin1'));subprocess.run(['work/editor-snow/editor-keys',str(PID),str(p),str(delay)],check=True,stdout=subprocess.DEVNULL)
app=os.getenv('VI_APP','vi');extra=' 2>/tmp/vi-prof.txt' if app!='vi' else ''
keys('scrollbench editor-'+mode+'\r'+app+' /tmp/vi-lines'+extra+'\r');time.sleep(2)
if case=='vertical':keys('jk'*60,.06)
elif case=='horizontal':keys('lh'*60,.06)
elif case=='directions':keys('ljhk'*30,.06)
elif case=='arrows':keys(chr(31)*20+chr(30)*20,.06)
elif case=='arrows-repeat':keys((chr(31)+chr(30))*60,.06)
elif case=='scroll':keys('j'*65+'k'*65,.18)
elif case=='count':keys('20j20k'*10,.06)
elif case=='page':keys((chr(6)+chr(2))*10,.24)
elif case=='burst':keys('jk'*60,.005)
keys(':q\r');time.sleep(.6)
c=Client(port=23250,password='');status,out=c.command('status runtime');
if app!='vi':
 status,prof=c.command('cat /tmp/vi-prof.txt');out+=prof
c.close();print(out)
Path('outputs/editor-response/'+mode+'-'+case+label+'.txt').write_text(out)
