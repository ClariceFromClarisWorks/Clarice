"""Check extracted QOL-1 guest transcripts, never simulated guest output."""
from pathlib import Path
p=Path('outputs/qol1')
b=p/'before'
assert (b/'pwd.txt').read_text() == '/mnt/Test 2\n'
assert (b/'escaped.txt').read_text() == '/mnt/Test 2\n'
assert (b/'read.txt').read_text() == 'SECOND-VOLUME\n'
for n in ('relative','absolute'):
    s=(b/(n+'.txt')).read_text()
    assert 'System Folder/' in s and 'sentinel.txt' not in s
s=(p/'after/regression.txt').read_text()
assert s.endswith('QOL-REGRESSION-DONE\n'),s
assert 'System Folder/' not in s
assert s.count('/mnt/Test 2\n') == 5
assert '/mnt/Test 2/sub\n' in s and '/mnt/Sys6Unix\n' in s
assert s.count('second.txt\n') == 2
assert 'SECOND-VOLUME\n' in s
assert s.count('BOOT-GUARD\n') == 3
assert s.count('SECOND\n') == 5
assert 'guard\nrenamed\ntouched\n' in s
assert 'from-second\nguard\n' in s
for name in ['invalid-cd','invalid-ls','invalid-write','invalid-rm','absent-cd','absent-ls','absent-write']:
    assert name+('=2\n' if name.endswith('write') else '=1\n') in s,name
assert 'Mac OS error -35' in s and 'Mac OS error -43' in s
assert '1000 entries, 3 runs' in s
assert (p/'after/boot-guard.txt').read_text() == 'BOOT-GUARD\n'
assert (p/'after/copied.txt').read_text() == 'SECOND\n'
# Existing namespace fixtures: mounted-volume set has one extra known disk.
paths=(p/'after/paths.txt').read_text().replace('Test 2/\n','')
assert paths==Path('tests/namespace/paths-expected.txt').read_text(),repr(paths)
assert (p/'after/links.txt').read_text()==Path('tests/namespace/links-expected.txt').read_text()
print('PASS: reproduced old cross-volume listings; corrected roots/nested paths, quoting, cd/pwd, safe mutations, links, errors and existing namespace fixtures')
