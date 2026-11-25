"""Normal Snow build; disposable /tmp/batch3a with a preseeded two-fork file."""
from pathlib import Path
import sys,json,time,re
sys.path.insert(0,'tests/remote-sessions');from client import Client
out=Path('outputs/filesystem-p1');out.mkdir(exist_ok=True);rows=[]
c=Client(port=23259,password='')
def check(label,cmd,expected=0,contains=None):
 t=time.monotonic();rc,text=c.command(cmd,180);ok=rc==expected and (contains is None or contains in text)
 rows.append(dict(test=label,command=cmd,status=rc,expected=expected,passed=ok,text=text,host_seconds=time.monotonic()-t));(out/'guest.json').write_text(json.dumps(rows,indent=2)+'\n');print('PASS' if ok else 'FAIL',label,rc,flush=True);assert ok,text
try:
 check('disposable workspace','cd /tmp/batch3a')
 check('encoded filenames and fork copy',"cp -p fork-source 'percent%25slash%2Fname'; cmp fork-source 'percent%25slash%2Fname'")
 for tool in ['cp','mv']:
  check(tool+' seed','cp -p fork-source source')
  for n in [511,512,600]:
   prefix='/tmp/batch3a/';leaf=tool+'%25%2Ftarget';pad=n-len(prefix)-len(leaf);target=prefix+'./'*(pad//2)+'/'*(pad%2)+leaf;assert len(target)==n
   check(tool+' encoded destination '+str(n),tool+" -p source '"+target+"'" if tool=='cp' else tool+" source '"+target+"'",0 if n==511 else 1)
   if n==511:
    check(tool+' boundary target contents',"cmp fork-source '"+leaf+"'")
    check(tool+' reseed after valid transfer','cp -p fork-source source')
   else:check(tool+' rejected transfer preserves source and destination',"cmp fork-source source && cmp fork-source '"+leaf+"'")
 check('directory operand','mkdir targets; cp -p source targets/; cmp source targets/source; mv targets/source targets/moved; cmp source targets/moved')
 check('symlink seed','mkdir -p target/sub; cp -p fork-source target/sub/sentinel; ln -s target dirlink')
 for operand in ['dirlink/','dirlink///','dirlink/.///']:
  check('reject '+operand,"rm -R '"+operand+"'",1)
  check('target and link intact '+operand,'cmp fork-source target/sub/sentinel; test "$(readlink dirlink)" = target')
 check('force cannot bypass rejection','rm -Rf dirlink/',1)
 check('unlink final symlink','rm -R dirlink; test ! -e dirlink && cmp fork-source target/sub/sentinel')
 check('dot slash seed','mkdir -p dot/sub; cp -p fork-source dot/sub/sentinel')
 check('dot slash rejection','rm -R dot/.///',1)
 check('dot contents intact','cmp fork-source dot/sub/sentinel')
 check('ordinary trailing slash with contained link','mkdir ordinary; ln -s ../target ordinary/childlink; rm -R ordinary///; test ! -e ordinary && cmp fork-source target/sub/sentinel')
 check('normal recursive removal','mkdir -p ordinary/sub; printf value > ordinary/sub/file; rm -R ordinary/; test ! -e ordinary')
 check('error and recovery','cp source missing/parent/file',1)
 check('fork and metadata after errors','hfsattrib source; cmp source fork-source',contains="type='TEXT' creator='TEST'")
 check('warm command data','cp -p source warm; rm warm; status runtime',contains='remaining=0')
 for n in range(3):
  check('repeated cp/rm cleanup '+str(n),'i=0; while test $i -lt 20; do cp -p source repeat || break; rm repeat || break; i=$((i+1)); done; test $i -eq 20; status runtime',contains='remaining=0')
 check('cancellation source','dd if=source of=large bs=1024 seek=2048 count=1')
 check('cancellation destination','cp -p fork-source cancel-target')
 c.t.write(b'cp large cancel-target\r\n');time.sleep(.2);c.t.get_socket().sendall(bytes([255,244]));reply=c.t.read_until(b' > ',60).decode('latin1');assert reply.endswith(' > '),reply
 check('cancel status and recovery','echo $?; cmp fork-source cancel-target; status runtime',contains='130\r\n')
 check('cleanup large scratch','rm large; status runtime',contains='remaining=0')
finally:c.close()
print('PASS',len(rows),'guest checks')
