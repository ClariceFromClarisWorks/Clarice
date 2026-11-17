"""Generate export commands (--export), or verify results after clean unmount."""
from pathlib import Path
import json,sys,re
r=Path(__file__).resolve().parents[2];w=r/'work/shell-practical';g=w/'guest';g.mkdir(exist_ok=True);out=r/'outputs/shell-practical'
if '--export' in sys.argv:
 names=[f'{prefix}{i}' for i in range(6) for prefix in ['o','e','s']]+[f'd{i}.{suffix}' for i in range(7) for suffix in ['out','err','status']]+['env.out','env.status','memory','recovered']
 lines=['set -e','hmount /src/work/parser-safety/audit.hda']+[f'hcopy -r :practical:{name} /src/work/shell-practical/guest/{name}' for name in names]+[f'hcopy -r :elastic:{name} /src/work/shell-practical/guest/{name}' for name in ['cancel-status','cancel-out','cancel-err']]+['humount']
 (w/'export.sh').write_text('\n'.join(lines)+'\n');sys.exit()
rows=[]
for i,row in enumerate(json.loads((out/'host.json').read_text())):
 actual=(g/f'o{i}').read_text();error=(g/f'e{i}').read_text();status=int((g/f's{i}').read_text());ok=status==0 and actual==row['reference']['stdout'] and not error
 rows.append(dict(name=row['name'],passed=ok,status=status,stdout=actual,stderr=error))
 if not ok:print('FAIL',row['name'],status,error,'output lengths',len(actual),len(row['reference']['stdout']))
diagnostics=[]
for i in range(7):
 err=(g/f'd{i}.err').read_text();status=int((g/f'd{i}.status').read_text());diagnostics.append(dict(error=err,status=status));assert status==2 and 'out of memory' not in err
assert (g/'recovered').read_text()=='RECOVERED\n'*7
assert (g/'env.out').read_text()=='30 512\n'*3 and (g/'env.status').read_text()=='0\n'*3
memory=re.findall(r'Runtime: heap=(\d+) largest=(\d+)',(g/'memory').read_text());assert len(memory)==3,memory
# Whole-application stability did not pass: preserve the measured decline and
# matched old/new evidence rather than relaxing it into a stability claim.
comparison=json.loads((out/'memory-comparison.json').read_text())
assert comparison['old']['heap_drop']==comparison['new']['heap_drop']==384
assert (g/'memory').read_text().count('remaining=0')==3
assert (g/'cancel-status').read_text().strip()=='CANCEL:130' and 'STDOUT_RESTORED' in (g/'cancel-out').read_text() and 'STDERR_RESTORED' in (g/'cancel-err').read_text()
assert all(x['passed'] for x in rows)
(out/'guest.json').write_text(json.dumps(dict(build=2026111544,platform='Snow Plus 4 MB / reported System 6.0.7 / fixed uncapped FFWD about 15–16x',scripts=rows,diagnostics=diagnostics,environment='30 exported values of 512 bytes, three runs',memory=memory,portable_remaining=0,native_heap_stable=False,native_retention='pre-existing matched old/new decline; see memory-comparison.json',cancellation_status=130,streams_restored=True),indent=2)+'\n')
print('PASS six upstream-script variants, seven syntax/recovery cases, elastic native awk, Ctrl+C / owned cleanup; native heap stability remains unresolved')
