"""Post-updater acceptance of standard names and explicit compatibility commands."""
from pathlib import Path
import sys,json,re,time
root=Path(__file__).resolve().parents[2];sys.path.insert(0,str(root/'tests/remote-sessions'))
from client import Client
c=Client(port=23244,password=Path(sys.argv[1]).read_text().strip());rows=[];out=root/'outputs/busybox-replacements/guest.json'
def save():out.write_text(json.dumps(rows,indent=2)+'\n')
def cmd(s,expected=0,timeout=180):
 rc,t=c.command(s,timeout);r=dict(command=s,status=rc,text=t);rows.append(r);save()
 if expected is not None:assert rc==expected,(s,rc,t)
 return r
def body(r):return r['text'].split('\r\n',1)[1].split('\r\nREMOTE_END_',1)[0].strip()
def timed(s,expected=0):
 r=cmd('time '+s+'; result=$?; status runtime; test "$result" -eq '+str(expected),timeout=600)
 r['seconds']=float(re.search(r'real ([0-9.]+) s',r['text'])[1]);m=re.search(r'owned-peak=(\d+) remaining=(\d+)',r['text']);r['peak'],r['remaining']=map(int,m.groups());assert r['remaining']==0;save();return r
try:
 cmd('status runtime');cmd('cat /etc/clarice/installed.catalog');cmd('test ! -f /etc/clarice/update.incomplete')
 for n in ['wc','tr','sort']:
  assert 'BusyBox 1.37.0' in body(cmd(n+' --version'))
  for name in [n,n+'-sbase']:
   cmd('appinstall check /bin/'+name);assert name in body(cmd(name+' --help'))
 print('PASS standard names, help, loader checks',flush=True)
 for name in ['ordinary.txt','large.txt','binary.dat','unicode.txt','sort-pressure.txt','macroman.txt','macroman-upper.txt','binary-nonul.dat']:
  cmd('wget -O /tmp/br-'+name+' http://192.168.1.112:8098/'+name)
 cmd("printf 'beta\\nalpha\\nalpha\\n' > /tmp/br-lines")
 for n,args in [('wc','-c < /tmp/br-ordinary.txt'),('tr','a-z A-Z < /tmp/br-ordinary.txt'),('sort','/tmp/br-ordinary.txt')]:
  for run in range(3):
   for name,dest in ([(n+'-sbase','old'),(n,'new')] if run!=1 else [(n,'new'),(n+'-sbase','old')]):
    r=timed(name+' '+args+' > /tmp/br-'+dest);r.update(applet=n,implementation='busybox' if name==n else 'sbase',run=run);save()
   assert body(cmd('cat /tmp/br-old')).split()==body(cmd('cat /tmp/br-new')).split() if n=='wc' else cmd('cmp /tmp/br-old /tmp/br-new')['status']==0
 for s in ['wc -lwmc /tmp/br-lines','tr -s a < /tmp/br-lines','sort -nu /tmp/br-lines'] :cmd(s)
 for n in ['wc','tr','sort']:
  rc=cmd(n+' --invalid-option > /tmp/br-out 2> /tmp/br-err',None)['status'];assert rc!=0
  cmd('test ! -s /tmp/br-out');cmd('test -s /tmp/br-err')
 for n in ['wc','sort'] :assert cmd(n+' /tmp/br-missing',None)['status']!=0
 for n,args in [('wc','-c'),('tr','a-z A-Z'),('sort','')]:cmd("printf '' | "+n+' '+args)
 cmd("printf 'a\\nb\\n' | tr a-z A-Z | wc -l")
 cmd('cp /tmp/br-lines /tmp/br-preserve');assert cmd('sort -o /tmp/br-lines /tmp/br-lines',None)['status']!=0;cmd('cmp /tmp/br-lines /tmp/br-preserve')
 cmd('sort-sbase -o /tmp/br-fallback /tmp/br-lines');cmd('sort /tmp/br-lines > /tmp/br-out');cmd('cmp /tmp/br-out /tmp/br-fallback')
 for n,expected in [('wc',28),('wc-sbase',20)]:assert body(cmd(n+' -m < /tmp/br-unicode.txt'))==str(expected)
 assert body(cmd('wc -m < /tmp/br-macroman.txt'))=='5'
 cmd("tr a-z A-Z < /tmp/br-macroman.txt > /tmp/br-mac-out")
 cmd('cmp /tmp/br-mac-out /tmp/br-macroman-upper.txt')
 cmd("tr -d '\\000' < /tmp/br-binary.dat > /tmp/br-out");cmd('cmp /tmp/br-out /tmp/br-binary-nonul.dat')
 cmd('cp /tmp/br-lines /tmp/br-locked');cmd('hfsattrib --locked on /tmp/br-locked')
 assert cmd('sort /tmp/br-ordinary.txt > /tmp/br-locked',None)['status']!=0
 cmd('cmp /tmp/br-locked /tmp/br-lines');cmd('hfsattrib --locked off /tmp/br-locked')
 timed('sort-sbase /tmp/br-large.txt > /tmp/br-spill');cmd('sort -c /tmp/br-spill');assert body(cmd('wc -c < /tmp/br-spill'))=='65536'
 c.t.write(b'sort /tmp/br-sort-pressure.txt > /tmp/br-out\r\n');time.sleep(2);c.t.write(b'\x03');t=c.t.read_until(b' > ',30);assert t.endswith(b' > ')
 assert 'CANCEL_STATUS=130\r\n' in cmd("printf 'CANCEL_STATUS=%s\\n' $?")['text'];assert 'remaining=0' in cmd('status runtime')['text']
 r=cmd('time sort /tmp/br-sort-pressure.txt > /tmp/br-out; result=$?; status runtime; printf "SORT_STATUS=%s\\n" "$result"',timeout=300)
 assert 'remaining=0' in r['text'];assert re.search(r'SORT_STATUS=(0|2)\r\n',r['text'])
 assert body(cmd('wc -c < /tmp/br-ordinary.txt'))=='8192';assert 'remaining=0' in cmd('status runtime')['text']
finally:c.close();save()
print('PASS',len(rows),'replacement observations')
