"""Validate the extracted disposable Snow fixture transcript, not simulated output."""
from pathlib import Path
s=Path('outputs/catalog-perf/catalog-result.txt').read_text()
assert s.endswith('CATALOG-GUEST-DONE\n'),s
assert s.count('/mnt/Test 2/catalog-test\n')==1,s
assert '/tmp/catalog-test\n' in s
assert 'System Folder/' not in s
assert 'guard\npercent%25name\nspace name\nsub/\n' in s,s
assert '.hidden' in s and 'SECOND\nBOOT\n' in s,s
assert 'absent=1\nmissing=1\n' in s,s
assert s.count('BOOT\n')==2,s
p=Path('outputs/catalog-perf/native-scroll-report.txt').read_text()
assert 'NATIVE_SCROLL_TEST PASS' in p,p
assert '0 pixel mismatches' in p and '0 staging/reference mismatches' in p,p
print('PASS: actual Snow two-volume listing transcript, hidden/escaped names, relative/absolute paths, safe file operations, fail-closed errors; native pixel and scrollbar reference checks')
