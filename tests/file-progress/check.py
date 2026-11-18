from pathlib import Path
import re,json,hashlib,statistics,struct
p=Path('outputs/file-progress');text=(p/'guest-results.txt').read_text();marks=list(re.finditer(r'(?m)^copy:(\d+):(before|after)\n',text));rows=[]
for i,m in enumerate(marks):
 b=text[m.end():marks[i+1].start() if i+1<len(marks) else len(text)]
 assert re.search(r'^status:0$',b,re.M)
 seconds=float(re.search(r'real ([0-9.]+) s',b)[1]);r=re.search(r'Runtime: heap=(\d+) largest=(\d+) image=(\d+) ticks=(\d+) load=(\d+) owned-peak=(\d+) remaining=(\d+)',b)
 assert r and r[7]=='0';rows.append(dict(run=int(m[1]),variant=m[2],seconds=seconds,heap=int(r[1]),largest=int(r[2]),image=int(r[3]),ticks=int(r[4]),load=int(r[5]),peak=int(r[6]),remaining=int(r[7])))
assert len(rows)==8
for field,expected in [('compare',0),('rename',0),('cross-move',0),('moved-data',0),('tree-copy',0),('tree-remove',0),('tree-absent',0),('locked-copy',1),('locked-remove',1),('unlock-remove',0),('quiet-copy',0),('invalid',2),('quiet-remove',0)]:
 values=re.findall(r'^'+field+r':(\d+)$',text,re.M);assert values and all(int(v)==expected for v in values),(field,values)
for v in ['before','after']:
 a=[r['seconds'] for r in rows if r['variant']==v];print(v,'first',a[0],'repeats',a[1:],'median',statistics.median(a[1:]))
(p/'measurements.json').write_text(json.dumps(rows,indent=2)+'\n')
def forks(path):
 b=path.read_bytes();n,q=struct.unpack_from('>II',b,83);o=128+((n+127)//128)*128
 assert len(b)>=o+q
 return dict(data_size=n,resource_size=q,data_sha256=hashlib.sha256(b[128:128+n]).hexdigest(),resource_sha256=hashlib.sha256(b[o:o+q]).hexdigest(),type_creator=b[65:73].hex(),timestamps=b[91:99].hex())
a=forks(Path('work/file-progress/fp-source.bin'));b=forks(Path('work/file-progress/moved.bin'))
for key in a:
 if key!='resource_sha256':assert a[key]==b[key],key
# Record the failed full-fork comparison, including baseline evidence. Do not
# silently normalize resource bytes or claim complete fork preservation.
source=Path('work/file-progress/fp-source.bin').read_bytes()[262272:]
base=Path('work/file-progress/before-export.bin').read_bytes()[262272:]
moved=(Path('work/file-progress/moved.bin')).read_bytes()[262272:]
diffs=lambda data:[i for i,(x,y) in enumerate(zip(source,data)) if x!=y]
issue=dict(full_resource_match=source==moved,baseline_differences=diffs(base),candidate_differences=diffs(moved),candidate_resource_sha256=b['resource_sha256'])
assert len(base)==len(moved)==len(source)
assert diffs(base)==diffs(moved)==list(range(48,126))
a['resource_validation']=issue
(p/'fork-validation.json').write_text(json.dumps(a,indent=2)+'\n')
print('PASS eight timed copies, statuses, data comparisons, rename, cross-volume move, recursive copy/remove, locks, quiet flag, invalid options; persisted data fork/type/creator/timestamps match; resource fork differs at bytes 48..125 in both baseline and candidate (system-reserved area; valid-file acceptance is in resource/check.py)')
