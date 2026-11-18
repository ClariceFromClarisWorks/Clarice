"""Check extracted complete output independently of the guest's cmp command."""
from pathlib import Path
r=Path('work/ls-catalog')
outputs=[(r/f'out-{v}.txt').read_bytes() for v in ('base','frmt','next')]
assert outputs[0]==outputs[1]==outputs[2]
lines=outputs[0].decode().splitlines();assert len(lines)==1002
for i,line in enumerate(lines[2:]):
    f=line.split()
    assert f[0]=='---' and int(f[1])==[0,1,8192,32768][i%4] and f[2]=='0' and f[-1]==f'f{i:04d}',line
names=(r/'names-base.txt').read_bytes()
assert names==(r/'names-next.txt').read_bytes()
assert names.decode().splitlines()==['.','..']+[f'f{i:04d}' for i in range(100)]
print('PASS: 1002 exact detailed rows, expected fork lengths and distinct names; 102 exact name-only rows')
