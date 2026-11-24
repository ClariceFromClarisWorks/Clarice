import json,time,re,sys,argparse
from pathlib import Path
from client import XviClient
p=argparse.ArgumentParser();p.add_argument('--password-file',type=Path,required=True);p.add_argument('--port',type=int,default=23239);a=p.parse_args()
c=XviClient(port=a.port,password=a.password_file.read_text().strip());rows=[]
out=Path('outputs/xvi/regression.json')
def save():out.write_text(json.dumps(rows,indent=2)+'\n')
def cmd(s,expected=0,contains=None):
 rc,text=c.command(s,90);rows.append(dict(command=s,status=rc,text=text));save();assert rc==expected,(s,rc,text)
 if contains is not None:assert contains in text,(s,text)
 return text
def edit(s,keys):
 r=c.edit(s,keys);rows.append(r);save();cmd('status runtime');assert 'remaining=0' in rows[-1]['text'];return r
try:
 cmd("printf 'alpha beta\\ngamma delta\\n' > /tmp/xvi-edit.txt")
 edit('xvi /tmp/xvi-edit.txt',b'/beta\rcwBETA\x1b:wq\r')
 cmd('cat /tmp/xvi-edit.txt',contains='alpha BETA\r\ngamma delta')
 edit('xvi /tmp/xvi-edit.txt',b'ggdd u:q!\r')
 cmd('cat /tmp/xvi-edit.txt',contains='alpha BETA\r\ngamma delta')
 edit('xvi /tmp/xvi-edit.txt',b':%s/delta/DELTA/\r:wq\r')
 cmd('cat /tmp/xvi-edit.txt',contains='gamma DELTA')
 cmd('hfsattrib --type TEXT --creator XVI1 /tmp/xvi-edit.txt')
 before=cmd('hfsattrib /tmp/xvi-edit.txt')
 edit('xvi /tmp/xvi-edit.txt',b'A retained\x1b:wq\r')
 after=cmd('hfsattrib /tmp/xvi-edit.txt');assert 'XVI1' in after and 'TEXT' in after
 cmd('hfsattrib --locked on /tmp/xvi-edit.txt')
 edit('xvi /tmp/xvi-edit.txt',b'A LOST\x1b:w!\r:q!\r')
 cmd('cat /tmp/xvi-edit.txt',contains='retained');assert 'LOST' not in rows[-1]['text']
 cmd('hfsattrib --locked off /tmp/xvi-edit.txt')
 edit('xvi -R /tmp/xvi-edit.txt',b'A LOST\x1b:w\r:q!\r')
 cmd('cat /tmp/xvi-edit.txt');assert 'LOST' not in rows[-1]['text']
 cmd('xvi --help > /tmp/xvi-help');cmd('cat /tmp/xvi-help',contains='Usage: xvi')
 cmd('xvi --bad-option > /tmp/xvi-out 2> /tmp/xvi-err',expected=2)
 cmd('test ! -s /tmp/xvi-out');cmd('cat /tmp/xvi-err',contains='unknown option')
 cmd('xvi < /tmp/xvi-edit.txt',expected=2) # /dev/null hits documented Core isatty defect
 cmd("awk 'BEGIN {for(i=0;i<128;i++)printf \"%063d\\n\",i}' > /tmp/xvi-8k")
 cmd('wc -c /tmp/xvi-8k',contains='8192')
 for path in ['', '/tmp/xvi-8k']:
  for i in range(3):edit('xvi'+(' '+path if path else ''),b':q!\r')
 cmd('test ! -f /etc/clarice/update.incomplete')
 cmd('cat /etc/clarice/installed.catalog',contains='xvi')
finally:c.close();save()
print('PASS',len(rows),'recorded observations')
