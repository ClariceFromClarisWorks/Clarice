from pathlib import Path
import hashlib,struct,json,shutil,re
r=Path('work/task2/automated/results');f=Path('work/task2/guest');o=Path('outputs/task2/guest')
checks=[]
def check(name,ok):checks.append({'check':name,'passed':bool(ok)})
for n,v in {'fresh-wc-status.txt':'0\n','recover-status.txt':'0\n','probe-recover-status.txt':'0\n','repair-status.txt':'0\n','damaged-before-status.txt':'126\n','damaged-restarted-status.txt':'126\n','old-damage-status.txt':'126\n','ls-status.txt':'0\n','df-status.txt':'0\n','incompatible-status.txt':'1\n','accept-install-status.txt':'0\n','accept-replace-status.txt':'0\n','accept-recover-status.txt':'0\n'}.items():check(n,(r/n).read_text()==v)
for n,v in {'grep-before.txt':'alpha\n','grep-restarted.txt':'alpha\n','grep-repaired.txt':'alpha\n','sed.txt':'beta\n','awk.txt':'beta\n','probe-before.txt':'independent command v2\n','probe-restarted.txt':'independent command v2\n','probe-recovered.txt':'independent command v1\n','accept-one.txt':'independent command v1\n','accept-two.txt':'independent command v2\n','accept-recovered.txt':'independent command v1\n'}.items():check(n,(r/n).read_text()==v)
for n in ('updated-grep.clx','restarted-grep.clx','retained-grep.clx'):check(n,(r/n).read_bytes()==(f/'grep2.clx').read_bytes())
for n in ('damaged-before.clx','damaged-restarted.clx','old-damage.clx'):check(n,(r/n).read_bytes()==(f/'corrupt.clx').read_bytes())
b=Path('work/task2/automated/original-grep.bin').read_bytes();n=struct.unpack_from('>I',b,83)[0];check('recovered original grep bytes',(r/'recovered-grep.clx').read_bytes()==b[128:128+n])
check('fresh bundled wc bytes',Path('work/task2/automated/final-wc.clx').read_bytes()==Path('work/task2/automated/original-wc.clx').read_bytes())
meta="type='TEST' creator='TST2' flags=4100 locked=off invisible=on"
for name in ('metadata-before.txt','metadata-replaced.txt','metadata-restarted.txt','metadata-recovered.txt'):check(name,all(line.split(': ',1)[1]==meta for line in (r/name).read_text().splitlines()))
newmeta=(r/'accept-metadata-before.txt').read_text().split(': ',1)[1].strip()
check('fresh replacement Finder metadata',all(line.split(': ',1)[1]==newmeta for line in (r/'accept-metadata-after.txt').read_text().splitlines()))
for name in ('replace-status.txt','probe-install-status.txt','probe-replace-status.txt'):check('repeat refusal '+name,(r/name).read_text()=='1\n')
for name in ('replace.txt','probe-install.txt','probe-replace.txt'):check('repeat refusal diagnostic '+name,'unfinished update or retained previous version' in (r/name).read_text())
for p in r.glob('*.txt'):shutil.copyfile(p,o/p.name)
(o/'results.json').write_text(json.dumps(checks,indent=2)+'\n')
(o/'payload-hashes.json').write_text(json.dumps({p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in r.glob('*.clx')},indent=2)+'\n')
timings=[]
for label,name in [('first','tmoe.txt'),('repeat 1','tmt.txt'),('repeat 2','timing-third.txt')]:
 text=(r/name).read_text();values={k:int(v) for k,v in re.findall(r'([\w-]+)=(\d+)',text)};values.update(run=label,source=name,seconds=values['load']/60);timings.append(values)
(o/'timings.json').write_text(json.dumps(timings,indent=2)+'\n')
print(f'{sum(c["passed"] for c in checks)}/{len(checks)} guest checks passed');print(json.dumps(timings,indent=2));assert all(c['passed'] for c in checks),[c for c in checks if not c['passed']]
