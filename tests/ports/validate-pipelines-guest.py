from pathlib import Path
import re
root=Path(__file__).resolve().parents[2]
evidence=root/'outputs/pipelines'
s=(evidence/'guest-results.txt').read_text()
expected='''PIPELINES-START
4 8 40 /tmp/pipeline-data
wc:0
alpha one
beta two
head:0
one
two
two
three
cut:0
ALPHA ONE
BETA TWO
BETA TWO
GAMMA THREE
tr:0
      1 alpha one
      2 beta two
      1 gamma three
uniq:0
3
pipe:0
19
redirect:0
invalid:1
missing:1
bound:2
help:wc:0
help:head:0
help:cut:0
help:tr:0
help:uniq:0
'''
assert s.startswith(expected),s
rows=re.findall(r'Runtime: heap=(\d+) largest=(\d+).*?remaining=(\d+)',s)
assert len(rows)==2 and rows[0]==rows[1] and rows[0][2]=='0',rows
assert s.endswith('PIPELINES-END\n')
cancel=(evidence/'guest-cancel.txt').read_text()
assert cancel.startswith('130\n') and 'remaining=0' in cancel
assert '16384' in (evidence/'guest-error.txt').read_text()
assert 'clarice-pipe-' not in (evidence/'tmp-listing.txt').read_text()
print('PASS: exact Plus output/status fixtures, stable free heap/largest block after 25 repeated tool runs, Ctrl+C status 130, zero retained invocation allocations, no pipeline spools left.')
