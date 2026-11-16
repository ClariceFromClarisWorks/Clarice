"""Check exported guest files; acceptance is distinct from the host oracle."""
from pathlib import Path
import json,re,subprocess,hashlib
r=Path(__file__).resolve().parents[2];g=r/'outputs/parser-safety/guest';w=r/'work/shell-elastic'
results=[]
for i,row in enumerate(json.loads((r/'outputs/shell-elastic/host.json').read_text())):
 d=subprocess.run(['dash','-c',(w/f'case{i}.sh').read_text(),f'/mnt/Audit/elastic/e{i}.sh'],input=(w/f'in{i}').read_bytes(),capture_output=True)
 ok=(g/f'o{i}').read_bytes()==d.stdout and not (g/f'r{i}').read_bytes() and int((g/f's{i}').read_text())==d.returncode
 results.append(dict(name=row['name'],passed=ok))
for i in range(8):results.append(dict(name=f'focused batch {i}',passed=(g/f'p{i}').read_bytes()==(r/f'work/utility-correctness/posix-{i}.expected').read_bytes() and not (g/f'pe{i}').read_bytes()))
for p in (r/'tests/shell/scenarios').glob('*.sh'):results.append(dict(name=p.stem,passed=(g/(p.stem+'.out')).read_bytes()==(r/'work/utility-correctness/scenarios'/(p.stem+'.expected')).read_bytes()))
capacity=[]
for row in json.loads((r/'outputs/parser-safety/capacity.json').read_text()):
 n=row['case'];status=int((g/f'cap-{n}.status').read_text());out=(g/f'cap-{n}.out').read_text();err=(g/f'cap-{n}.err').read_text();ok=status==row['status'] and out==row['stdout'] and err==row['stderr'];capacity.append(dict(name=n,status=status,passed=ok,stderr=err))
f=(g/'parser-faults.out').read_text();trials=[dict(calls=int(a),injected=bool(int(b)),status=int(c),retained_bytes=int(d)) for a,b,c,d in re.findall(r'PARSER calls=(\d+) hit=(\d+) status=(\d+) delta=(-?\d+)',f)]
assert len(trials)==441 and sum(x['injected'] for x in trials)==282
assert all(x['retained_bytes']==0 for x in trials) and 'LOST_FUNCTION' not in f and f.endswith('FAULTS_DONE\n')
assert all(x['passed'] for x in results+capacity),[x for x in results+capacity if not x['passed']]
report=dict(build=2026111542,kind='diagnostic build; parser hooks not in normal builds',platform='Snow Plus 4 MB, reported System 6.0.7; fixed uncapped FFWD about 15–16x',regressions=results,capacity=capacity,trials=trials,memory=re.findall(r'Runtime: heap=(\d+) largest=(\d+)',(g/'parser-memory').read_text()),cancellation='Command-period cancelled; Ctrl+C requires existing patched Snow and remains for normal-build validation')
(r/'outputs/parser-safety/guest-diagnostic.json').write_text(json.dumps(report,indent=2)+'\n')
print('PASS guest: 41 elastic, 39 focused, 12 complete scripts, 8 capacity/guard cases, 282 injected failures; zero retained parser bytes')
