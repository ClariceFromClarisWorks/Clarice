from pathlib import Path
import struct,json
r=Path(__file__).resolve().parents[2];p=r/'outputs/utility-correctness'
checks={}
def expect(file,items):
 s=(p/file).read_text()
 for item in items:assert item in s,(file,item)
 checks[file]=True
expect('resources.txt',['COPY:0','REMOVE:0','ABSENT:1','COPY_OOM:1','COPY_CANCEL:130','REMOVE_OOM:1','REMOVE_CANCEL:130','RECOVER:0','FORKS:0'])
expect('recovery.txt',['WRITE_FAIL:1','WRITE_CANCEL:130','CAPACITY:2','SEQUENTIAL1:0','SEQUENTIAL2:0','SEQUENTIAL3:0'])
expect('stream-recovery.txt',['STREAM_OOM:2','STREAM_CANCEL:130','STREAM_RECOVERY:0','AWK:0','AWK_REPEAT:0'])
for name in ['resources.txt','recovery.txt','stream-recovery.txt']:
 for line in (p/name).read_text().splitlines():
  if 'remaining=' in line:assert line.endswith('remaining=0')
  if 'live=' in line:assert line.endswith('live=0')
assert len((p/'copied-list').read_text().splitlines())==26
assert not any('Clv tmp' in x for x in (p/'after-recovery.txt').read_text().splitlines())
def forks(path):
 b=path.read_bytes();d,n=struct.unpack('>II',b[83:91]);off=128+((d+127)//128)*128
 return b[128:128+d],b[off:off+n],b[65:83],b[91:102]
a=forks(r/'work/userland-fixtures/fork-source.bin');b=forks(p/'fork-copy.bin');c=forks(p/'fork-recovery.bin')
assert a[:2]==b[:2] and a[2][:8]==b[2][:8] and b==c
for i in range(1,17):assert (r/f'work/utility-correctness/awk-out-{i}').read_bytes()==f'{i}\n'.encode()
checks.update(deep_copy_remove=True,copy_failure_forks_metadata=True,cleanup=True,awk_16_exact_outputs=True)
(p/'resources-validation.json').write_text(json.dumps(checks,indent=2)+'\n')
print('PASS guest resource assertions, forks/metadata, cancellation/failure statuses, cleanup and 16 exact awk files')
