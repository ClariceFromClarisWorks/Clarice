#!/usr/bin/env python3
"""Validate saved QOL-2 guest evidence, not a substitute for running the guest."""
from pathlib import Path
import re
root=Path(__file__).resolve().parents[2]
p=root/'outputs/qol2'
s=(p/'final.txt').read_text()
rows=[dict((k,int(v)) for k,v in re.findall(r'(\w+)=(\d+)',line)) for line in s.splitlines() if line.startswith('stage=')]
assert len(rows)==4
for r in rows:
    assert r['posted']==r['received']==r['retained']==240 and r['correct']==1,r
assert s.count('\nPASS ')==20 and 'SAFETY-DONE failures=0' in s and 'PROBE-DONE' in s
assert (p/'host-input-result.txt').read_text()==''.join(chr(97+i%26) for i in range(180))+'\n'
print('PASS: four guest rates, 20 safety checks, exact physical-keyboard output')
