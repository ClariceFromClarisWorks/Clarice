"""Spool I/O failure and HOME scope regressions using the existing shell harness."""
import subprocess,os,json
from pathlib import Path
rows=[]
for cmd in ['echo BAD | cat', 'cat <<EOF\nBAD\nEOF\n', 'x=$(echo BAD)']:
 r=subprocess.run(['work/sh-host',cmd],env={**os.environ,'CLARICE_SHELL_FAIL_REWIND':'1','CLARICE_SHELL_RECOVERY':'1','CLARICE_SHELL_ALLOC_STATS':'1'},capture_output=True,text=True)
 assert r.returncode and 'BAD' not in r.stdout and 'STREAMS-RESTORED' in r.stdout and 'remaining=0' in r.stderr,(cmd,r)
 rows.append(dict(source=cmd,status=r.returncode,out=r.stdout,error=r.stderr))
r=subprocess.run(['work/sh-host','printf "%s:%s\\n" "$HOME" ~; HOME=/tmp; printf "%s\\n" ~/file'],capture_output=True,text=True);assert r.stdout=='/home:/home\n/tmp/file\n',r.stdout
rows.append(dict(source='HOME defaults and override',out=r.stdout))
Path('outputs/userland-p2/failures.json').write_text(json.dumps(rows,indent=2)+'\n')
print('PASS three spool rewind failures, streams/heap recovery; HOME/tilde')
