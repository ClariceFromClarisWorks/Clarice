import sys,json
from pathlib import Path
sys.path.insert(0,'tests/remote-sessions');from client import Client
c=Client(port=23259,password='');rows=[]
def run(name,source,timeout=90):
 rc,out=c.command(source,timeout);rows.append(dict(test=name,status=rc,output=out));print(out,flush=True);assert rc==0,(name,out)
try:
 run('script8-status','time sh /mnt/Sys6Unix/Perf/8192.sh; r=$?; echo script-status=$r; test "$r" = 0')
 for i in range(5):run('repeat-'+str(i),'/mnt/Sys6Unix/Perf/entry.clx; r=$?; status runtime; test "$r" = 0')
 run('corrupt','cp /mnt/Sys6Unix/Perf/entry.clx /tmp/perf-bad; printf x | dd of=/tmp/perf-bad bs=1 seek=1024 conv=notrunc; appinstall check /tmp/perf-bad; r=$?; echo rejected=$r; test "$r" = 126')
 run('recover','echo recovered; /bin/ls /tmp >/dev/null; status runtime')
finally:c.close()
Path('outputs/userland-perf/final.json').write_text(json.dumps(rows,indent=2)+'\n')
print('PASS explicit script status, repeated loads, corrupt payload rejection and recovery')
