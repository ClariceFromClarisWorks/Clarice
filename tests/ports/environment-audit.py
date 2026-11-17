from pathlib import Path
import os,subprocess,json,tempfile
r=Path(__file__).resolve().parents[2];out=r/'outputs/shell-practical';rows=[]
for count,length,budget in [(30,4096,None),(30,4096,64),(30,4096,512),(30,4096,4096),(30,4096,16384),(3,5,None)]:
 env={**os.environ,'CLARICE_TEST_ENV_COUNT':str(count),'CLARICE_TEST_ENV_VALUE':'x'*length,'CLARICE_TEST_STATS':'1'}
 if budget:env['CLARICE_TEST_ALLOC_BUDGET']=str(budget)
 p=subprocess.run([str(r/'work/awk-host/awk'),'BEGIN { n=0; for(k in ENVIRON)n++; print n, length(ENVIRON["ENV'+str(count-1)+'"]) }'],env=env,capture_output=True)
 assert b'remaining=0' in p.stderr and (p.returncode!=0 if budget else p.returncode==0 and p.stdout==f'{count} {length}\n'.encode()),(count,length,budget,p.returncode,p.stdout,p.stderr)
 rows.append(dict(entries=count,value_bytes=length,budget=budget,status=p.returncode,stdout=p.stdout.decode(),accounting=p.stderr.decode()))
# The editor adapter now has its own elastic import/failure coverage.
p=subprocess.run(['python3',str(r/'tests/terminal/environment.py')],capture_output=True,text=True,check=True)
(out/'environment.json').write_text(json.dumps(dict(awk=rows,editor=dict(elastic=True,result=p.stdout)),indent=2)+'\n');print('PASS elastic awk and editor environments / exhaustion / cleanup')
