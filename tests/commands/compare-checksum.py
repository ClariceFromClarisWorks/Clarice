"""Exercise prepared, genuine sbase sources against host reference utilities."""
from pathlib import Path
import subprocess,tempfile,os
r=Path(__file__).resolve().parents[2]
subprocess.run(['python3',str(r/'scripts/prepare-pipelines.py')],check=True)
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'poll.h').write_text('static void port_poll(void) {}\n')
 for name in ('cmp','cksum'):
  subprocess.run(['cc','-fsanitize=address,undefined','-include',str(p/'poll.h'),'-I',str(r/'src/vendor/sbase'),str(r/f'work/build/pipelines/{name}.c'),str(r/'src/vendor/sbase/libutil/eprintf.c'),str(r/'src/vendor/sbase/libutil/fshut.c'),'-o',str(p/name)],check=True)
 data=bytes(range(256))*4096
 (p/'a').write_bytes(data);(p/'b').write_bytes(data);(p/'c').write_bytes(data[:513]+b'X'+data[514:]);(p/'empty').write_bytes(b'');(p/'short').write_bytes(data[:91])
 cases=[('cmp',['a','b']),('cmp',['a','c']),('cmp',['-l','a','c']),('cmp',['-s','a','short']),('cmp',['empty','empty']),('cmp',['-s','a','absent']),('cksum',['a','empty','short']),('cksum',[])]
 for name,args in cases:
  actual=subprocess.run([str(p/name),*args],cwd=p,input=data,capture_output=True)
  ref=subprocess.run(['/usr/bin/'+name,*args],cwd=p,input=data,capture_output=True)
  assert actual.returncode==ref.returncode,(name,args,actual,ref)
  # cmp default wording differs by implementation; byte lists/checksums do not.
  if '-l' in args:assert actual.stdout.split()==ref.stdout.split()
  elif name=='cksum' or '-s' in args or args[0]==args[1]:assert actual.stdout==ref.stdout,(args,actual.stdout,ref.stdout)
 for name in ('cmp','cksum'):
  q=subprocess.run([str(p/name),'--help'],capture_output=True,text=True);assert q.returncode==0 and q.stdout and not q.stderr
  assert max(map(len,q.stdout.splitlines()))<=72
  q=subprocess.run([str(p/name),'--bad-option'],capture_output=True);assert q.returncode and q.stderr and not q.stdout
 q=subprocess.run([str(p/'cmp'),'-l','-s','a','b'],cwd=p,capture_output=True);assert q.returncode==2 and q.stderr
print('PASS: cmp/cksum binary 1 MiB, differences, empty/truncated/missing, stdin, help, invalid options; ASan/UBSan')
