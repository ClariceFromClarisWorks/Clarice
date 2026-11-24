from client import Client
from pathlib import Path
import json,time,subprocess,sys
out=Path('outputs/filesystem-fixes');rows=[]
c=Client(port=23282,password='')
def check(label,source,status=0,contains=None):
 start=time.monotonic();rc,text=c.command(source,180);row=dict(test=label,command=source,status=rc,expected=status,text=text,host_seconds=time.monotonic()-start);rows.append(row);(out/'guest.json').write_text(json.dumps(rows,indent=2)+'\n');print(label,rc,flush=True)
 assert rc==status,(label,rc,text)
 if contains:assert contains in text,(label,text)
def local(source):
 r=Path.cwd();control=r.parent/'Clarice/work/snow-performance/control';keys=r.parent/'Clarice/work/ux-six/keys';pid=Path('work/filesystem-fixes/guest/pid').read_text();p=Path('work/filesystem-fixes/guest/keys');p.write_text(source+'\n');subprocess.run([str(control),pid,'activate'],check=True);time.sleep(.3);subprocess.run([str(keys),pid,str(p),'.08'],check=True);time.sleep(3)
try:
 check('setup',"mkdir '/mnt/FS Audit A/fix1840'; cd '/mnt/FS Audit A/fix1840'; mkdir '/mnt/FS Audit B/fix1840'")
 check('empty operand',"mkdir -p ''",1)
 check('parents and separators','mkdir -p . nested//child/; test -d nested/child')
 check('locked seed',"printf NEW > locked; hfsattrib --locked on locked; printf OLD > target")
 check('locked overwrite','mv locked target',1)
 check('locked preserved',"test \"$(cat target)\" = OLD && test \"$(cat locked)\" = NEW")
 check('locked absent','mv locked absent',1)
 check('no duplicate','test ! -e absent')
 check('locked cross volume',"mv locked '/mnt/FS Audit B/fix1840/absent'",1)
 check('no cross duplicate',"test ! -e '/mnt/FS Audit B/fix1840/absent'")
 check('normal overwrite',"hfsattrib --locked off locked; mv locked target; test ! -e locked && test \"$(cat target)\" = NEW")
 check('fork seed',"cp -p '/mnt/FS Audit A/fork-source' fork")
 check('cross same leaf',"mv fork '/mnt/FS Audit B/fix1840/fork'; test ! -e fork; cmp '/mnt/FS Audit A/fork-source' '/mnt/FS Audit B/fix1840/fork'")
 check('cwd seed','mkdir cwd; cd cwd')
 check('cwd rename','mv ../cwd ../renamed-cwd; pwd',contains='/fix1840/renamed-cwd')
 check('cwd relative','touch child; cd ..; test -f renamed-cwd/child')
 check('ancestor seed','mkdir -p parent/sub; cd parent/sub')
 check('ancestor subshell','(mv ../../parent ../../changed); pwd',contains='/fix1840/changed/sub')
 check('ancestor usable','touch child; cd ../..; test -f changed/sub/child')
 check('same leaf directory move','mkdir holder; cd changed/sub; mv ../../changed ../../holder; pwd',contains='/fix1840/holder/changed/sub')
 check('directory move usable','touch moved-child; cd ../../..; test -f holder/changed/sub/moved-child')
 check('local seed','mkdir -p local/sub')
 local("cd '/mnt/FS Audit A/fix1840/local/sub'")
 check('remote renames local ancestor','mv local local-renamed')
 local('pwd; touch local-child')
 check('local recovered','test -f local-renamed/sub/local-child')
 check('remote help','telnet --help',contains='Usage:')
 check('remote nano diagnostic','nano /tmp/file',2,contains='local input ownership')
 check('remote nano path diagnostic','/bin/nano /tmp/file',2,contains='local input ownership')
 deep="/mnt/FS Audit A/fix1840/bound/a/"+'/'.join(['abcdefghijklmnopqrst']*10)
 check('native bound seed',"mkdir -p '"+deep+"'; cd '"+deep+"'")
 check('reject unusable renamed cwd',"mv '/mnt/FS Audit A/fix1840/bound/a' '/mnt/FS Audit A/fix1840/bound/1234567890123456789012345678901'",1)
 check('bounded cwd preserved',"touch retained; test -f retained; cd '/mnt/FS Audit A/fix1840'")
 check('runtime','status runtime',contains='remaining=0')
 for i in range(4):check('repeat '+str(i),'cp target repeat; cmp target repeat; rm repeat; status runtime',contains='remaining=0')
 # Validate edited command output, independently of echoed line.
 c.t.write(b'echo AC\x1b[DB\r\n');text=c.t.read_until(b' > ',30);assert b'\r\nABC\r\n' in text,text
 c.t.write(b'echo ABX\x1b[D\x7fC\r\n');text2=c.t.read_until(b' > ',30);assert b'\r\nACX\r\n' in text2,text2
 (out/'cursor.json').write_text(json.dumps([text.decode('latin1'),text2.decode('latin1')],indent=2)+'\n')
finally:c.close()
print('PASS',len(rows),'guest checks plus two cursor-edited commands',flush=True)
