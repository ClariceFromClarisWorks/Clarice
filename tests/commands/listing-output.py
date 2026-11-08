from pathlib import Path
import runpy,sys
p=Path('outputs/ls-opt2')
# Reuse metadata assertions against this run, preserving earlier evidence.
source=Path('tests/commands/visibility-validate.py').read_text().replace("outputs/ls-metadata","outputs/ls-opt2")
exec(compile(source,'visibility-validate.py','exec'))
lines=(p/'ls-opt.txt').read_text().splitlines()
assert len(lines)==1002,len(lines)
assert [line.split()[-1] for line in lines[2:]]==[f'file{i:04d}.txt' for i in range(1000)]
for line in lines[2:]:
 fields=line.split();assert fields[:3]==['---','35','0'],line
 assert fields[3:5]==['2026-11-08','05:53'],line
for name in ('native-light.txt','native-dark.txt'):
 s=(p/name).read_text();assert 'PASS' in s and '0 pixel mismatches' in s and '0 premature screen changes' in s
s=(p/'scroll-quick.txt').read_text();assert 'SCROLL_TEST PASS (0 failures)' in s
print('PASS: 1,000 distinct complete rows, both fork lengths and creation dates; light/dark native pixel reports and quick renderer regression')
