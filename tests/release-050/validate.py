"""Check the generated candidate and actual captured guest evidence."""
from pathlib import Path
import hashlib,json,re
root=Path(__file__).resolve().parents[2]
evidence=root/'outputs/release-050-candidate'
manifest=json.loads((evidence/'manifest.json').read_text())
assert len(manifest)==47 and len({e['name'] for e in manifest})==47
for entry in manifest:
 data=(root/'work/distribution/kit/Utilities'/entry['name']).read_bytes()
 assert data[:4]==b'CLX1' and len(data)==entry['bytes']
 assert hashlib.sha256(data).hexdigest()==entry['sha256']
s=(evidence/'guest-results.txt').read_text()
for name in ('ipconfig','ping','wget','ftp','telnet'):
 assert 'check-'+name+':0\n' in s and 'help-'+name+':0\n' in s
for n in (4,5,6):assert f'get-{n}:0\n' in s and f'put-{n}:0\n' in s
for text in ('AWK_READY\n','exists:1\n','http:0\n','bytes:0\n','RC_DONE\n'):assert text in s
assert set(re.findall(r'remaining=(\d+)',s))=={'0'}
assert json.loads((evidence/'binary-integrity.json').read_text())['byte_equal'] is True
print('PASS: 47 packaged images match manifest; actual guest checks/help, FTP/HTTP statuses, pipeline/awk and tracked cleanup evidence.')
