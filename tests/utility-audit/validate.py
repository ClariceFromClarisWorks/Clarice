from pathlib import Path
import json,re,statistics
r=Path(__file__).resolve().parents[2];p=r/'outputs/utility-audit/guest'
expected={'small':b'a\na\nb\nz\n','long':b'a'*20001+b'\n'+(b'z'*20000+b'\n')*2,'unique':b'a'*20001+b'\n'+b'z'*20000+b'\n','record':b'x'*65536+b'\n','length':b'65536\n','fields':b'','oom-sort':b'','oom-awk':b'','recovery-sort':b'a\na\nb\nz\n','recovery-awk':b'1\n1\n1\n1\n'}
for name,data in expected.items():
 assert (p/f'{name}.out').read_bytes()==data,name
 if name in ('fields','oom-sort','oom-awk'):assert (p/f'{name}.err').stat().st_size,name
 else:assert not (p/f'{name}.err').stat().st_size,name
s=(p/'evidence.txt').read_text();assert 'ELASTIC_DONE' in s
for chunk in s.split('CASE ')[1:]:
 name=chunk.splitlines()[0];status=int(re.search(r'STATUS (\d+)',chunk)[1]);assert status==(2 if name in ('fields','oom-sort','oom-awk') else 0),(name,status)
 assert 'remaining=0' in chunk,name
b=(p/'bench.txt').read_text();assert 'BENCH_DONE' in b
rows=[]
for chunk in b.split('CASE ')[1:]:
 label,repeat=chunk.splitlines()[0].rsplit(' ',1);values={k:int(v) for k,v in re.findall(r'([\w-]+)=(\d+)',chunk)};rows.append({'case':label,'repeat':int(repeat),'values':values})
assert len(rows)==18
out={'runs':rows,'warm_medians':{}}
for label in sorted({x['case'] for x in rows}):
 group=[x['values'] for x in rows if x['case']==label and x['repeat']>0]
 out['warm_medians'][label]={k:statistics.median(x[k] for x in group) for k in ['ticks','load','read','prepare','image-alloc','meta-alloc','record','imports','relocations','validate','payload','metadata','stdio']}
(r/'outputs/utility-audit/ls-snow.json').write_text(json.dumps(out,indent=2)+'\n')
print(json.dumps(out['warm_medians'],indent=2));print('PASS guest exact byte comparison, explicit low-memory errors, recovery, zero owned allocations; 18 ls invocations')
