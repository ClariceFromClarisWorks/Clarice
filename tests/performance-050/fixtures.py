"""Deterministic disposable inputs and complete guest command timing script."""
from pathlib import Path
import json
r=Path(__file__).resolve().parents[2];w=r/'work/performance-050';w.mkdir(exist_ok=True)
for size,n in [('small',8),('ordinary',128),('large',1024)]:
 (w/size).write_text(''.join(f'{i%97:04d}: alpha beta gamma {i%11:04d} payload abcdefghijklmnopqrstuvwxyz\n' for i in range(n)))
(w/'binary').write_bytes(bytes(range(256))*256)
cases=[]
for size in ['small','ordinary','large']:
 p='/mnt/Audit/'+size
 for name,cmd in [('cat',f'cat {p}'),('grep',f'grep alpha {p}'),('sed',f"sed 's/alpha/omega/g' {p}"),('awk',f"awk '{{n+=length($0)}} END {{print n}}' {p}"),('sort',f'sort {p}'),('cut',f"cut -d : -f 1 {p}"),('tr',f"tr a-z A-Z < {p}"),('wc',f'wc {p}'),('cmp',f'cmp {p} {p}'),('cksum',f'cksum {p}'),('cp',f'cp {p} /tmp/perf-copy')]:cases.append((name+'-'+size,cmd+' > /dev/null'))
for n in ['ten','hundred']:
 for name,args in [('ls','-la'),('find',''),('du','')]:cases.append((name+'-'+n,f'{name} {args} /mnt/Audit/{n} > /dev/null'))
cases += [('df','df > /dev/null'),('dd','dd if=/mnt/Audit/binary of=/tmp/perf-dd bs=4096 > /dev/null'),('pipeline','cat /mnt/Audit/ordinary | grep alpha | wc -l > /dev/null'),('shell','sh /mnt/Audit/shell > /dev/null')]
(w/'shell').write_text('f() { printf "%s\\n" "$1"; }; for i in 1 2 3 4 5 6 7 8 9 0; do x=$(f "$i"); test "$x" = "$i" || exit 1; done\n')
case_dir=w/'cases';case_dir.mkdir(exist_ok=True)
for label,cmd in cases:
 lines=['for repeat in 0 1 2; do',f'echo CASE {label} "$repeat"','time '+cmd,'echo STATUS $?','status runtime','done']
 (case_dir/label).write_text('\n'.join(lines)+'\n')
(w/'baseline.sh').write_text('prefs paging off\necho AUDIT_BEGIN\nfor case in /mnt/Audit/cases/*; do sh "$case"; done\necho AUDIT_DONE\n')
(w/'profile').write_text('export PATH=/bin:/usr/bin\nsh /mnt/Audit/baseline.sh > /mnt/Audit/baseline.txt 2>&1\n')
(w/'cases.json').write_text(json.dumps(cases,indent=2)+'\n')

# Matched candidate subset; other case scripts use the same low-footprint driver.
after=w/'after-cases';after.mkdir(exist_ok=True)
for p in case_dir.iterdir():
 if p.name.startswith(('find-','du-','tr-','wc-')):(after/p.name).write_bytes(p.read_bytes())
more={'head':'head /mnt/Audit/large > /dev/null','tail':'tail /mnt/Audit/large > /dev/null','uniq':'uniq /mnt/Audit/large > /dev/null','tee':'tee /tmp/perf-tee < /mnt/Audit/large > /dev/null','od':'od -A x -t x1 -N 4096 /mnt/Audit/binary > /dev/null','file':'file /mnt/Audit/binary > /dev/null','more':'more /mnt/Audit/ordinary > /dev/null','mv':'mv /tmp/perf-copy /tmp/perf-moved','rm':'rm /tmp/perf-moved'}
for name,cmd in more.items():
 setup='cp /mnt/Audit/small /tmp/perf-copy\n' if name=='mv' else 'cp /mnt/Audit/small /tmp/perf-moved\n' if name=='rm' else ''
 (after/name).write_text('for repeat in 0 1 2; do\n'+setup+f'echo CASE {name} "$repeat"\ntime '+cmd+'\necho STATUS $?\nstatus runtime\ndone\n')
(w/'after.sh').write_text('prefs paging off\necho AUDIT_BEGIN\nfor case in /mnt/Audit/after-cases/*; do sh "$case"; done\necho AUDIT_DONE\n')
(w/'utf').write_bytes(('alpha café 世界\n\t beta\n'*100).encode()+b'\x00\xff\xc3\n')
