"""Normal candidate utilities on disposable HFS fixtures; no hardware addresses."""
from pathlib import Path
import sys,json,time,re
sys.path.insert(0,'tests/remote-sessions');from client import Client
c=Client(port=23260,password='');p=Path('outputs/filesystem-perf/acceptance.json');rows=[]
def run(name,s,expected=0):
 rc,t=c.command(s,240);rows.append(dict(test=name,command=s,status=rc,text=t));p.write_text(json.dumps(rows,indent=2)+'\n');print(rc,name,flush=True);assert rc==expected,t
try:
 run('seed','mkdir -p /tmp/traverse-check/sub; echo KEEP > /tmp/traverse-out; cp -p /tmp/batch3b/source /tmp/traverse-check/sub/slash%2F%25; ln -s /tmp/traverse-out /tmp/traverse-check/link; ln -s /tmp/absent /tmp/traverse-check/dangling')
 for args in ['','-type f','-type l','-maxdepth 0','-maxdepth 1','-name "[s]*"']:
  run('find output '+args,f'/mnt/Perf/before/find /tmp/traverse-check {args} > /tmp/find-before; /mnt/Perf/after/find /tmp/traverse-check {args} > /tmp/find-after; cmp /tmp/find-before /tmp/find-after')
 for args in ['','-a','-s','-h','-k']:
  run('du output '+args,f'/mnt/Perf/before/du {args} /tmp/traverse-check > /tmp/du-before; /mnt/Perf/after/du {args} /tmp/traverse-check > /tmp/du-after; cmp /tmp/du-before /tmp/du-after')
 for tool in ('find','du'):
  run('root file '+tool,f'/mnt/Perf/before/{tool} /tmp/traverse-out > /tmp/leaf-before; /mnt/Perf/after/{tool} /tmp/traverse-out > /tmp/leaf-after; cmp /tmp/leaf-before /tmp/leaf-after')
  run('missing path '+tool,f'/mnt/Perf/after/{tool} /tmp/missing-traverse',1)
 run('recursive copy two forks and symlinks','/mnt/Perf/after/cp -Rp --no-progress /tmp/traverse-check /mnt/Perf/check && fsprobe cross /tmp/traverse-check/sub/slash%2F%25 /mnt/Perf/check/sub/slash%2F%25 && test "$(readlink /mnt/Perf/check/link)" = /tmp/traverse-out && test "$(readlink /mnt/Perf/check/dangling)" = /tmp/absent')
 run('safe recursive removal','/mnt/Perf/after/rm -R --no-progress /mnt/Perf/check && test ! -e /mnt/Perf/check && test "$(cat /tmp/traverse-out)" = KEEP')
 for variant,profile in (('before',True),('after',True),('after',False)):
  for tool in ('find','du','cp','rm'):
   if tool=='rm':run('cancel seed '+variant,'/mnt/Perf/after/cp -R --no-progress /mnt/Perf/n100 /mnt/Perf/cancel-rm')
   args='/mnt/Perf/n1000' if tool in ('find','du') else ('-R --no-progress /mnt/Perf/n1000 /mnt/Perf/cancel-cp' if tool=='cp' else '-R --no-progress /mnt/Perf/cancel-rm')
   image=('iprofile' if tool in ('find','du') else 'fprofile') if profile else tool
   prefix=tool+' ' if profile else ''
   c.t.write(f'/mnt/Perf/{variant}/{image} {prefix}{args} > /dev/null\r\n'.encode());time.sleep(.15)
   start=time.monotonic();c.t.get_socket().sendall(bytes([255,244]));reply=c.t.read_until(b'\r\n/ > ',30);ms=(time.monotonic()-start)*1000
   assert reply.endswith(b'\r\n/ > '),reply
   rows.append(dict(test=f'cancel {variant} {tool} profile={profile}',host_reply_ms=round(ms,2),text=reply.decode('latin1')));p.write_text(json.dumps(rows,indent=2)+'\n')
   run('status and recovery '+variant+' '+tool,'test "$?" -eq 130 && status runtime')
   if tool in ('cp','rm'):run('partial disposable cleanup','/mnt/Perf/after/rm -Rf --no-progress /mnt/Perf/cancel-cp /mnt/Perf/cancel-rm')
 for n in range(3):
  run('repeat '+str(n),'i=0; while test $i -lt 10; do /mnt/Perf/after/find /mnt/Perf/n100 > /dev/null || break; /mnt/Perf/after/du /mnt/Perf/n100 > /dev/null || break; i=$((i+1)); done; test $i -eq 10 && status runtime')
 run('final source retained','test "$(cat /tmp/traverse-out)" = KEEP && fsprobe same /tmp/batch3b/source /tmp/traverse-check/sub/slash%2F%25')
finally:c.close()
