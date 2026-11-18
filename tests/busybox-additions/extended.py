"""Interoperability, errors, stream behavior and measured full operations in Snow."""
from pathlib import Path
import sys,json,time,re
r=Path(__file__).resolve().parents[2];sys.path.insert(0,str(r/'tests/remote-sessions'));from client import Client
c=Client(port=23245,password=(r.parent/'Clarice-Main/work/remote-sessions/test-password').read_text().strip());rows=[]
def run(cmd,expected=0,contains=None,timeout=120):
 t=time.monotonic();rc,text=c.command(cmd,timeout);row=dict(command=cmd,status=rc,text=text,host_seconds=time.monotonic()-t);rows.append(row);(r/'outputs/busybox-additions/extended.json').write_text(json.dumps(rows,indent=2)+'\n');assert rc==expected,(cmd,rc,text)
 if contains:assert contains in text,(cmd,contains,text)
 print('PASS',cmd,flush=True);return row
try:
 for name in ['diff','base64','xxd','patch','gzip','gunzip','tar']:run(name+' --bogus',2)
 for name in ['binary8k','binary64k','text8k','host.gz','bad.gz','host.tar','truncated.tar']:
  run(f'wget -O /tmp/ba-{name} http://192.168.1.112:8099/{name}')
 run('gunzip -c /tmp/ba-host.gz > /tmp/ba-dec')
 run('cmp /tmp/ba-binary8k /tmp/ba-dec')
 run('gunzip -t /tmp/ba-bad.gz',1)
 run('tar -tf /tmp/ba-host.tar',contains='hello.txt')
 run('tar -xOf /tmp/ba-host.tar binary8k > /tmp/ba-dec')
 run('cmp /tmp/ba-binary8k /tmp/ba-dec')
 run('cat /tmp/ba-host.tar | tar -xOf - hello.txt',contains='hello tar')
 run('tar -xOf /tmp/ba-host.tar missing',1)
 run('tar -tf /tmp/ba-truncated.tar',1)
 run("printf '' > /tmp/ba-empty; base64 /tmp/ba-empty > /tmp/ba-dec; cmp /tmp/ba-empty /tmp/ba-dec")
 run("printf 'A' | base64",contains='QQ==')
 run("printf 'Q' | base64 -d",1)
 run('base64 -w 0 /tmp/ba-binary8k > /tmp/ba-encoded; base64 -d /tmp/ba-encoded > /tmp/ba-dec; cmp /tmp/ba-binary8k /tmp/ba-dec')
 run('xxd -i -l 4 /tmp/ba-binary8k',contains='0x00, 0x01, 0x02, 0x03')
 run('xxd -ps 256 -l 4 /tmp/ba-binary8k',contains='00010203')
 run('xxd -a /tmp/ba-empty',1)
 run('cat /tmp/add-hex | xxd -rp - > /tmp/ba-dec')
 run('cmp /tmp/add-bin /tmp/ba-dec')
 run('tar -xvOf /tmp/ba-host.tar hello.txt',1)
 run("printf '--- a\\n+++ b\\n@@ -1,2 +1,2 @@\\n-alpha\\n' > /tmp/ba-incomplete; cp /tmp/add-old /tmp/ba-keep")
 run('patch -i /tmp/ba-incomplete /tmp/add-old',1)
 run('cmp /tmp/add-old /tmp/ba-keep')
 run('hfsattr --locked on /tmp/add-old')
 run('patch -i /tmp/add-diff /tmp/add-old',1)
 run('cmp /tmp/add-old /tmp/ba-keep')
 run('hfsattr --locked off /tmp/add-old')
 for size in ['8k','64k']:
  source='/tmp/ba-binary'+size
  for cmd in [f'base64 {source} > /tmp/ba-encoded',f'xxd -p {source} > /tmp/ba-hex',f'gzip -c {source} > /tmp/ba-data.gz', 'gunzip -c /tmp/ba-data.gz > /tmp/ba-dec',f'diff {source} {source}']:
   for repeat in range(3):
    row=run(cmd);row['repeat']=repeat;row['size']=size;stats=run('status runtime',contains='remaining=0');row['stats']=stats['text']
  run(f'cmp {source} /tmp/ba-dec')
finally:c.close()
(r/'outputs/busybox-additions/extended.json').write_text(json.dumps(rows,indent=2)+'\n')
