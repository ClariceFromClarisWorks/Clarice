import sys,json
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'xvi'))
from client import XviClient
c=XviClient(port=23240,password=Path(sys.argv[1]).read_text().strip());rows=[]
try:
 for cmd in ['vi --help','status runtime']:
  rc,text=c.command(cmd,60);rows.append(dict(command=cmd,status=rc,text=text));print(rc,text,flush=True)
 rows.append(c.edit('vi /tmp/vi-new',b'iHello from original vi\x1b:wq\r'));print(rows[-1],flush=True)
 for cmd in ['cat /tmp/vi-new','status runtime']:
  rc,text=c.command(cmd,60);rows.append(dict(command=cmd,status=rc,text=text));print(rc,text,flush=True)
finally:
 c.close();Path('outputs/vi/smoke.json').write_text(json.dumps(rows,indent=2)+'\n')
