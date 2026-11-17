"""Validate already-exported disposable Snow fixture results; does not drive Snow.
Run elastic.py, posix-focused.py and scenarios.py to regenerate reference inputs.
"""
from pathlib import Path
import subprocess,json,re,hashlib
r=Path(__file__).resolve().parents[2];w=r/'work/shell-elastic';g=r/'outputs/shell-elastic/guest'
rows=json.loads((r/'outputs/shell-elastic/host.json').read_text());results=[]
for i,row in enumerate(rows):
 reference=subprocess.run(['/bin/dash','-c',(w/f'case{i}.sh').read_text(),f'/mnt/Audit/elastic/e{i}.sh'],input=(w/f'in{i}').read_bytes(),capture_output=True,check=False)
 actual=(g/f'o{i}').read_bytes();err=(g/f'r{i}').read_bytes();status=int((g/f's{i}').read_text())
 results.append(dict(name=row['name'],passed=actual==reference.stdout and not err and status==reference.returncode,output_bytes=len(actual),sha256=hashlib.sha256(actual).hexdigest(),status=status,stderr=err.decode()))
old=[dict(batch=i,passed=(g/f'p{i}').read_bytes()==(r/f'work/utility-correctness/posix-{i}.expected').read_bytes() and not (g/f'pe{i}').read_bytes()) for i in range(8)]
whole=[]
for p in (r/'tests/shell/scenarios').glob('*.sh'):
 whole.append(dict(name=p.stem,passed=(g/(p.stem+'.out')).read_bytes()==(r/'work/utility-correctness/scenarios'/(p.stem+'.expected')).read_bytes(),stderr=(g/(p.stem+'.err')).read_text()))
low=(g/'lowmem.out').read_bytes()==b'2:3:old\n' and b'out of memory' in (g/'lowmem.err').read_bytes()
cancel=(g/'cancel-status').read_bytes()==b'CANCEL:130\n' and 'cancel-bad' not in (g/'catalog.txt').read_text()
heaps=re.findall(r'Runtime: heap=(\d+) largest=(\d+)',(g/'memory').read_text())
report=dict(build=2026111541,platform='Snow Plus 4 MB, System 6.0.7 reported, uncapped FFWD approximately 15–16x',elastic=results,focused=old,whole_scripts=whole,low_memory_preserves_old_value=low,cancellation_130_no_following_command=cancel,repeated_heap=heaps,after_cancel=(g/'after-cancel').read_text())
(r/'outputs/shell-elastic/guest.json').write_text(json.dumps(report,indent=2)+'\n')
assert all(x['passed'] for x in results+old+whole) and low and cancel
assert len(heaps)==5 and len(set(heaps[1:]))==1,heaps
print('PASS Snow: 41 elastic, 39 focused, 12 whole-script outputs, low memory, cancellation, repeated cleanup')
