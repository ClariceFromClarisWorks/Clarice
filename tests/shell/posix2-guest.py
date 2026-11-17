"""Validate exported native outputs, separately from host comparisons.
Run only after guest execution and clean Audit unmount; see RESULTS.md.
"""
from pathlib import Path
import json,subprocess,re
r=Path(__file__).resolve().parents[2];g=r/'work/shell-posix2/guest';out=r/'outputs/shell-posix2';rows=[]
for i,case in enumerate(json.loads((out/'host.json').read_text())):
 actual=(g/f'o{i}').read_bytes();status=int((g/f's{i}').read_text())
 ok=actual==case['expected'].encode() and status==case['status']
 rows.append(dict(name=case['name'],passed=ok,status=status,stdout=actual.decode(),stderr=(g/f'e{i}').read_text()))
 if not ok:print('FAIL',i,rows[-1],case['expected'])
assert (g/'env.out').read_bytes()==b'local:\nexport:public\n' and not (g/'env.err').read_bytes() and int((g/'env.status').read_text())==0
reg=[]
for i,case in enumerate(json.loads((r/'outputs/shell-elastic/host.json').read_text())):
 w=r/'work/shell-elastic';p=subprocess.run(['dash','-c',(w/f'case{i}.sh').read_text(),f'/mnt/Audit/elastic/e{i}.sh'],input=(w/f'in{i}').read_bytes(),capture_output=True)
 reg.append(dict(name=case['name'],passed=(g/f'elastic-o{i}').read_bytes()==p.stdout and int((g/f'elastic-s{i}').read_text())==p.returncode and not (g/f'elastic-r{i}').read_bytes()))
for i in range(8):reg.append(dict(name=f'focused-{i}',passed=(g/f'elastic-p{i}').read_bytes()==(r/f'work/utility-correctness/posix-{i}.expected').read_bytes() and not (g/f'elastic-pe{i}').read_bytes()))
for p in (r/'tests/shell/scenarios').glob('*.sh'):reg.append(dict(name=p.stem,passed=(g/f'elastic-{p.stem}.out').read_bytes()==(r/'work/utility-correctness/scenarios'/f'{p.stem}.expected').read_bytes()))
mem=re.findall(r'Runtime: heap=(\d+) largest=(\d+)',(g/'memory').read_text())
assert len(mem)==3 and max(int(x[0]) for x in mem)-min(int(x[0]) for x in mem)<=8 and len({x[1] for x in mem})==1,mem
assert (g/'memory').read_text().count('remaining=0')==3
assert all(x['passed'] for x in rows+reg),[x for x in rows+reg if not x['passed']]
report=dict(build=2026111543,platform='Snow Plus / 4 MB / reported System 6.0.7, fixed uncapped FFWD about 15–16x',cases=rows,regressions=reg,native_awk_export_visibility=True,memory=mem,cancellation=(g/'cancel-status').read_text(),restoration=(g/'cancel-out').read_text(),error_restoration=(g/'cancel-err').read_text())
assert report['cancellation'].strip()=='CANCEL:130' and 'STDOUT_RESTORED' in report['restoration'] and 'STDERR_RESTORED' in report['error_restoration']
(out/'guest.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS 54 POSIX cases, 39 focused / 12 complete / 41 elastic regressions, native exports, heap variation at most 8 bytes, cancellation/restoration')
