#!/usr/bin/env python3
"""Fetch genuine ex/vi and its supported traditional regex; apply Mac boundaries."""
from pathlib import Path
import subprocess,shutil,urllib.request,hashlib
root=Path(__file__).resolve().parents[1];work=root/'work/vi';up=work/'upstream';cross=work/'cross'
revision='3f1e9786d249d18012600ae21175eb3521579402'
work.mkdir(parents=True,exist_ok=True)
if not up.exists():subprocess.run(['git','clone','https://github.com/n-t-roff/heirloom-ex-vi',str(up)],check=True)
subprocess.run(['git','-C',str(up),'checkout','--detach',revision],check=True)
regex=work/'regexp.h'
url='https://raw.githubusercontent.com/eunuchs/heirloom-project/2be457cd6b889105d5a550c473238e6922e4bd51/heirloom/heirloom/libcommon/regexp.h'
if not regex.exists():regex.write_bytes(urllib.request.urlopen(url).read())
assert hashlib.sha256(regex.read_bytes()).hexdigest()=='f5a68615b7997c87205f68763af673d8632462c189dccaca35c27220e049e73f'
if cross.exists():shutil.rmtree(cross)
shutil.copytree(up,cross,ignore=shutil.ignore_patterns('.git'));shutil.copy2(regex,cross/'regexp.h')
subprocess.run(['patch','-p1','-i',str(root/'src/ports/vi/upstream.patch')],cwd=cross,check=True)
