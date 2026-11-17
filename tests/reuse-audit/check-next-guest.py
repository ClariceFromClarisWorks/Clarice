from pathlib import Path
import hashlib
p=Path('outputs/reuse-next/guest')
assert (p/'http-status.txt').read_text().splitlines()==['length:0','chunked:0','truncated:1','malformed:1','ambiguous:1','404:1','cmp:0','1:0','2:0','3:0']
assert (p/'cancel-status.txt').read_text().strip()=='130'
listing=(p/'hfs-listing.txt').read_text().splitlines()
assert not any(x in listing for x in ['cancel','truncated','malformed','ambiguous','404'])
assert not any(x.startswith('.wget-') for x in listing)
for name in ['length','chunked','recover','repeat-1','repeat-2','repeat-3']:
 b=(Path('work/http-upstream')/(name+'.bin')).read_bytes()
 assert b==bytes(range(256))*16
 print(name,len(b),hashlib.sha256(b).hexdigest())
r=(p/'final-result.txt').read_text();assert r.count('dir:0')==3 and r.count('kind:0')==3 and 'missing:1' in r
# /dev/full was an invalid guest error fixture: it is not a Clarice device.
# Dedicated fault images verify failure after creation instead.
if (p/'fault-result.txt').exists():
 r=(p/'fault-result.txt').read_text();assert 'write:2' in r and 'cancel:130' in r and 'recover:0' in r
 l=(p/'fault-hfs-listing.txt').read_text().splitlines();assert set(l)=={'listing','output','result','name'},l
print('PASS exported HTTP integrity/status/cleanup and mktemp directory cases')
