"""Compare the port with pristine sbase grep using the host's regex library."""
from pathlib import Path
import subprocess,tempfile,os,json
root=Path(__file__).resolve().parents[2]
port=root/'work/ports-host/grep';reference=root/'work/ports-host/reference-grep'
results=[]
with tempfile.TemporaryDirectory() as tmp:
 d=Path(tmp); data=b'alpha\nBeta\nalphabet\n123\n\nword word\nlast'
 (d/'a').write_bytes(data);(d/'b').write_bytes(b'beta\nalpha\n');(d/'patterns').write_bytes(b'^alpha\n[0-9]\n');(d/'empty').write_bytes(b'')
 cases=[['alpha'],['-E','^(alpha|Beta)$'],['-F','alpha'],['-i','beta'],['-v','alpha'],['-n','alpha'],['-c','alpha'],['-q','alpha'],['-q','missing'],['-x','alpha'],['-e','alpha','-e','123'],['-f','patterns'],['-f','empty'],['-E','[[:digit:]]+'],[r'\(word\) \1'],['-E','a{1,3}'],['-H','alpha','a'],['-h','alpha','a','b'],['-l','alpha','a','b'],['alpha','a','b'],['-s','alpha','missing'],['alpha','missing'],['-E','['],['--bad-option'],['-e','alpha\n123'],['alpha','-']]
 for args in cases:
  def run(exe):return subprocess.run([str(exe),*args],cwd=tmp,input=data,capture_output=True,env={**os.environ,'LC_ALL':'C'},timeout=10)
  a,b=run(port),run(reference)
  # Diagnostics identify different regex libraries/executable names; compare presence and status.
  ok=(a.stdout,a.returncode,bool(a.stderr))==(b.stdout,b.returncode,bool(b.stderr))
  assert b'AddressSanitizer' not in a.stderr and b'runtime error:' not in a.stderr,a.stderr
  results.append({'args':args,'status':a.returncode,'passed':ok})
  assert ok,(args,a.stdout,a.stderr,a.returncode,b.stdout,b.stderr,b.returncode)
 # Exercise buffered spans, refills, embedded NULs and a final unterminated line.
 import random
 rng=random.Random(68000)
 for case in range(40):
  lines=[bytes(rng.choice(b'abc XYZ123') for _ in range(rng.choice([0,1,17,255,511,512,513,2048]))) for _ in range(12)]
  if case%7==0: lines[0]=b'a\0ab'
  body=b'\n'.join(lines)+(b'\n' if case%2 else b'')
  args=['-Fn','ab']
  a=subprocess.run([str(port),*args],input=body,capture_output=True,timeout=10)
  b=subprocess.run([str(reference),*args],input=body,capture_output=True,timeout=10)
  assert (a.stdout,a.returncode)==(b.stdout,b.returncode),(case,a.stderr)
  assert b'AddressSanitizer' not in a.stderr and b'runtime error:' not in a.stderr,a.stderr
  results.append({'test':'buffer boundaries','case':case,'passed':True})
 for label,args,body,env,status in [
  ('empty pattern',[''],b'alpha\n',{},0),
  ('word boundaries',['-w','word'],b'word word\n',{},0),
  ('long line',['-F','a'],b'a'*17000,{},2),
  ('cancel',['a'],b'a\n'*100000,{'CLARICE_CANCEL_AFTER':'4'},130),
  ('write error',['a'],b'a\n',{'CLARICE_FAIL_WRITE':'1'},2),
  ('allocation bound',['-E','(a|b|c|d){255}'],b'a\n',{},2)]:
  a=subprocess.run([str(port),*args],input=body,capture_output=True,env={**os.environ,**env},timeout=10)
  assert a.returncode==status,(label,a.returncode,a.stderr)
  assert b'AddressSanitizer' not in a.stderr and b'runtime error:' not in a.stderr,a.stderr
  results.append({'test':label,'status':a.returncode,'passed':True})
(root/'outputs/grep/host-tests.json').write_text(json.dumps(results,indent=2)+'\n')
print(f'PASS: {len(cases)} option comparisons plus 40 buffered-I/O comparisons; line/allocation bounds, cancellation and output failure')
