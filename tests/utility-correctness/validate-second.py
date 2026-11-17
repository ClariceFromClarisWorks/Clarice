"""Assert saved actual Snow evidence; does not run the emulator."""
from pathlib import Path
import json,hashlib,re,subprocess
r=Path(__file__).resolve().parents[2];g=json.loads((r/'outputs/utility-expansion/second-guest.json').read_text())
statuses=[0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,1,1,0,0,0,0,0,0,0,0]
for i,rc in enumerate(statuses):assert int(g[f'b{i}.status'])==rc,(i,g[f'b{i}.status'])
for i,text in {0:'',1:'child\nparent\n',2:'/custom\n',3:'1\n',4:'HOME=/only\n',5:'1\n',6:'1\n',7:'alive\n',14:'keep',18:'created:0\nremoved:1\n',26:'one\n',29:'/large\n',30:'one\n\n',31:'failed:1\nsafe',32:'failed:1\nlocked',36:'2050\n'}.items():assert g[f'b{i}.out']==text,(i,g[f'b{i}.out'])
for i in [9,10,20,21,23,24,25]:assert not g[f'b{i}.out'] and g[f'b{i}.err'],i
assert not g['b22.out'] and not g['b22.err'] # quiet mktemp failure
assert g['b11.err']=='128+0 records in\n128+0 records out\n'
assert 'out of memory' in g['b31.err'] and 'Permission denied' in g['b32.err']
for i in [6,27]:assert g[f'b{i}.err'].startswith('real ')
assert int(g['cancel-dd'])==int(g['cancel-sleep'])==130 and 'remaining=0' in g['after-cancel']
files={'copy':bytes(range(256))*512,'zeros':bytes(8192),'seek':bytes(26)+bytes(range(35)),'gap':b'prefix'+bytes(304)+bytes(range(31)),'empty':b'','keep':b'keep','oomkeep':b'safe','locked':b'locked'}
for n,data in files.items():
 actual=g['_binary'][n];assert actual['bytes']==len(data) and actual['sha256']==hashlib.sha256(data).hexdigest() and actual['byte_comparison_pass'],n
for i,data in [(12,files['zeros']),(13,(bytes(range(256))*3)[:548]),(15,files['seek']),(16,files['gap']),(17,b'')]:
 ref=subprocess.run(['/usr/bin/cksum'],input=data,capture_output=True,check=True).stdout.decode().split();assert g[f'b{i}.out'].split()[:2]==ref[:2],i
heap=[int(x) for x in re.findall(r'heap=(\d+)',g['second-repeat'])];largest=[int(x) for x in re.findall(r'largest=(\d+)',g['second-repeat'])]
assert len(heap)==6 and max(heap)-min(heap)<=16 and max(largest)-min(largest)<=256
assert g['second-repeat'].count('remaining=0')==6
for i in [33,34,35]:assert 'remaining=0' in g[f'b{i}.out']
print('PASS: 37 Snow utility cases, eight byte comparisons, CRC references, native OOM/locked files, cancellation130 and bounded repeat heap')
