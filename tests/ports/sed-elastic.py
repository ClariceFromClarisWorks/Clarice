from pathlib import Path
import subprocess,os,json,time
root=Path(__file__).resolve().parents[2]
port=root/'work/sed-host/sed'; ref=root/'work/sed-host/reference-sed'
rows=[]
for size in (0,1,127,16384,65536):
 data=b'x'*size+b'\n'
 for expression in ('s/x/y/','s/x/y/g','s/x/y/2','s/z/y/g','s/x*/Q/g','s/$/END/','s/^/BEGIN/'):
  args=[expression];t=time.monotonic()
  p=subprocess.run([str(port),*args],input=data,capture_output=True,env={**os.environ,'CLARICE_TEST_STATS':'1'},timeout=20)
  elapsed=time.monotonic()-t
  r=subprocess.run([str(ref),*args],input=data,capture_output=True,timeout=20)
  assert (p.stdout,p.returncode)==(r.stdout,r.returncode),(size,args,p.stderr)
  assert b'remaining=0' in p.stderr and b'AddressSanitizer' not in p.stderr and b'runtime error:' not in p.stderr,p.stderr
  rows.append({'size':size,'expression':expression,'host_seconds':elapsed,'stats':p.stderr.decode().strip()})
for expr,body in [(r's/\(ab\)/<&-\1>/g',b'ab ab\n'),('s/x/y/g',b'x'*65536+b'\n')]:
 p=subprocess.run([str(port),expr],input=body,capture_output=True,env={**os.environ,'CLARICE_TEST_ALLOC_BUDGET':'24000'},timeout=10)
 assert p.returncode==2 if len(body)>24000 else p.returncode==0,(p.returncode,p.stderr)
p=subprocess.run([str(port),'s/x/y/g'],input=b'x'*65536+b'\n',capture_output=True,env={**os.environ,'CLARICE_CANCEL_AFTER':'160','CLARICE_TEST_STATS':'1'},timeout=10)
assert p.returncode==130 and b'remaining=0' in p.stderr,(p.returncode,p.stderr)
(root/'outputs/utility-correctness/sed-host.json').write_text(json.dumps(rows,indent=2)+'\n')
print('PASS 35 large/boundary/empty-match comparisons, low-memory and cancellation cleanup')
