"""Verify extracted disposable Snow results and the current sealed package."""
from pathlib import Path
import struct,sys,json
p=Path(sys.argv[1])
before=(p/'fixture.bin').read_bytes();after=(p/'after.bin').read_bytes()
def forks(b):
 d,r=struct.unpack_from('>II',b,83);offset=128+((d+127)//128)*128
 return b[128:128+d],b[offset:offset+r]
assert forks(before)==forks(after)
assert after[65:73]==b'SIT!SITx'
for a,b in [(73,81),(91,99),(101,102)]:assert before[a:b]==after[a:b],(a,b)
s=(p/'guest.txt').read_text()
for text in ['HFSATTRIB-END','/bin/hfsattrib','old-status=127','shorthand=0','invalid=2','invalid-option=2','invalid-mixed=2','directory-status=1',"type='sit!' creator='sitX'",'flags=6010 locked=off invisible=on','flags=2010 locked=on invisible=off']:
 assert text in s,text
runtime=[x for x in s.splitlines() if x.startswith('Runtime:')]
assert len(runtime)==3 and all('remaining=0' in x and 'owned-peak=0' in x for x in runtime)
def memory(line):return line.split(' image=')[0]
assert memory(runtime[-1])==memory(runtime[-2])
c=(p/'cancel.txt').read_text();assert 'cancel=130' in c and 'remaining=0' in c and "type='SIT!' creator='SITx'" in c
seal=json.loads(Path('work/release/sealed.json').read_text());names=[x['name'] for x in seal['commands']]
assert 'hfsattrib' in names and 'hfsattr' not in names and len(names)==49
print('PASS Snow syntax/options/errors, cancellation/recovery, stable repeated heap, zero owned allocations; both forks and unrelated metadata exact; sealed49-command rename')
