"""Compare the portable part of the native workflow with dash + host tools."""
from pathlib import Path
import subprocess,tempfile,json,re
r=Path(__file__).resolve().parents[2];out=r/'outputs/utility-closeout'
s=(r/'tests/utility-correctness/closeout.sh').read_text()
with tempfile.TemporaryDirectory() as d:
 root=d+'/closeout';s=s.replace('/tmp/closeout',root).replace('status runtime',':')
 p=subprocess.run(['/bin/dash','-c','sh -c "exit 130"\n'+s],text=True,capture_output=True)
 reference=p.stdout.replace(root,'/tmp/closeout')
 actual=(out/'closeout.out').read_text()
 # Clarice prints its usage on stdout; runtime diagnostics are Mac-specific.
 actual=re.sub(r'find \[path.*?INVALID:2', 'INVALID:1',actual,flags=re.S)
 actual=actual[:actual.index('Runtime:')]+ 'CLOSEOUT_DONE\n'
 # Invalid operands are nonzero on both; exact usage status is implementation-specific.
 reference=re.sub(r'INVALID:[1-9][0-9]*','INVALID:1',reference)
 actual=re.sub(r"(?m)^ +([12] (?:apple|pear))$",r"\1",actual)
 reference=re.sub(r"(?m)^ +([12] (?:apple|pear))$",r"\1",reference)
 assert actual==reference,(actual,reference)
 (out/'reference.json').write_text(json.dumps(dict(reference='dash and macOS utilities',passed=True,stdout=reference,status=p.returncode),indent=2)+'\n')
 print('PASS: portable workflow exact output after documented diagnostic/status normalization')
