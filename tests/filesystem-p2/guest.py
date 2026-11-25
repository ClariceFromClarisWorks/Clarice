"""Disposable Snow HFS acceptance. Requires fsprobe and seeded valid resource file."""
from pathlib import Path
import sys,json,time
sys.path.insert(0,'tests/remote-sessions');from client import Client
out=Path('outputs/filesystem-p2');out.mkdir(exist_ok=True);rows=[];c=Client(port=23259,password='')
def run(label,s,expected=0,contains=None):
 rc,text=c.command(s,240);rows.append(dict(test=label,command=s,status=rc,expected=expected,text=text));(out/'guest.json').write_text(json.dumps(rows,indent=2)+'\n');print(rc,label,flush=True);assert rc==expected and (contains is None or contains in text),text
try:
 run('setup','cd /tmp/batch3b; fsprobe seed source; mkdir -p target/sub same destination; cp -p source target/sub/sentinel; ln -s source filelink; ln -s target dirlink; ln -s absent missinglink')
 for name in ['filelink','dirlink','missinglink']:
  run('reject slash '+name,'fsprobe unlink '+name+'/',1)
  run('link still exists '+name,'readlink '+name)
  run('unlink only link '+name,'fsprobe unlink '+name)
 run('targets intact','cmp source target/sub/sentinel && test -d target && test ! -e missinglink && test ! -e filelink && test ! -e dirlink')
 run('ordinary file and empty directory cleanup','printf TEMP > tempfile; fsprobe unlink tempfile; mkdir empty; fsprobe unlink empty/; test ! -e tempfile && test ! -e empty')
 run('missing entry failure','fsprobe unlink absent',1)
 run('native link operand','ln -s source filelink; fsprobe unlink "native:Sys6Unix:System Folder:Clarice Data:tmp:batch3b:filelink:"',1)
 run('native no-follow removal','fsprobe unlink "native:Sys6Unix:System Folder:Clarice Data:tmp:batch3b:filelink"; test -f source')
 run('same-volume fallback seed','cp -p source same/start; fsprobe seed same/start')
 run('same-volume parent and leaf change','mv same/start destination/end && test ! -e same/start && fsprobe same source destination/end')
 run('cross-volume seed','cp -p source cross; fsprobe seed cross')
 run('cross-volume move','mv cross /mnt/Batch3B/moved && test ! -e cross && fsprobe cross source /mnt/Batch3B/moved')
 run('two-fork large input','cp -p source big; dd if=source of=big bs=1024 seek=2048 count=1; cp -p big big-ref')
 run('failure destination seed','cp -p source /mnt/Batch3B/target')
 run('fill disposable destination','dd if=source of=/mnt/Batch3B/filler bs=1024 seek=65536 count=1',1)
 run('disk-full move fails','mv big /mnt/Batch3B/target',1)
 run('disk-full preserves source and old destination','fsprobe same big big-ref && fsprobe cross source /mnt/Batch3B/target')
 run('recover space','rm /mnt/Batch3B/filler')
 c.t.write(b'mv big /mnt/Batch3B/cancelled\r\n');time.sleep(.2);c.t.get_socket().sendall(bytes([255,244]));assert c.t.read_until(b' > ',60).endswith(b' > ')
 run('cancelled move recovery','test "$?" -eq 130 && test -f big && test ! -e /mnt/Batch3B/cancelled && fsprobe same big big-ref && status runtime',contains='remaining=0')
 run('subsequent successful move','cp -p source retry; mv retry /mnt/Batch3B/retried && fsprobe cross source /mnt/Batch3B/retried')
 for n in range(3):run('repeated cleanup '+str(n),'i=0; while test $i -lt 15; do cp -p source repeat || break; fsprobe unlink repeat || break; i=$((i+1)); done; test $i -eq 15 && status runtime',contains='remaining=0')
 run('cleanup large scratch','rm big big-ref; status runtime',contains='remaining=0')
finally:c.close()
print('PASS',len(rows),'guest checks')
