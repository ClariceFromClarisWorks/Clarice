"""Capability tokens in saved actual CLX help, paired with source/parser review.
Not an assertion that every possible option combination has executed.
"""
from pathlib import Path
import json
r=Path(__file__).resolve().parents[2];b=r/'outputs/utility-expansion'
g=json.loads((b/'second-guest.json').read_text());rows=json.loads((b/'help-inventory-final.json').read_text())
checks={
'benchls':['three','timing','emulator'], 'benchscan':['three','timing','emulator'],
'serial':['-b','--test','8N1','Ctrl+]','Cmd-period','Printer'],
'kermit':['-b','-d','send','receive','CRC-16','MacBinary','Existing'],
'wget':['-O','Content-Length','200','TLS','Incomplete'],
'clarice':['-y','--check','--source','update.conf','previous','relaunches'],
'ls':['-','Created','resource'], 'df':['-h','-k','HFS'], 'cat':['-u','stdin','fork'],
'grep':['-E','-F','-i','-v','-n','-c','-l','-q','-s','-w','-x','-H/-h','-e/-f','Status'],
'sed':['-n','-E/-r','-e/-f','Addresses','s supports','-i'],
'awk':['-F','-v','-f','--csv','-safe','-d','--version','ENVIRON','system()'],
'wc':['-l','-w','-c','-m','Default'], 'head':['-count','-n','default 10'],
'cut':['-b','-c','-f','-d','-s','-n'], 'tr':['-cCds','-d','-s','-c/-C','classes'],
'uniq':['-c','-d','-u','-f','-s'], 'tee':['-a','-i','stdin'],
'tail':['-f','-n','-c','+count','-m','Ctrl+C'], 'sort':['-C','-k','-b','-d','-i','-m','-o','-t','spill'],
'cp':['-Rp','-R','-p','resource','Symlinks'], 'mv':['volumes','Directory'],
'touch':['-c','modification','type/creator'], 'rmdir':['empty'], 'mkdir':['-p','parents'],
'rm':['-frR','-r/-R','-f','locks'], 'ln':['-s','Hard links','-f'], 'readlink':['targets','--'],
'hfsattr':['--type','--creator','--locked','--invisible','four bytes'],
'find':['-name','-path','-type','-maxdepth','-print','AND','unsupported'],
'du':['-ashk','-k','-h','-a','-s','resource'], 'file':['-b','type/creator','libmagic'],
'od':['-A','-t','-j','-N','-b','-c','-x','-v','big-endian'], 'more':['Space','Return','Ctrl+C','Redirected'],
'basename':['suffix'], 'dirname':['parent'], 'cmp':['-l','-s','Status','resource'],
'cksum':['CRC','byte count','cryptographic'], 'free':['-b','-k','-m','-h','heap','largest'],
'dd':['if=','of=','bs=','ibs=','obs=','count=','skip=','seek=','notrunc','sync','swab','lcase','ucase','noerror','fork'],
'mktemp':['-qt','-p','-q','-t','TMPDIR','-d','-u','exclusively'],
'sleep':['seconds','cooperatively','Ctrl+C'], 'printenv':['exported','status 1','Private'],
'env':['-i','-u','name=value','restored'], 'time':['stderr','60 Hz','user/sys','pipeline'],
'command':['-v','PATH','One name'],
}
for x in rows:
 o=g['o'+str(x['index'])];assert o and int(g['s'+str(x['index'])])==0 and not g['e'+str(x['index'])],x
 assert max(map(len,o.splitlines()))<=(80 if x['name']=='nano' else 72),x
 for token in checks.get(x['name'],[]):assert token.lower() in ' '.join(o.lower().split()),(x['name'],token)
print(f'PASS: {len(rows)} actual guest help entries, {len(checks)} command capability contracts; remaining builtin text covered by builtin-help.py')
