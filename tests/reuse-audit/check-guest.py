from pathlib import Path
p=Path("outputs/reuse-audit")
# Verify actual guest output, not just the marker.
g=p/'guest'
assert (g/'status.txt').read_text().split()==['0','1']+['0']*12
assert (g/'range.txt').read_text().splitlines()==['/tmp/reuse-fixture/file1.txt','/tmp/reuse-fixture/sub/file23.txt']
for name,expected in [('class','9beta.log'),('escape','a*.txt'),('literal','literal[.txt')]:
 assert (g/(name+'.txt')).read_text().strip()=='/tmp/reuse-fixture/'+expected
assert len((g/'neg.txt').read_text().splitlines())==6
assert 'error -43' in (g/'error.txt').read_text()
assert '[' in (g/'help.txt').read_text()
print('PASS extracted guest outputs, statuses, help and error')
