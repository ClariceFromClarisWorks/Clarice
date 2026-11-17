"""Whole portable scripts: production shell adapter vs dash, isolated work dirs.
Host external utilities exercise real upstream ports where available, not CLX/HFS.
Guest repeats use the same source and native external commands.
"""
from pathlib import Path
import os, subprocess, tempfile, json, shutil
r=Path(__file__).resolve().parents[2];out=r/'outputs/utility-correctness';work=r/'work/utility-correctness/scenarios';work.mkdir(exist_ok=True)
rows=[]
paths=':'.join(str(r/'work'/p) for p in ['pipelines-host','awk-host','sed-host','ports-host'])+':/usr/bin:/bin'
for p in sorted((r/'tests/shell/scenarios').glob('*.sh')):
 results=[]
 for shell in ['clarice','dash']:
  with tempfile.TemporaryDirectory(prefix='cl-sh-') as td:
   env={**os.environ,'LC_ALL':'C','ASAN_OPTIONS':'detect_leaks=0','CLARICE_SHELL_EXTERNAL':'1','PATH':paths}
   cmd=[str(r/'work/sh-host'),p.read_text()] if shell=='clarice' else ['/bin/dash','-c',p.read_text(),'sh']
   run=subprocess.run(cmd,cwd=td,env=env,capture_output=True,timeout=30)
   files={str(f.relative_to(td)):f.read_bytes().hex() for f in Path(td).rglob('*') if f.is_file()}
   results.append(dict(stdout=run.stdout.decode(),stderr=run.stderr.decode(),status=run.returncode,files=files))
 a,b=results;ok=a['stdout']==b['stdout'] and a['status']==b['status'] and a['files']==b['files'] and 'Sanitizer' not in a['stderr']
 rows.append(dict(name=p.stem,passed=ok,actual=a,reference=b))
 print(('PASS ' if ok else 'FAIL ')+p.stem)
 if not ok:print(rows[-1])
 shutil.copyfile(p,work/p.name);(work/(p.stem+'.expected')).write_text(b['stdout'])
(out/'scenarios-host.json').write_text(json.dumps(rows,indent=2)+'\n')
assert all(row['passed'] for row in rows)
