import json,time,argparse
from pathlib import Path
from client import XviClient
p=argparse.ArgumentParser();p.add_argument('--password-file',type=Path,required=True);a=p.parse_args()
c=XviClient(port=23239,password=a.password_file.read_text().strip());rows=[];out=Path('outputs/xvi/more.json')
def save():out.write_text(json.dumps(rows,indent=2)+'\n')
def cmd(s,expected=0,contains=None):
 rc,text=c.command(s,90);rows.append(dict(command=s,status=rc,text=text));save();assert rc==expected,(s,rc,text)
 if contains is not None:assert contains in text,(s,text)
 return text
def edit(s,keys):
 r=c.edit(s,keys);rows.append(r);save();cmd('status runtime');assert 'remaining=0' in rows[-1]['text'];return r
try:
 cmd('xvi < /tmp/xvi-edit.txt',expected=2)
 cmd("awk 'BEGIN {for(i=0;i<128;i++)printf \"%063d\\n\",i}' > /tmp/xvi-8k")
 cmd('wc -c /tmp/xvi-8k',contains='8192')
 for path in ['', '/tmp/xvi-8k']:
  for i in range(3):edit('xvi'+(' '+path if path else ''),b':q!\r')
 before=cmd('cat /tmp/xvi-edit.txt')
 edit('xvi /tmp/xvi-edit.txt',b'ddu:wq\r')
 cmd('cat /tmp/xvi-edit.txt',contains='alpha BETA retained\r\ngamma DELTA')
 edit('xvi /tmp/xvi-8k',b'G$A END\x1b:wq\r')
 cmd('tail -1 /tmp/xvi-8k',contains='127 END')
 cmd('test ! -f /etc/clarice/update.incomplete')
 cmd('cat /etc/clarice/installed.catalog',contains='xvi')
 cmd('ls -1 /mnt/GlyphFull')
finally:c.close();save()
print('PASS',len(rows),'observations')
