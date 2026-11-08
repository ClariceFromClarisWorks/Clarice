#!/usr/bin/env python3
"""Validate extracted bytes, independently of the guest's PASS marker."""
from pathlib import Path
import struct, json, hashlib, re
root=Path(__file__).resolve().parents[1]
w=root/'work/emulator';out=root/'outputs/phase1'
def macbinary(path):
    b=path.read_bytes();d,r=struct.unpack('>II',b[83:91]);off=128+(d+127)//128*128
    return b[128:128+d],b[off:off+r],b[65:73]
before=macbinary(w/'fixture.bin');after=macbinary(w/'fork-after.bin')
h=(w/'history-final.bin').read_bytes();magic,count=struct.unpack('>II',h[:8]);records=[]
for seq in range(max(1,count-127),count+1):
    pos=8+((seq-1)%128)*1028
    stored=struct.unpack('>I',h[pos:pos+4])[0]
    assert stored==seq,(seq,stored)
    records.append(h[pos+4:pos+1028].split(b'\0')[0].decode('mac_roman'))
report=(out/'guest-report.txt').read_text()
checks={
 'guest_suite_pass':'PHASE1_CHECKS PASS' in report and 'PHASE1_CHECKS FAIL' not in report,
 'guest_selftest_pass':'selftest: PASS' in report,
 'all_reported_statuses_zero':all(int(x)==0 for x in re.findall(r'\[status (-?\d+);',report)),
 'redirect_append_exact':(out/'result.txt').read_bytes()==b'alpha beta\nready\n',
 'data_fork_replaced_exactly':after[0]==b'preserved\n',
 'resource_fork_preserved':after[1]==before[1],
 'finder_type_creator_preserved':after[2]==before[2]==b'TEXTTEST',
 'history_header_valid':magic==0x53364831,
 'history_fixed_size':len(h)==8+128*1028,
 'history_latest_exit':records[-1]=='exit',
 'history_128_committed_slots':len(records)==128,
 'history_wrap_heap_stable':'heap before 241740 after 241740' in report,
}
result={'checks':checks,'pass':all(checks.values()),'history_bytes':len(h),'history_sequence':count,'resource_fork_sha256':hashlib.sha256(after[1]).hexdigest(),'application_macbinary_bytes':(out/'Sys6Unix.bin').stat().st_size}
(out/'validation.json').write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps(result,indent=2))
assert result['pass']
