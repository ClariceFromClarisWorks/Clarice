#!/usr/bin/env python3
"""Pin BusyBox 1.37.0; preserve upstream and stage only the vi boundary patch."""
from pathlib import Path
import urllib.request,hashlib,tarfile,subprocess,shutil
r=Path(__file__).resolve().parents[1];w=r/'work/busybox';w.mkdir(parents=True,exist_ok=True)
a=w/'busybox-1.37.0.tar.bz2';sha='3311dff32e746499f4df0d5df04d7eb396382d7e108bb9250e7b519b837043a4'
if not a.exists():a.write_bytes(urllib.request.urlopen('https://busybox.net/downloads/busybox-1.37.0.tar.bz2').read())
assert hashlib.sha256(a.read_bytes()).hexdigest()==sha
u=w/'busybox-1.37.0'
if u.exists():shutil.rmtree(u)
with tarfile.open(a) as t:
 for m in t.getmembers():
  assert not m.name.startswith('/') and '..' not in Path(m.name).parts and not m.issym() and not m.islnk()
 t.extractall(w)
c=w/'cross';(c/'editors').mkdir(parents=True,exist_ok=True);shutil.copy2(u/'editors/vi.c',c/'editors/vi.c')
subprocess.run(['patch','-p1','-i',str(r/'src/ports/busybox/upstream.patch')],cwd=c,check=True)
shutil.copy2(c/'editors/vi.c',w/'vi.c')
