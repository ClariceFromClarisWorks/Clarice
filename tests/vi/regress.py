"""Focused real guest checks for the experimental original vi port."""
import sys,json
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'xvi'))
from client import XviClient
c=XviClient(port=23240,password=Path(sys.argv[1]).read_text().strip());rows=[]
def cmd(source,expected=0,contains=None):
 rc,text=c.command(source,90);rows.append(dict(command=source,status=rc,text=text));assert rc==expected,(source,rc,text)
 if contains is not None:assert contains in text,(contains,text)
 return text
def edit(source,keys):
 row=c.edit(source,keys);rows.append(row);print(source,flush=True);cmd('status runtime',contains='remaining=0')
try:
 cmd('vi --version',contains='Berkeley');cmd('vi --help',contains='Usage:');cmd('vi --bad-option',2,contains='unsupported option')
 cmd("printf 'alpha beta\\nsecond line\\n' > /tmp/vi-edit")
 edit('vi /tmp/vi-edit',b'/beta\rcwBETA\x1b:wq\r');cmd('cat /tmp/vi-edit',contains='alpha BETA')
 edit('vi /tmp/vi-edit',b':%s/line/ROW/\r:wq\r');cmd('cat /tmp/vi-edit',contains='second ROW')
 edit('vi /tmp/vi-edit',b'ix\x1bu:wq\r');cmd('cat /tmp/vi-edit',contains='\r\nalpha BETA\r\nsecond ROW\r\n')
 edit('vi /tmp/vi-edit',b'\x1b[BiARROW \x1b:wq\r');cmd('cat /tmp/vi-edit',contains='ARROW second ROW')
 edit('vi -R /tmp/vi-edit',b'iBAD\x1b:w\r:q!\r');assert 'BAD' not in cmd('cat /tmp/vi-edit')
 cmd('hfsattrib --type TEXT --creator VI01 /tmp/vi-edit')
 edit('vi /tmp/vi-edit',b'A retained\x1b:wq\r');cmd('hfsattrib /tmp/vi-edit',contains='VI01')
 cmd('hfsattrib --locked on /tmp/vi-edit')
 edit('vi /tmp/vi-edit',b'A LOST\x1b:w!\r:q!\r');assert 'LOST' not in cmd('cat /tmp/vi-edit')
 cmd('hfsattrib --locked off /tmp/vi-edit')
 cmd('vi --help > /tmp/vi-help');cmd('cat /tmp/vi-help',contains='Usage:')
 cmd('vi --bad-option > /tmp/vi-out 2> /tmp/vi-err',2);cmd('test ! -s /tmp/vi-out');cmd('cat /tmp/vi-err',contains='unsupported option')
 cmd('vi < /tmp/vi-edit',2,contains='terminal required')
 cmd('vi -r',2,contains='unsupported option')
 edit('vi /tmp/vi-edit',b':!echo no\r:q!\r')
 cmd('cp /tmp/vi-8k /tmp/vi-reference')
 edit('vi /tmp/vi-8k',b':wq\r')
 cmd('cmp /tmp/vi-8k /tmp/vi-reference')
 cmd('test ! -f /etc/clarice/update.incomplete');cmd('cat /etc/clarice/installed.catalog',contains='vi')
 cmd('status runtime',contains='remaining=0')
finally:
 c.close();Path('outputs/vi/regression.json').write_text(json.dumps(rows,indent=2)+'\n')
print('PASS',len(rows),'observations')
