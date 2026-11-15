from pathlib import Path
import subprocess,json,tempfile
root=Path(__file__).resolve().parents[2];results=[]
# Function keeps both reads on one input descriptor.
script='x=old; f() { read x; echo first:$?:$x; read x; echo second:$?:$x; }; f'
for n in (0,1,126,127,128,8192):
 p=subprocess.run([str(root/'work/sh-host'),script],input=b'a'*n+b'\nnext\n',capture_output=True)
 expected=('first:'+('2:old' if n>127 else '0:'+'a'*n)+'\nsecond:0:next\n').encode()
 assert p.returncode==0 and p.stdout==expected,(n,p.stdout,p.stderr)
 assert b'AddressSanitizer' not in p.stderr and b'runtime error:' not in p.stderr,p.stderr
 results.append({'length':n,'passed':True})
p=subprocess.run([str(root/'work/sh-host'),'read x; echo status:$?:$x'],input=b'last',capture_output=True)
assert p.stdout==b'status:1:last\n',p
p=subprocess.run([str(root/'work/sh-host'),'read x','20'],input=b'x'*100000,capture_output=True)
assert p.returncode==130,p
(root/'outputs/utility-audit/read-host.json').write_text(json.dumps(results,indent=2)+'\n')
print('PASS read: 127-byte boundary, overflow consumes one record/preserves variable, following record and EOF')
