import sys,json
from pathlib import Path
sys.path.insert(0,'tests/remote-sessions');from client import Client
c=Client(port=23259,password='');rows=[]
try:
 for i in range(3):
  rc,out=c.command('time appinstall check /bin/awk; /bin/ls /tmp >/dev/null; status runtime',90)
  assert rc==0 and 'remaining=0' in out and 'error' not in out.lower();rows.append(out);print(out,flush=True)
finally:c.close()
Path('outputs/userland-perf/loads-'+sys.argv[1]+'.json').write_text(json.dumps(rows,indent=2)+'\n')
