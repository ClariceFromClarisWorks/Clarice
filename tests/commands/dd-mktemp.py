from pathlib import Path
import os,subprocess,tempfile
r=Path(__file__).resolve().parents[2];base=r/'work/pipelines-host'
def run(n,args,cwd,extra={},data=b''):
 q=subprocess.run([str(base/n),*args],cwd=cwd,env={**os.environ,'CLARICE_TEST_STATS':'1',**extra},input=data,capture_output=True,timeout=20)
 assert b'AddressSanitizer' not in q.stderr and b'runtime error:' not in q.stderr and b'remaining=0' in q.stderr,q
 return q
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'in').write_bytes(bytes(range(256))*512)
 cases=[[],['bs=1k','count=3'],['ibs=13','obs=29','skip=2','count=7'],['ibs=7','obs=13','seek=2','count=5'],['bs=31','count=0'],['bs=31','count=1','seek=100','conv=notrunc'],['bs=31','count=1','seek=2','conv=notrunc'],['bs=33','count=4','conv=swab,sync']]
 for opts in cases:
  for f in ['out','ref']:(p/f).write_bytes(b'preserve prefix and tail'*30)
  q=run('dd',['if=in','of=out',*opts],p);ref=subprocess.run(['/bin/dd','if=in','of=ref',*opts],cwd=p,capture_output=True)
  assert q.returncode==ref.returncode==0 and (p/'out').read_bytes()==(p/'ref').read_bytes(),(opts,q,ref)
 for size in ['0','-1','2147483648','99999999999999999','2x2147483647','2048kxxx']:
  (p/'out').write_bytes(b'keep');q=run('dd',['if=in','of=out','bs='+size],p);assert q.returncode and (p/'out').read_bytes()==b'keep',q
 (p/'out').write_bytes(b'keep');q=run('dd',['if=in','of=out','bs=65536'],p,{'CLARICE_TEST_ALLOC_BUDGET':'8192'});assert q.returncode and (p/'out').read_bytes()==b'keep'
 q=run('dd',['if=in','of=out','bs=512'],p,{'CLARICE_CANCEL_AFTER':'8'});assert q.returncode==130
 q=run('dd',['if=in','of=out'],p,{'CLARICE_FAIL_FILE_WRITE':'1'});assert q.returncode!=0
 q=run('dd',['bs=13'],p,data=bytes(range(256)));assert q.returncode==0 and q.stdout==bytes(range(256))
 env={'CLARICE_TEST_TICKS':'12345','TMPDIR':d}
 for i in range(32):
  q=run('mktemp',['same.XXXXXX'],p,env);assert q.returncode==0,q
  path=p/q.stdout.decode().strip();assert path.exists();path.write_bytes(b'owned')
 q=run('mktemp',['same.XXXXXX'],p,env);assert q.returncode and not q.stdout
 assert len(list(p.glob('same.*')))==32 and all(f.read_bytes()==b'owned' for f in p.glob('same.*'))
 before=set(p.iterdir());q=run('mktemp',['-p',d],p,{**env,'CLARICE_FAIL_WRITE':'1'});assert q.returncode and set(p.iterdir())==before,q
 for args in [['bad'],['-u'],['-d'],['--bad']]:assert run('mktemp',args,p,env).returncode
 q=run('mktemp',['-q','/missing/place/name.XXXXXX'],p,env);assert q.returncode and not q.stdout
 for n in ['dd','mktemp']:
  q=run(n,['--help'],p);assert q.returncode==0 and max(map(len,q.stdout.splitlines()))<=72
print('PASS: eight dd reference copies; overflow/OOM preserve destination; cancel/write failures; binary stdio; mktemp collision/ownership/output-failure cleanup/options')
