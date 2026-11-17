from pathlib import Path
import os,subprocess,time
r=Path(__file__).resolve().parents[2];base=r/'work/pipelines-host'
def run(n,args,extra={}):return subprocess.run([str(base/n),*args],env={**os.environ,'CLARICE_TEST_STATS':'1',**extra},capture_output=True,timeout=10)
q=run('sleep',['0']);assert q.returncode==0 and not q.stdout and b'remaining=0' in q.stderr
start=time.monotonic();q=run('sleep',['1']);elapsed=time.monotonic()-start;assert q.returncode==0 and elapsed>=.9
for args in [['-1'],['bad'],['.5'],['1','2']]:assert run('sleep',args).returncode!=0
q=run('sleep',['10'],{'CLARICE_CANCEL_AFTER':'4'});assert q.returncode==130 and b'remaining=0' in q.stderr
large={'CLARICE_TEST_ENV_COUNT':'64','CLARICE_TEST_ENV_VALUE':'x'*2048}
q=run('printenv',['ENV63'],large);assert q.returncode==0 and q.stdout==b'x'*2048+b'\n' and b'remaining=0' in q.stderr
q=run('printenv',['ABSENT'],large);assert q.returncode==1 and not q.stdout
q=run('printenv',[],large);assert q.returncode==0 and len(q.stdout.splitlines())==64
for budget in ('0','128','1024','4096'):
 q=run('printenv',[],{**large,'CLARICE_TEST_ALLOC_BUDGET':budget});assert q.returncode!=0 and b'remaining=0' in q.stderr
for n in ['sleep','printenv']:
 q=run(n,['--help']);assert q.returncode==0 and max(map(len,q.stdout.splitlines()))<=72
 q=run(n,['--bad']);assert q.returncode!=0 and not q.stdout
print('PASS: sleep duration/cancel/options; 64 exported 2KiB values, missing names, allocation failure and cleanup')
