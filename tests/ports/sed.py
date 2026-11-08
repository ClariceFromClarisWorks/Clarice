from pathlib import Path
import subprocess,tempfile,os,json
root=Path(__file__).resolve().parents[2];port=root/'work/sed-host/sed';reference=root/'work/sed-host/reference-sed'
results=[]
with tempfile.TemporaryDirectory() as tmp:
 d=Path(tmp);data=b'alpha\nBeta\nalphabet\n123\nword word\nlast\n';(d/'input').write_bytes(data);(d/'program').write_text('s/alpha/A/g\n')
 cases=[['s/alpha/A/'],['s/a/X/g'],['-n','2,4p'],['/alpha/d'],['1q'],['-n','/alpha/p'],['y/ab/AB/'],['-e','s/alpha/A/','-e','s/Beta/B/'],['-f','program'],['s/alpha/A/','input'],['N;s/\\n/:/'],['-n','1h;2{g;p;}'],[':again;s/aa/a/;t again'],['-n','$p'],['s/[[:digit:]]/X/g'],[r's/\(word\) \1/\1/'],['-n','1,2w saved'],['bad'],['p','missing']]
 for args in cases:
  def run(exe): return subprocess.run([str(exe),*args],cwd=tmp,input=data,capture_output=True,env={**os.environ,'LC_ALL':'C'},timeout=10)
  a,b=run(port),run(reference);ok=(a.stdout,a.returncode,bool(a.stderr))==(b.stdout,b.returncode,bool(b.stderr))
  assert b'AddressSanitizer' not in a.stderr and b'runtime error:' not in a.stderr,a.stderr
  assert ok,(args,a.stdout,a.stderr,a.returncode,b.stdout,b.stderr,b.returncode)
  results.append({'args':args,'status':a.returncode,'passed':True})
 for label,args,body,env,status in [('cancel loop',[':x;b x'],b'a\n',{'CLARICE_CANCEL_AFTER':'4'},130),('long line',['p'],b'a'*17000,{},2),('write error',['p'],b'a\n',{'CLARICE_FAIL_WRITE':'1'},2)]:
  a=subprocess.run([str(port),*args],cwd=tmp,input=body,capture_output=True,env={**os.environ,**env},timeout=10)
  assert a.returncode==status,(label,a.returncode,a.stderr)
  assert b'AddressSanitizer' not in a.stderr and b'runtime error:' not in a.stderr,a.stderr
  results.append({'test':label,'status':status,'passed':True})
(root/'outputs/grep/sed-host-tests.json').write_text(json.dumps(results,indent=2)+'\n')
print(f'PASS: {len(cases)} upstream sed comparisons; infinite-loop cancellation, line cap and write failure')
