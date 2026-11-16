"""Actual Clarice portable runtime: failures, cancellation and tracked cleanup."""
from pathlib import Path
import subprocess,os,tempfile
r=Path(__file__).resolve().parents[2]
with tempfile.TemporaryDirectory() as d:
 p=Path(d);data=bytes(range(256))*4096;(p/'a').write_bytes(data);(p/'b').write_bytes(data)
 count=0
 for name,args in [('cmp',['-s',str(p/'a'),str(p/'b')]),('cksum',[str(p/'a')])]:
  exe=str(r/'work/pipelines-host'/name)
  for extras in [{},{'CLARICE_CANCEL_AFTER':'8'},*[{'CLARICE_TEST_ALLOC_BUDGET':str(n)} for n in (0,64,512,2048,8192)],{'CLARICE_FAIL_WRITE':'1'}]:
   q=subprocess.run([exe,*args],capture_output=True,env={**os.environ,'CLARICE_TEST_STATS':'1',**extras},timeout=20)
   assert b'AddressSanitizer' not in q.stderr and b'runtime error:' not in q.stderr,q.stderr
   assert b'remaining=0' in q.stderr,(name,extras,q)
   if 'CLARICE_CANCEL_AFTER' in extras:assert q.returncode==130,(name,q)
   elif extras.get('CLARICE_TEST_ALLOC_BUDGET') in ('0','64','512'):assert q.returncode!=0,(name,q)
   elif not extras:assert q.returncode==0,(name,q)
   elif 'CLARICE_FAIL_WRITE' in extras and name=='cksum':assert q.returncode!=0,q
   count+=1
  q=subprocess.run([exe,*args],capture_output=True,env={**os.environ,'CLARICE_TEST_STATS':'1'});assert q.returncode==0 and b'remaining=0' in q.stderr
print(f'PASS: {count} runtime trials plus successful recovery; bounded memory, cancellation, output failure, no retained allocations')
