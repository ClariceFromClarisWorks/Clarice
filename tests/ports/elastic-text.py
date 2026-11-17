"""Focused upstream-output, allocation failure and cleanup checks for long records."""
from pathlib import Path
import subprocess,os,tempfile,json,re
root=Path(__file__).resolve().parents[2];results=[]
def run(name,args,data,extra=None):
 base='awk-host' if name=='awk' else 'pipelines-host'
 p=subprocess.run([str(root/'work'/base/name),*args],input=data,capture_output=True,env=dict(os.environ,CLARICE_TEST_STATS='1',**(extra or {})),timeout=90)
 assert b'AddressSanitizer' not in p.stderr and b'runtime error:' not in p.stderr,p.stderr
 m=re.search(rb'PORT_STATS peak=(\d+) remaining=(\d+)',p.stderr);assert m and int(m[2])==0,(p.returncode,p.stderr)
 results.append(dict(command=name,args=args,input_bytes=len(data),status=p.returncode,peak=int(m[1]),remaining=int(m[2]),budget=(extra or {}).get('CLARICE_TEST_ALLOC_BUDGET')))
 return p
with tempfile.TemporaryDirectory(prefix='clx-elastic-',dir='/tmp') as td:
 env={'CLARICE_TEST_TMP':td}
 for size in (16,16383,16384,16385,20000,65536):
  for newline in (b'',b'\n'):
   data=b'z'*size+newline
   a=run('sort',[],data,env);assert a.returncode==0 and a.stdout==b'z'*size+b'\n',(size,a.stderr)
   a=run('awk',['{print length($0)}'],data);assert a.returncode==0 and a.stdout==str(size).encode()+b'\n',(size,a.stderr)
 data=b'z'*20000+b'\n'+b'a'*20001+b'\n'+b'z'*20000+b'\n'+b'b\n'*180
 for args in ([],['-r'],['-u'],['-k','1,1']):
  a=run('sort',args,data,env);b=subprocess.run([str(root/'work/pipelines-host/reference-sort'),*args],input=data,capture_output=True)
  assert (a.returncode,a.stdout)==(b.returncode,b.stdout),(args,a.stderr)
  assert not list(Path(td).iterdir())
 a=run('awk',['{print NF,length($0),length($NF)}'],b'x '*20000+b'end\n');assert a.returncode==0 and a.stdout==b'20001 40003 3\n',a.stderr
 for name,args in [('sort',[]),('awk',['{print length($0)}'])]:
  a=run(name,args,b'x'*65536+b'\n',dict(env,CLARICE_TEST_ALLOC_BUDGET='24000'));assert a.returncode!=0 and not a.stdout,a.stderr
  a=run(name,args,b'x\n',dict(env,CLARICE_TEST_ALLOC_BUDGET='24000'));assert a.returncode==0,a.stderr
  a=run(name,args,data*5,dict(env,CLARICE_CANCEL_AFTER='20'));assert a.returncode==130,a.stderr
  assert not list(Path(td).iterdir())
 for fail in ('CLARICE_FAIL_WRITE','CLARICE_FAIL_FILE_WRITE'):
  a=run('sort',[],data,dict(env,**{fail:'1'}));assert a.returncode!=0 and not list(Path(td).iterdir()),a.stderr
out=root/'outputs/utility-audit';out.mkdir(exist_ok=True);(out/'elastic-host.json').write_text(json.dumps(results,indent=2)+'\n')
print('PASS',len(results),'elastic text cases: exact output, long records/fields, bounded-memory failure, cancellation, spill cleanup and output errors')
