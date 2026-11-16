from pathlib import Path
import subprocess,tempfile,os,json,shutil,shlex
r=Path(__file__).resolve().parents[2];fixtures=r/'tests/shell/practical';out=r/'outputs/shell-practical';rows=[]
cases=[('musl-empty','musl-specs.sh',[]),('musl-paths','musl-specs.sh',['/include space','/lib space','/lib/loader']),('definitions-default','ncurses-def.sh',[]),('definitions-spaces','ncurses-def.sh',['definitions space']),('parameters-default','parametrized.sh',[]),('parameters-spaces','parametrized.sh',['caps space'])]
paths=':'.join(str(r/'work'/p) for p in ['pipelines-host','awk-host','sed-host','ports-host'])+':/usr/bin:/bin'
for name,file,args in cases:
 source='sh ./'+file+(' '+ ' '.join(map(shlex.quote,args)) if args else '')
 results=[]
 for shell in ['clarice','dash']:
  with tempfile.TemporaryDirectory() as td:
   for f in fixtures.iterdir():shutil.copyfile(f,Path(td)/f.name)
   shutil.copyfile(fixtures/'Caps',Path(td)/'caps space');shutil.copyfile(fixtures/'ncurses_defs',Path(td)/'definitions space')
   env={**os.environ,'LC_ALL':'C','PATH':paths,'CLARICE_SHELL_EXTERNAL':'1','CLARICE_SHELL_ALLOC_STATS':'1'}
   p=subprocess.run([str(r/'work/sh-host'),source] if shell=='clarice' else ['dash','-c',source],cwd=td,env=env,capture_output=True,timeout=60)
   results.append(dict(status=p.returncode,stdout=p.stdout.decode(),stderr=p.stderr.decode()))
 a,b=results;ok=a['status']==b['status']==0 and a['stdout']==b['stdout'] and 'remaining=0' in a['stderr'];rows.append(dict(name=name,source=source,passed=ok,actual=a,reference=b));print('PASS' if ok else 'FAIL',name)
(out/'host.json').write_text(json.dumps(rows,indent=2)+'\n')
assert all(x['passed'] for x in rows)
