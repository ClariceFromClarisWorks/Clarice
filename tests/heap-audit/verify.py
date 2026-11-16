"""Validate exported build-1545 evidence; does not run or imply hardware tests."""
from pathlib import Path
import re,json
r=Path(__file__).resolve().parents[2];o=r/'outputs/heap-audit'
def read(n):return json.loads((o/(n+'.json')).read_text()) if n=='final-errors' else (o/n).read_text()
def memory(n):return [dict(heap=int(a),largest=int(b)) for a,b in re.findall(r'heap=(\d+) largest=(\d+)',read(n))]
a=memory('fixed-new-memory');assert len(a)==5 and len({x['heap'] for x in a[1:]})==1
b=memory('fixed-empty');assert len(b)==10 and len({x['heap'] for x in b})==1
walk=[]
for i in range(1,5):
 m=re.search(r'walk valid=1 free=\d+/\d+ ptr=(\d+)/(\d+) handle=(\d+)/(\d+)',read(f'fixed-walk-{i}'));assert m
 walk.append(list(map(int,m.groups())))
assert len({(x[0],x[2],x[3]) for x in walk})==1
s=read('editor-env-results');assert s.count('RESULT:0')==5 and s.count('remaining=0')==5 and not read('editor-env-errors')
e=memory('editor-env-results');assert max(x['heap'] for x in e)-min(x['heap'] for x in e)<=2 and len({x['largest'] for x in e})==1
s=read('final-results');assert s.count('FAILURE:2')==6 # three import failures + three AWK_FAILURE labels
assert s.count('RECOVERY:0')==3 and 'EXHAUSTION:2' in s and 'AFTER_EXHAUSTION:0' in s
assert s.count('remaining=0')==11 and s.count('LARGE:2')==3
assert read('final-errors').count('editor: insufficient memory importing environment')==3
n=memory('nano-matched');assert len(n)==3 and n[1]==n[2] and n[0]['heap']<=n[1]['heap']
assert read('nano-matched').count('NANO:0')==3 and read('nano-matched').count('remaining=0')==3
assert read('heap-cancel-status')=='CANCEL:130\n'
assert read('heap-cancel-out').startswith('499\nRECOVERY:0\n') and 'remaining=0' in read('heap-cancel-out') and 'STDOUT_RESTORED' in read('heap-cancel-out')
assert read('heap-cancel-err').startswith('STDERR_RESTORED\n')
p=json.loads(read('practical.json'));assert len(p)==6 and all(x['status']==0 and x['matches_dash'] for x in p)
(o/'summary.json').write_text(json.dumps(dict(build=2026111545,platform='Snow Plus / 4 MB / reported 6.0.7 / fixed uncapped',physical_acceptance=False,awk_fixed=a,awk_empty=b,heap_walk_counts=walk,editor=e,nano=n,cancel_status=130,post_cancel_array_entries=500,exhausted_array_request=1000,exhaustion_status=2,upstream_variants=6),indent=2)+'\n')
print('PASS native argv ownership, editor imports/failure/recovery, matched nano cleanup, Ctrl+C and six upstream variants')
