#!/usr/bin/env python3
from pathlib import Path
import hashlib, zipfile
root=Path(__file__).resolve().parents[1];out=root/'outputs/update1'
files=[root/'README.md',root/'THIRD_PARTY.md',root/'AGENTS.md',root/'outputs/ARCHITECTURE.md']
for directory in ('src','assets/font','licenses','scripts','tests'):
    files += [p for p in (root/directory).rglob('*') if p.is_file() and '__pycache__' not in p.parts]
for name in ('Sys6Unix.bin','Sys6Unix.dsk','TEST-REPORT.md','benchmarks.json','validation.json','guest-report.txt','guest-shell-results.txt','host-shell-results.json','result.txt','benchmark-1000.png','finder-while-paging.png','build.log'):
    files.append(out/name)
manifest=''.join(hashlib.sha256(p.read_bytes()).hexdigest()+'  '+str(p.relative_to(root))+'\n' for p in sorted(files))
(out/'SHA256SUMS.txt').write_text(manifest);files.append(out/'SHA256SUMS.txt')
archive=out/'Sys6Unix-update1.zip'
with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED) as z:
    for p in sorted(files):z.write(p,Path('Sys6Unix')/p.relative_to(root))
with zipfile.ZipFile(archive) as z:
    assert z.testzip() is None
    assert not any('/work/' in n or n.endswith('.ROM') or n.endswith('.hda') for n in z.namelist())
print(f'{archive}: {archive.stat().st_size} bytes; {len(files)} files; archive integrity OK')
