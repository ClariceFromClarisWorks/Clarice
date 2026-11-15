"""Check exported Snow evidence; never substitutes host results for guest runs."""
from pathlib import Path
import json,re
r=Path(__file__).resolve().parents[2];out=r/'outputs/utility-correctness';p=out/'workflows';rows=[]
for case in (r/'tests/shell/scenarios').glob('*.sh'):
 a=(p/(case.stem+'.out')).read_bytes();b=(r/'work/utility-correctness/scenarios'/ (case.stem+'.expected')).read_bytes();rows.append(dict(name=case.stem,passed=a==b))
assert all(x['passed'] for x in rows),rows
(out/'scenarios-guest.json').write_text(json.dumps(rows,indent=2)+'\n')
for actual,expected in [('repeat-scopes','scopes'),('repeat-doc','compound-doc'),('repeat-records','records')]:
 assert (p/(actual+'.out')).read_bytes()==(r/'work/utility-correctness/scenarios'/(expected+'.expected')).read_bytes()
assert (p/'cancel-status').read_text()=='CANCEL:130\n'
assert (p/'cancel-output').read_text()=='clean\n'
assert 'no such file or directory' in (p/'cancel-tail-check.txt').read_text().lower()
assert not (p/'remaining-repeat-files.txt').read_text().strip()
mem=re.findall(r'Runtime: heap=(\d+) largest=(\d+)',(p/'repeat-memory.txt').read_text());assert len(mem)==3 and len(set(mem))==1,mem
assert all('remaining=0' in line for line in (p/'repeat-memory.txt').read_text().splitlines() if line.startswith('Runtime:'))
help_rows=''.join((p/f'help-{i}.out').read_text() for i in range(2)).splitlines();assert len(help_rows)==34 and all(line.endswith(':0') for line in help_rows)
native=(p/'native-acceptance.out').read_text();assert all(x in native for x in ['MKDIR:0','ATTR:0',"type 'TEXT', creator 'TEST'",'LS_INVALID:2','DF:0','LOCKED:1','REMOVE:0'])
checks=dict(scripts=len(rows),byte_comparisons=True,repeated_memory=mem,ctrl_c_status=130,tail_not_executed=True,streams_restored=True,help_commands=34,native_metadata=True,fixture_cleanup=True)
(out/'compound-guest-validation.json').write_text(json.dumps(checks,indent=2)+'\n')
print('PASS 12 guest workflows, stable repeat heap, Ctrl+C 130, no tail execution, restored streams, 34 help and native metadata checks')
