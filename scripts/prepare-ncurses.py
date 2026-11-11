#!/usr/bin/env python3
"""Verify and stage pinned upstream ncurses. No changes to the source archive."""
from pathlib import Path
import hashlib, shutil, tarfile, urllib.request
root=Path(__file__).resolve().parents[1]
work=root/'work/ncurses'
work.mkdir(parents=True,exist_ok=True)
archive=work/'ncurses-6.5.tar.gz'
if not archive.exists():
    urllib.request.urlretrieve('https://invisible-island.net/archives/ncurses/ncurses-6.5.tar.gz',archive)
assert hashlib.sha256(archive.read_bytes()).hexdigest()=='136d91bc269a9a5785e5f9e980bc76ab57428f604ce3e5a5a90cebc767971cc6'
source=work/'ncurses-6.5'
if source.exists(): shutil.rmtree(source)
with tarfile.open(archive) as tf:
    for member in tf.getmembers():
        assert not member.name.startswith('/') and '..' not in Path(member.name).parts
        assert member.isdir() or member.isfile(), 'unexpected archive link or special file'
    tf.extractall(work)
with (source/'misc/terminfo.src').open('a') as f:
    f.write('\n'+(root/'src/ports/terminal/clarice.ti').read_text())
build=work/'build'
if build.exists(): shutil.rmtree(build)
build.mkdir()
