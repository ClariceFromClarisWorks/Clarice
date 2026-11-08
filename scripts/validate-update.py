#!/usr/bin/env python3
from pathlib import Path
import struct,json,re,hashlib
root=Path(__file__).resolve().parents[1];out=root/'outputs/update1'
def mb(path):
 b=path.read_bytes();d,r=struct.unpack('>II',b[83:91]);off=128+(d+127)//128*128
 return b[128:128+d],b[off:off+r],b[65:73]
before=mb(root/'work/emulator/fixture.bin');after=mb(out/'fork-after.bin')
report=(out/'guest-report.txt').read_text();heap=re.search(r'heap before (\d+) after (\d+)',report)
h=(out/'history.bin').read_bytes();magic,count=struct.unpack('>II',h[:8]);seqs=[];records=[]
for seq in range(count-127,count+1):
 pos=8+(seq-1)%128*1028;seqs.append(struct.unpack('>I',h[pos:pos+4])[0]==seq);records.append(h[pos+4:pos+1028].split(b'\0')[0])
p=(out/'preferences.bin').read_bytes()
checks={'guest_regression_pass':'PHASE1_CHECKS PASS' in report and 'FAIL' not in report,'guest_statuses_zero':all(int(x)==0 for x in re.findall(r'\[status (-?\d+);',report)),'guest_30_scripts_exact':(out/'guest-shell-results.txt').read_bytes()==(root/'tests/shell/expected.txt').read_bytes(),'host_30_scripts_pass':all(r['pass'] for r in json.loads((out/'host-shell-results.json').read_text())),'heap_140_commands_stable':bool(heap and heap[1]==heap[2]),'data_fork_result':after[0]==b'preserved\n','resource_fork_unchanged':after[1]==before[1],'type_creator_unchanged':after[2]==before[2]==b'TEXTTEST','redirect_append_result':(out/'result.txt').read_bytes()==b'alpha beta\nready\n','history_size_capped':len(h)==131592,'history_header_and_sequences':magic==0x53364831 and all(seqs),'history_clean_exit':records[-1]==b'exit','preferences_valid':len(p)==44 and p[:4]==b'S6P2','saved_font_monaco_9':p[4]==9 and p[8:15]==b'\x06Monaco','saved_light_theme_screen_pager':p[5:7]==b'\x00\x01'}
result={'checks':checks,'pass':all(checks.values()),'heap_before_after':list(map(int,heap.groups())) if heap else [],'history_bytes':len(h),'history_sequence':count,'preferences_bytes':len(p),'resource_fork_sha256':hashlib.sha256(after[1]).hexdigest(),'macbinary_bytes':(out/'Sys6Unix.bin').stat().st_size}
(out/'validation.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result,indent=2));assert result['pass']
