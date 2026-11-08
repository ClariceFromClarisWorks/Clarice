#!/usr/bin/env python3
from pathlib import Path
import hashlib, zipfile
root=Path(__file__).resolve().parents[1]; out=root/'outputs/scrolling'
files=[root/n for n in ('README.md','THIRD_PARTY.md','AGENTS.md','outputs/ARCHITECTURE.md')]
for directory in ('src','assets/font','licenses','scripts','tests'):
    files += [p for p in (root/directory).rglob('*') if p.is_file() and '__pycache__' not in p.parts]
files += [p for p in out.iterdir() if p.is_file() and p.suffix in ('.md','.json','.txt','.png','.bin','.dsk','.log') and p.name!='SHA256SUMS.txt']
(out/'SHA256SUMS.txt').write_text(''.join(hashlib.sha256(p.read_bytes()).hexdigest()+'  '+str(p.relative_to(root))+'\n' for p in sorted(files)))
files.append(out/'SHA256SUMS.txt')
archive=out/'Sys6Unix-scrolling.zip'
with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED) as z:
    for p in sorted(files): z.write(p,Path('Sys6Unix')/p.relative_to(root))
with zipfile.ZipFile(archive) as z:
    assert z.testzip() is None
    assert not any('/work/' in n or n.endswith(('.ROM','.hda')) for n in z.namelist())
print(f'{archive}: {archive.stat().st_size} bytes; {len(files)} files; integrity OK')
