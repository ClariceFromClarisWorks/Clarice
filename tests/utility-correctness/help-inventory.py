"""Prepare a complete source-derived guest help inventory, without claiming acceptance."""
from pathlib import Path
import re,json
r=Path(__file__).resolve().parents[2];out=r/'work/utility-expansion';out.mkdir(parents=True,exist_ok=True)
s=(r/'src/command-install.inc').read_text().split('};',1)[0]
external=[n for n,_ in re.findall(r'\{"([^"\\]+)",(\d+)\}',s)]
s=(r/'src/help.inc').read_text();builtin=re.findall(r'\{"([^"\\]+)","',s)
rows=[];commands=[]
for kind,names in [('external',external),('builtin',builtin),('experimental',['nano'])]:
 for name in names:
  index=len(rows);command=(f'/mnt/Audit/tools/{name} --help' if kind=='external' else f'help {name}' if kind=='builtin' else '/mnt/Audit/nano --help')
  if name in ('[',): command="help '['"
  rows.append(dict(index=index,name=name,kind=kind,command=command,verified=False))
  commands.extend([f'{command} > /mnt/Audit/help/o{index} 2> /mnt/Audit/help/e{index}',f'echo $? > /mnt/Audit/help/s{index}'])
commands+=['echo HELP_DONE']
(out/'help.sh').write_text('\n'.join(commands)+'\n');(out/'inventory.json').write_text(json.dumps(rows,indent=2)+'\n')
print(f'Prepared {len(external)} bundled commands, {len(builtin)} builtin topics and nano; guest execution pending')
