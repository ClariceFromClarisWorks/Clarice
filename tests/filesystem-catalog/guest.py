"""Normal68000 Snow, disposable catalog names and outside-root fork/metadata sentinels."""
from pathlib import Path
import sys,json,time,re
sys.path.insert(0,'tests/remote-sessions');from client import Client
out=Path('outputs/filesystem-catalog');start=int(sys.argv[1]) if len(sys.argv)>1 else 0;rows=json.loads((out/'guest.json').read_text())[:start] if start else [];attempt=0;c=Client(port=23260,password='');
if start:c.command('cd /tmp/batch3e')
native='native:Sys6Unix:System Folder:Clarice Data:tmp:batch3e:'
def run(label,cmd,expected=0,contains=None):
 global attempt
 attempt+=1
 if attempt<=start:return
 rc,t=c.command(cmd,240);rows.append(dict(test=label,command=cmd,status=rc,expected=expected,text=t));(out/'guest.json').write_text(json.dumps(rows,indent=2)+'\n');print(label,rc,flush=True);assert rc==expected,t
 if contains is not None:assert contains in t,t
try:
 run('setup','mkdir -p /tmp/batch3e/outer/selected; cd /tmp/batch3e; cp -p /tmp/batch3b/source reference; fsprobe seed reference; cp -p reference outer/sentinel')
 run('literal nested catalog dots','mkdir outer/selected/%2E outer/selected/%2E%2E; cp -p reference outer/selected/%2E/%2E%2E; cp -p reference outer/selected/%2E%2E/%2E')
 leaves=[('%2F','/'),('%25','%'),('mid%2Fname','mid/name'),('tail%2F','tail/'),('%252E','%2E'),('%252E%252E','%2E%2E'),('%252F','%2F'),('%253A','%3A'),('..%252F..','..%2F..')]
 for encoded,raw in leaves:run('literal '+raw,"cp -p reference 'outer/selected/"+encoded+"' && fsprobe same reference '"+native+'outer:selected:'+raw+"'")
 run('link fixture','ln -s /tmp/batch3e/outer outer/selected/link; ln -s nowhere outer/selected/dangling')
 run('navigation unchanged','cd outer/selected; test "$(pwd)" = /tmp/batch3e/outer/selected; cd ..; test "$(pwd)" = /tmp/batch3e/outer; cd ./selected/../..; test "$(pwd)" = /tmp/batch3e')
 run('native dot directory addresses',"fsprobe same reference '"+native+"outer:selected:.:..' && fsprobe same reference '"+native+"outer:selected:..:.'")
 run('wildcard identities',"printf '<%s>\\n' outer/selected/.*",contains='<outer/selected/%2E%2E>')
 run('find literal output','find outer/selected',contains='outer/selected/%2E%2E/%2E')
 run('du literal output','du outer/selected',contains='outer/selected/%2E%2E')
 run('copy nested names','cp -Rp --no-progress outer/selected copied && fsprobe same reference copied/%2E/%2E%2E && fsprobe same reference copied/%2E%2E/%2E && test "$(readlink copied/link)" = /tmp/batch3e/outer')
 for encoded,raw in leaves:run('copied identity '+raw,"fsprobe same reference 'copied/"+encoded+"'")
 run('native directory move','mkdir -p destination; mv copied destination/copied && test ! -e copied && fsprobe same reference destination/copied/%2E/%2E%2E')
 run('cross-volume move','cp -p reference cross%2F; mv cross%2F /mnt/Perf/batch3e-cross%2F && fsprobe cross reference /mnt/Perf/batch3e-cross%2F && test ! -e cross%2F')
 run('reject directory link slash','rm -R outer/selected/link/',1)
 run('outside sentinel intact','fsprobe same reference outer/sentinel')
 run('remove only native dot entry',"rm -R '"+native+"destination:copied:.' && test ! -e destination/copied/%2E && fsprobe same reference destination/copied/%2E%2E/%2E && fsprobe same reference outer/sentinel")
 run('recursive removal confines root','rm -R --no-progress outer/selected && test ! -e outer/selected && fsprobe same reference outer/sentinel')
 run('ordinary trailing slash','rm -R --no-progress destination/// && test ! -e destination && fsprobe same reference outer/sentinel')
 run('missing entry','rm -R missing',1)
 run('failure recovery','fsprobe same reference outer/sentinel && echo RECOVERED && status runtime',contains='remaining=0')
 for repeat in range(3):
  run('repeated nested copy/remove '+str(repeat),'mkdir -p repeat/%2E/%2E%2E; cp -p reference repeat/%2E/%2E%2E/file; cp -Rp --no-progress repeat clone; rm -R --no-progress repeat clone; fsprobe same reference outer/sentinel; status runtime',contains='remaining=0')
 run('cancel seed','cp -Rp --no-progress /mnt/Perf/n100 /mnt/Perf/cancel3e')
 c.t.write(b'rm -R --no-progress /mnt/Perf/cancel3e\r\n');time.sleep(.15);c.t.write(b'\x03');reply=c.t.read_until(b'\r\nbatch3e > ',60);assert reply.endswith(b'\r\nbatch3e > '),reply
 run('cancellation status and recovery','test "$?" -eq 130 && fsprobe same reference outer/sentinel && status runtime',contains='remaining=0')
 run('retry cleanup','rm -Rf --no-progress /mnt/Perf/cancel3e; rm /mnt/Perf/batch3e-cross%2F; fsprobe same reference outer/sentinel; status runtime',contains='remaining=0')
finally:c.close()
print('PASS',len(rows),'normal68000 checks')
