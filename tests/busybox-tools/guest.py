"""Matched isolated-Snow comparisons. Run with Python3.9 Telnet client."""
from pathlib import Path
import sys,json,time,re,shlex
r=Path(__file__).resolve().parents[2];sys.path.insert(0,str(r/'tests/xvi'))
from client import XviClient
mode=sys.argv[1];password=Path(sys.argv[2]).read_text().strip()
c=XviClient(port=23243,password=password);rows=[]
out=r/'outputs/busybox-tools'/f'{mode}.json'
def save():out.write_text(json.dumps(rows,indent=2)+'\n')
def cmd(s,expected=0,timeout=180):
 start=time.monotonic();rc,text=c.command(s,timeout);row=dict(command=s,status=rc,text=text,host_seconds=time.monotonic()-start);rows.append(row);save()
 if expected is not None:assert rc==expected,(s,rc,text)
 return row
def body(row):
 return row['text'].split('\r\n',1)[1].split('\r\nREMOTE_END_',1)[0].strip().replace('\r\n','\n')
def measured(s,expected=0):
 row=cmd('time '+s+'; result=$?; status runtime; test "$result" -eq '+str(expected),timeout=240)
 m=re.search(r'real ([0-9.]+) s',row['text']);assert m,row
 row['guest_seconds']=float(m[1])
 m=re.search(r'Runtime: heap=(\d+) largest=(\d+) image=(\d+) ticks=(\d+) load=(\d+) owned-peak=(\d+) remaining=(\d+)',row['text']);assert m,row
 row.update(dict(zip(['heap','largest','image','ticks','load','peak','remaining'],map(int,m.groups()))));assert row['remaining']==0,row
 save();return row
names=list(json.loads((r/'src/ports/busybox-tools/help.json').read_text()))
try:
 if mode=='setup':
  for n in ['small.txt','ordinary.txt','large.txt','binary.dat','unicode.txt','long.txt']:
   cmd('wget -O /tmp/bbt-'+n+' http://192.168.1.112:8097/'+n)
  cmd('cp /tmp/bbt-large.txt /tmp/bbt-copy.txt')
 elif mode=='smoke':
  for n in names:
   row=measured('bb'+n+' --version');row['applet']=n;save();print('loaded',n,flush=True)
   cmd('bb'+n+' --help > /tmp/bbt-help')
  for s in ["bbwc -c < /tmp/bbt-small.txt", "bbtr a-z A-Z < /tmp/bbt-small.txt > /tmp/bbt-out", "bbcmp /tmp/bbt-large.txt /tmp/bbt-copy.txt", "bbsort /tmp/bbt-small.txt > /tmp/bbt-out", "bbsed 's/alpha/ALPHA/g' /tmp/bbt-small.txt > /tmp/bbt-out", "bbgrep -c alpha /tmp/bbt-small.txt"]:print(body(cmd(s)),flush=True)
 elif mode=='measure':
  for size in ['large','ordinary','small']:
   file='/tmp/bbt-'+size+'.txt'
   workloads=[('wc-bytes','wc','-c < '+file),('tr','tr','a-z A-Z < '+file),('wc-all','wc','< '+file),('cmp','cmp',file+' '+file),('sort','sort',file),('sed','sed',"'s/alpha/ALPHA/g' "+file),('grep-fixed','grep','-F alpha '+file),('grep-regex','grep',"-E '^0[0-9].*alpha' "+file),('cat','cat',file),('cut','cut',"-d ' ' -f 1,3 "+file),('head','head','-n 20 '+file),('tail','tail','-n 20 '+file),('uniq','uniq',file),('tee','tee','< '+file),('cksum','cksum','< '+file)]
   for label,name,args in workloads:
    for run in range(3):
     for prefix in (['','bb'] if run!=1 else ['bb','']):
      row=measured(prefix+name+' '+args+' > /tmp/bbt-'+('candidate' if prefix else 'reference'))
      row.update(applet=name,workload=label,size=size,run=run,implementation='busybox' if prefix else 'existing');save()
     if run==1:continue # First/final outputs checked; middle reverses invocation order.
     if name=='wc':
      a=body(cmd('cat /tmp/bbt-reference'));b=body(cmd('cat /tmp/bbt-candidate'));assert a.split()==b.split(),(a,b)
     else:cmd('cmp /tmp/bbt-reference /tmp/bbt-candidate')
    print(size,label,'passed',flush=True)
  for name,args in [('basename','/tmp/example.txt .txt'),('dirname','/tmp/example.txt')]:
   for run in range(3):
    for prefix in ['', 'bb']:
     row=measured(prefix+name+' '+args+' > /tmp/bbt-'+('candidate' if prefix else 'reference'));row.update(applet=name,workload=name,size='path',run=run,implementation='busybox' if prefix else 'existing');save()
    cmd('cmp /tmp/bbt-reference /tmp/bbt-candidate')
finally:c.close();save()
print('PASS',mode,len(rows),flush=True)
