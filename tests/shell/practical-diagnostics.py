from pathlib import Path
import subprocess,json,os
r=Path(__file__).resolve().parents[2];out=r/'outputs/shell-practical';rows=[]
for source in ['echo "','echo '+"'",'echo ${x:-"bad}', 'if true; then', 'for x in a; do', 'echo $(echo hi', 'echo ${}']:
 p=subprocess.run([str(r/'work/sh-host'),source],capture_output=True,env={**os.environ,'CLARICE_SHELL_ALLOC_STATS':'1','CLARICE_SHELL_RECOVERY':'1'})
 assert p.returncode==2 and b'out of memory' not in p.stderr and b'remaining=0' in p.stderr and b'STREAMS-RESTORED' in p.stdout,(source,p.returncode,p.stderr)
 rows.append(dict(source=source,status=p.returncode,error=p.stderr.decode(),recovered=True))
(out/'diagnostics.json').write_text(json.dumps(rows,indent=2)+'\n')
print('PASS seven syntax diagnostics and recovery')
