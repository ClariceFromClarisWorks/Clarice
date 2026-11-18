from pathlib import Path
import sys,json,time,re
sys.path.insert(0,str(Path('tests/filesystem-fixes/remote').resolve()))
from client import Client
out=Path('outputs/filesystem-confidence');rows=[];c=Client(port=23283,password='')
def check(label,cmd,expected=0,contains=None):
 t=time.monotonic();rc,text=c.command(cmd,600);ok=rc==expected and (contains is None or contains in text);rows.append(dict(test=label,command=cmd,expected=expected,status=rc,passed=ok,text=text,seconds=time.monotonic()-t));(out/'stress.json').write_text(json.dumps(rows,indent=2)+'\n');print(('PASS' if ok else 'FAIL'),label,rc,flush=True);assert ok,(label,text)
try:
 check('working directory',"cd '/mnt/FS Audit A/confidence-final'")
 check('large seek creation','dd if=moved of=large bs=1024 seek=2048 count=1')
 check('large size','ls -l large',contains='2097163')
 check('large copy and catalog size','cp large largecopy && ls -l largecopy')
 check('disk full target seed',"cp -p '/mnt/FS Audit A/fork-source' '/mnt/FS Audit B/confidence-final/full-target'")
 check('fill destination volume',"dd if=moved of='/mnt/FS Audit B/confidence-final/filler' bs=1024 seek=65536 count=1",1)
 check('full disk copy preserves target',"cp large '/mnt/FS Audit B/confidence-final/full-target'",1)
 check('full disk target retained',"cmp '/mnt/FS Audit A/fork-source' '/mnt/FS Audit B/confidence-final/full-target'")
 check('full disk move preserves source',"mv large '/mnt/FS Audit B/confidence-final/full-target'",1)
 check('source still present','test -f large')
 check('target still retained',"cmp '/mnt/FS Audit A/fork-source' '/mnt/FS Audit B/confidence-final/full-target'")
 check('free space recovery',"rm '/mnt/FS Audit B/confidence-final/filler'; cp large '/mnt/FS Audit B/confidence-final/recovered'; ls -l '/mnt/FS Audit B/confidence-final/recovered'")
 check('final cleanup',"status runtime",contains='remaining=0')
finally:c.close()
print('PASS',len(rows),'large-file and real-full-volume checks',flush=True)
