#!/usr/bin/env python3
"""Restore the accepted nano 9.2 source/header preparation in a fresh checkout.

Does not replace an existing source tree. Patches are applied by link-nano-probe.sh.
The ncurses build remains a separate prerequisite.
"""
from pathlib import Path
import hashlib
import shutil
import tarfile
import urllib.request

root = Path(__file__).resolve().parents[1]
work = root / 'work/nano'
work.mkdir(parents=True, exist_ok=True)
archive = work / 'nano-9.2.tar.xz'
if not archive.exists():
    urllib.request.urlretrieve('https://www.nano-editor.org/dist/v9/nano-9.2.tar.xz', archive)
expected = '05ecb99247b782e8a5b3a25ed4101dd034b0236902f7449bc9795b717642f7e9'
if hashlib.sha256(archive.read_bytes()).hexdigest() != expected:
    raise SystemExit('nano archive checksum mismatch')
source = work / 'cross/nano-9.2'
if source.exists():
    raise SystemExit('Existing nano source preserved; use a fresh checkout to reproduce preparation')
source.parent.mkdir(parents=True, exist_ok=True)
with tarfile.open(archive) as tf:
    for member in tf.getmembers():
        if (not (member.name == 'nano-9.2' or member.name.startswith('nano-9.2/')) or '..' in Path(member.name).parts
                or not (member.isdir() or member.isfile())):
            raise SystemExit('Unexpected nano archive entry')
    tf.extractall(source.parent)
prepared = root / 'src/ports/nano/prepared'
shutil.copytree(prepared / 'generated', source, dirs_exist_ok=True)
for name in ('config', 'regex-include'):
    shutil.copytree(prepared / name, work / name, dirs_exist_ok=True)
(work / 'objects').mkdir(exist_ok=True)
print('Prepared pinned nano 9.2; run build-ncurses.sh, then link-nano-probe.sh')
