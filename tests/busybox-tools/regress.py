"""Guest common-semantics, explicit gaps, binary and cancellation checks."""
from pathlib import Path
import sys,json,time,re
r=Path(__file__).resolve().parents[2];sys.path.insert(0,str(r/'tests/xvi'));from client import XviClient
c=XviClient(port=23243,password=Path(sys.argv[1]).read_text().strip());out=r/'outputs/busybox-tools/regression.json';rows=json.loads(out.read_text()) if '--resume' in sys.argv[2:] else []
done=set()
for i in range(len(rows)-2):
 if rows[i+1].get('command')==('bb'+rows[i].get('command','')).replace('/tmp/bbt-reference','/tmp/bbt-candidate') and rows[i+2].get('command')=='cmp /tmp/bbt-reference /tmp/bbt-candidate' and rows[i+2]['status']==0:done.add(rows[i]['command'])
def save():out.write_text(json.dumps(rows,indent=2)+'\n')
def cmd(s,expected=0):
 rc,t=c.command(s,180);rows.append(dict(command=s,status=rc,text=t));save()
 if expected is not None:assert rc==expected,(s,rc,t)
 return rc,t
def pair(name,args):
 if name+' '+args+' > /tmp/bbt-reference' in done:return
 cmd(name+' '+args+' > /tmp/bbt-reference');cmd('bb'+name+' '+args+' > /tmp/bbt-candidate');cmd('cmp /tmp/bbt-reference /tmp/bbt-candidate');print('equal',name,args,flush=True)
try:
 cmd("printf 'alpha beta\\nalpha beta\\nGamma\\nlast\\n' > /tmp/bbt-cases")
 for name,args in [('cat','/tmp/bbt-binary.dat'),('tr',"-s 'a' < /tmp/bbt-cases"),('tr',"-cd 'a-z\\n' < /tmp/bbt-cases"),('tr',"'[:lower:]' '[:upper:]' < /tmp/bbt-cases"),('sort','-r /tmp/bbt-cases'),('sort','-u /tmp/bbt-cases'),('sort',"-t ' ' -k 1,1n /tmp/bbt-ordinary.txt"),('sed',"-n '2,3p' /tmp/bbt-cases"),('sed',"-e 's/alpha/A/g' -e 's/beta/B/g' /tmp/bbt-cases"),('sed',"'s/\\(alpha\\)/[\\1]/g' /tmp/bbt-cases"),('grep','-n -i gamma /tmp/bbt-cases'),('grep','-v alpha /tmp/bbt-cases'),('grep','-c -e alpha -e Gamma /tmp/bbt-cases'),('cut','-b 1-4 /tmp/bbt-cases'),('tail','-c 100 /tmp/bbt-binary.dat'),('uniq','-d /tmp/bbt-cases'),('tee','< /tmp/bbt-binary.dat'),('cksum','< /tmp/bbt-binary.dat'),('basename','/tmp/example.txt .txt'),('dirname','/tmp/example.txt')]:pair(name,args)
 cmd('wget -O /tmp/bbt-first100 http://192.168.1.112:8097/first100.dat')
 cmd('bbhead -c 100 /tmp/bbt-binary.dat > /tmp/bbt-out');cmd('cmp /tmp/bbt-out /tmp/bbt-first100')
 cmd('bbcmp /tmp/bbt-cases /tmp/bbt-cases');cmd('bbcmp -s /tmp/bbt-cases /tmp/bbt-small.txt',1);cmd('bbcmp /tmp/bbt-missing /tmp/bbt-cases',2)
 cmd('bbgrep no_such_pattern /tmp/bbt-cases',1);cmd("bbgrep '[' /tmp/bbt-cases",2)
 for name in ['wc','tr','cmp','sort','sed','grep','cat','cut','head','tail','uniq','tee','cksum']:
  rc,t=cmd('bb'+name+' --not-an-option > /tmp/bbt-out 2> /tmp/bbt-err',None);assert rc!=0,(name,t)
  cmd('test ! -s /tmp/bbt-out');cmd('test -s /tmp/bbt-err')
 for s in ['bbsed -i s/a/b/ /tmp/bbt-cases','bbsort -o /tmp/bbt-cases /tmp/bbt-cases','bbgrep -r alpha /tmp','bbtail -f /tmp/bbt-cases','bbtee -i < /tmp/bbt-cases']:
  rc,_=cmd(s,None);assert rc!=0
 cmd("printf 'alpha beta\\nalpha beta\\nGamma\\nlast\\n' > /tmp/bbt-original");cmd('cmp /tmp/bbt-cases /tmp/bbt-original')
 cmd('bbtee /tmp/bbt-tee < /tmp/bbt-binary.dat > /tmp/bbt-out');cmd('cmp /tmp/bbt-tee /tmp/bbt-binary.dat');cmd('cmp /tmp/bbt-out /tmp/bbt-binary.dat')
 for s in ['bbwc -c /tmp/bbt-missing','bbsort /tmp/bbt-missing','bbsed p /tmp/bbt-missing','bbgrep alpha /tmp/bbt-missing','bbhead /tmp/bbt-missing','bbtail /tmp/bbt-missing','bbuniq /tmp/bbt-missing','bbcat /tmp']:
  rc,_=cmd(s,None);assert rc!=0
 cmd('wget -O /tmp/bbt-nonul http://192.168.1.112:8097/binary-nonul.dat')
 cmd("bbtr -d '\\000' < /tmp/bbt-binary.dat > /tmp/bbt-out");cmd('cmp /tmp/bbt-out /tmp/bbt-nonul')
 cmd("tr -d '\\000' < /tmp/bbt-binary.dat > /tmp/bbt-reference",None);cmd('cmp /tmp/bbt-reference /tmp/bbt-nonul',None)
 # Character semantics are evidence, not an ASCII-equivalence assertion.
 for s in ['wc -m /tmp/bbt-unicode.txt','bbwc -m /tmp/bbt-unicode.txt','wc -w /tmp/bbt-unicode.txt','bbwc -w /tmp/bbt-unicode.txt']:
  cmd(s)
 # Long-line and constrained heap behaviour. Both outcomes are recorded honestly.
 for s in ["bbgrep -F a /tmp/bbt-long.txt > /tmp/bbt-out", "bbsed 's/a/A/' /tmp/bbt-long.txt > /tmp/bbt-out", 'bbsort /tmp/bbt-long.txt > /tmp/bbt-out']:
  cmd(s,None);cmd('status runtime')
 # Actual remote cancellation inside a CPU-only sed branch loop.
 c.t.write(b"bbsed ':again;b again' /tmp/bbt-small.txt\r\n");time.sleep(2);c.t.write(b'\x03');data=c.t.read_until(b' > ',30)
 assert data.endswith(b' > '),repr(data);rows.append(dict(test='sed branch cancellation',transcript=data.decode('latin1')));save()
 rc,t=cmd("printf 'CANCEL_STATUS=%s\\n' $?");assert 'CANCEL_STATUS=130\r\n' in t,t
 cmd('status runtime');cmd('bbwc -c < /tmp/bbt-small.txt');cmd('status runtime')
finally:c.close();save()
print('PASS',len(rows),'checks')
