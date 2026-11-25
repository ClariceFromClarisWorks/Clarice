"""Normal 68000 Snow acceptance through the existing loopback-only bridge."""
import sys,json,time
from pathlib import Path
sys.path.insert(0,'tests/remote-sessions');from client import Client
c=Client(port=23259,password='');rows=[]
def run(name,source,expected=(),timeout=90):
 rc,out=c.command(source,timeout);rows.append(dict(test=name,status=rc,output=out));print(out,flush=True)
 assert rc==0,(name,rc,out)
 for item in expected:assert item in out,(name,item,out)
 return out
try:
 out=run('local','cat /tmp/closure-result',('PASS-case','PASS-hyphen','at=<~>','PASS-home','PASS-glob','refused=126','PASS-receipt','CLOSURE-LOCAL-DONE\r\n0'))
 assert out.count('remaining=0')==6
 run('remote-pattern',r"case b in [a\-c]) false;; *) echo PASS;; esac; x=bword; test ${x#[a\-c]} = bword; r=$?; echo trim=$r; test $r = 0",('PASS\r\n','trim=0'))
 run('remote-at',r"set -- x '~'; set -- $@; printf '<%s>\n' \"$@\"; test \"$2\" = '~'".replace('\\"','"'),('<~>',))
 run('remote-home','''awk 'BEGIN {print ENVIRON["HOME"]}'; HOME=/override awk 'BEGIN {print ENVIRON["HOME"]}'; env -i /bin/awk 'BEGIN {print ("HOME" in ENVIRON)}'; echo "$HOME"''',('\r\n/home\r\n','\r\n/override\r\n','\r\n0\r\n'))
 c.t.write(b'! while :; do :; done | cat\r\n');time.sleep(1);c.t.write(b'\x03');reply=c.t.read_until(b' > ',30)
 out=run('negated-pipeline-cancel','echo cancel=$?; echo recovered; status runtime',('cancel=130','recovered\r\n','remaining=0'))
 rows[-1]['interrupt_reply']=reply.decode('latin1')
 run('receipt-corrupt','appinstall install /mnt/Sys6Unix/Perf/entry.clx closure-bad; test -f /etc/clarice/clx-closure-bad.ok || exit 1; printf x | dd of=/bin/closure-bad bs=1 seek=1024 conv=notrunc; appinstall check /bin/closure-bad; r=$?; echo rejected=$r; test "$r" = 126 && test ! -f /etc/clarice/clx-closure-bad.ok',('rejected=126','CLX payload checksum mismatch'))
 for i in range(3):run('repeat-'+str(i),'/bin/ls /tmp >/dev/null; status runtime',('remaining=0',))
finally:c.close()
Path('outputs/batch1-closure/guest.json').write_text(json.dumps(rows,indent=2)+'\n');print('PASS normal-build local/remote closure acceptance')
