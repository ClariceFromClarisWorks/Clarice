from pathlib import Path
import subprocess,os,re,json
r=Path(__file__).resolve().parents[2];rows=[]
fixtures=[('record','read -r x',b'a'*8192+b'\n'),('multi','read a b c',b'one two three\n'),('substitution','x=$(printf hello); printf "%s\\n" "$x"',b''),('scope','x=old; set -- outer; (set -- inner; x=new); printf "%s:%s\\n" "$x" "$1"',b''),('args','set -- '+ ' '.join(str(i) for i in range(100)),b''),('prefix','x=old; x=new read y; printf "%s:%s\\n" "$x" "$y"',b'value\n')]
for name,source,data in fixtures:
 env={**os.environ,'CLARICE_SHELL_ALLOC_STATS':'1'}
 base=subprocess.run([str(r/'work/sh-host'),source],input=data,capture_output=True,env=env)
 m=re.search(rb'ALLOC calls=(\d+) peak=(\d+) remaining=0 fault=0',base.stderr);assert m,(name,base.stderr)
 count=int(m[1]);failures=0
 for i in range(1,count+1):
  p=subprocess.run([str(r/'work/sh-host'),source],input=data,capture_output=True,env={**env,'CLARICE_SHELL_FAIL_ALLOC':str(i)},timeout=10)
  assert p.returncode>=0 and p.returncode!=124 and b'remaining=0 fault=1' in p.stderr and b'AddressSanitizer' not in p.stderr and b'runtime error:' not in p.stderr,(name,i,p.returncode,p.stderr)
  if name in ['record','multi','args']:assert p.returncode!=0,(name,i,p.stdout,p.stderr)
  failures+=1
 rows.append(dict(name=name,injected_failures=failures,host_adapter_peak=int(m[2]),remaining=0))
for source,data,expected,status in [
 ('x=old; read x; echo "$?:$x"',b'abc\0def\n',b'2:old\n',0),
 ('x=$(while true; do :; done); echo BAD',b'',b'',130)]:
 cmd=[str(r/'work/sh-host'),source]+(['20'] if status==130 else [])
 p=subprocess.run(cmd,input=data,capture_output=True,env={**os.environ,'CLARICE_SHELL_ALLOC_STATS':'1'},timeout=10)
 assert p.returncode==status and p.stdout==expected and b'remaining=0' in p.stderr,(p.returncode,p.stdout,p.stderr)
(r/'outputs/shell-elastic/allocation-failures.json').write_text(json.dumps(rows,indent=2)+'\n')
print('PASS',sum(x['injected_failures'] for x in rows),'adapter allocation failures; zero retained tracked allocations')
