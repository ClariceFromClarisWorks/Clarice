#!/usr/bin/env python3
"""Fetch and verify the pinned upstream BusyBox source; no generated port edits."""
from pathlib import Path
import urllib.request, hashlib, tarfile, shutil
root=Path(__file__).resolve().parents[1];work=root/'work/busybox';work.mkdir(parents=True,exist_ok=True)
archive=work/'busybox-1.37.0.tar.bz2'
expected='3311dff32e746499f4df0d5df04d7eb396382d7e108bb9250e7b519b837043a4'
if not archive.exists():
 data=urllib.request.urlopen('https://busybox.net/downloads/busybox-1.37.0.tar.bz2').read()
 if hashlib.sha256(data).hexdigest()!=expected:raise ValueError('BusyBox archive hash mismatch')
 archive.write_bytes(data)
if hashlib.sha256(archive.read_bytes()).hexdigest()!=expected:raise ValueError('BusyBox archive hash mismatch')
source=work/'busybox-1.37.0'
if source.exists():shutil.rmtree(source)
with tarfile.open(archive) as f:
 for member in f.getmembers():
  if member.name.startswith('/') or '..' in Path(member.name).parts or member.issym() or member.islnk():raise ValueError('Unsafe archive member')
 f.extractall(work)
