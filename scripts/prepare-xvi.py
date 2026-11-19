#!/usr/bin/env python3
"""Fetch pinned genuine xvi; keep adaptations separate from pristine source."""
from pathlib import Path
import subprocess, shutil
root=Path(__file__).resolve().parents[1]
base=root/'work/xvi'; upstream=base/'upstream'
commit='109d2f39849ec81cc36001dbc341aaa45ff7e8ce'
if not upstream.exists(): subprocess.run(['git','clone','https://github.com/martinwguy/xvi.git',str(upstream)],check=True)
subprocess.run(['git','-C',str(upstream),'checkout','--detach',commit],check=True)
cross=base/'cross'
if cross.exists(): shutil.rmtree(cross)
shutil.copytree(upstream/'src',cross)
p=cross/'xvi.h'; s=p.read_text(); s='/* Clarice adaptation by Clarice contributors, 18 November 2026. */\n'+s
s=s.replace('#ifdef\tATARI', '#ifdef CLARICE\n# include "clarice.h"\n# define GOT_OS\n#endif\n\n#ifdef\tATARI',1)
p.write_text(s)
