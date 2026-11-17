"""Validate recorded guest/host evidence; does not manufacture guest acceptance."""
from pathlib import Path
import json,re
r=Path(__file__).resolve().parents[2];o=r/'outputs/distribution'
read=lambda n:(o/n).read_text()
assert '77 loader/installer scenarios' in read('host-tests.txt')
assert json.loads(read('core-census.json'))['clx_payload']==0
expected=['setup:0','kept:0','damaged-check:126','repair:0','hello','recovery:0','brokenblocked:1','partial:1','unfinishedresumed:0','reserve:0','low-setup:1','retry:0','awk recovered','DISTRIBUTION_DONE']
assert read('d-results.txt').splitlines()==expected
assert not read('d-errors.txt')
assert 'insufficient application memory' in read('d-low-error.txt')
assert read('cancel-status.txt').strip()=='130'
assert read('cancel-retry-status.txt').strip()=='0'
assert not read('cancel-retry-error.txt')
assert read('d-pipeline.txt')=='a\nb\n' and read('d-awk.txt')=='awk works\n'
assert json.loads(read('metadata.json'))['exact_recovered_macbinary']
assert not read('media-errors.txt')
assert read('final-status.txt').strip()=='0'
assert read('final-awk.txt')=='ready\n'
assert 'appinstall setup DIRECTORY' in read('final-missing.txt')
assert 'Installed awk\n' in read('final-setup.txt')
assert len(re.findall(r'^Installed ',read('final-setup.txt'),re.M))==43
assert read('final-errors.txt').startswith('real ') and len(read('final-errors.txt').splitlines())==1
for n in ['final-after.txt','repeat-memory.txt','cancel-memory.txt','d-final.txt']:
 assert 'remaining=0' in read(n)
print('PASS: external collection, 43-command fresh setup, ordinary execution, damaged preservation, repair/recovery, real heap pressure, Ctrl+C, metadata and missing-command guidance')

assert 'install_commands' not in (r/'src/shell.c').read_text()
assert 'Commands.r' not in (r/'src/CMakeLists.txt').read_text()
assert len(json.loads(read('installed.json')))==43
assert read('final-restart-status.txt').strip()=='0'
assert read('final-restart-awk.txt')=='restart works\n'
assert not read('final-restart-errors.txt')
assert 'Installed ' not in read('final-restart.txt')
assert '126 bad CLX' in read('final-help.txt')
print('PASS: normal final-build restart, no reinstallation, accurate help, installed content and startup source guards')
