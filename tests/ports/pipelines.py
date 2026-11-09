"""Compare actual upstream source programs and bounded ASan/UBSan ports."""
from pathlib import Path
import subprocess,os
root=Path(__file__).resolve().parents[2]; base=root/'work/pipelines-host'
data=b'alpha one\nbeta two\nbeta two\ngamma three\n'
cases=[('wc',[],data),('wc',['-l'],data),('wc',['-w'],data),('wc',['-c'],data),('wc',['-m'],'café 世界\n'.encode()),('wc',[],b''),('head',[],data),('head',['-n','2'],data),('head',['-n','0'],data),('head',['-n','1'],b'no newline'),('cut',['-b','2-4'],data),('cut',['-c','2-3'],'café 世界\n'.encode()),('cut',['-f','2','-d',' '],data),('cut',['-f','2','-s','-d',':'],b'a:b\nplain\nc:d\n'),('cut',['-b','1,3-4,8-'],data),('tr',['a-z','A-Z'],data),('tr',['-d','aeiou'],data),('tr',['-s',' '],b'a  b   c\n'),('tr',['[:lower:]','[:upper:]'],'café\n'.encode()),('tr',['-cd','0-9'],b'a12b34\n'),('uniq',[],data),('uniq',['-c'],data),('uniq',['-d'],data),('uniq',['-u'],data),('uniq',['-f','1'],b'a same\nb same\nc other\n'),('uniq',['-s','2'],b'a same\nb same\nc other\n')]
def run(name,args,data,ref=False,env=None):
 return subprocess.run([str(base/(('reference-' if ref else '')+name)),*args],input=data,capture_output=True,env=env,timeout=20)
for name,args,data in cases:
 p=run(name,args,data);q=run(name,args,data,True)
 assert (p.returncode,p.stdout)==(q.returncode,q.stdout),(name,args,p,q)
 assert b'ERROR: AddressSanitizer' not in p.stderr and b'runtime error:' not in p.stderr,p.stderr
for name in ['wc','head','cut','tr','uniq']:
 p=run(name,['--help'],b'');assert p.returncode==0 and b'sbase port' in p.stdout
for name,args in [('wc',[]),('head',['-n','100000']),('cut',['-b','1-']),('tr',['a','b']),('uniq',[])]:
 p=run(name,args,b'a\n'*10000,env=dict(os.environ,CLARICE_CANCEL_AFTER='4'));assert p.returncode==130,(name,p)
 p=run(name,args,b'a\n'*10000,env=dict(os.environ,CLARICE_FAIL_WRITE='1'));assert p.returncode!=0,(name,p)
for name,args in [('head',[]),('cut',['-b','1']),('uniq',[])]:
 p=run(name,args,b'a'*17000+b'\n');assert p.returncode==2 and b'16384' in p.stderr,(name,p)
for name,args in [('head',['-n','bad']),('cut',['-b','0']),('uniq',['-f','bad']),('tr',['a',''])]:
 p=run(name,args,b'');assert p.returncode!=0,(name,p)
print(f'PASS: {len(cases)} upstream comparisons, 5 help, 5 cancellation, 5 write-error, 3 line-limit and 4 invalid-option checks. Runtime checks allocation cap and complete release on every invocation.')
# File operands, multiple-file headers/totals and explicit output files.
import tempfile
with tempfile.TemporaryDirectory(dir=root/'work') as td:
 p=Path(td);(p/'a').write_bytes(b'one two\nthree\n');(p/'b').write_bytes(b'four\nfive six\n')
 for name,args in [('wc',[str(p/'a'),str(p/'b')]),('head',['-n','1',str(p/'a'),str(p/'b')]),('cut',['-b','1-2',str(p/'a'),str(p/'b')]),('wc',[str(p/'missing')])]:
  a=run(name,args,b'');b=run(name,args,b'',True);assert (a.returncode,a.stdout)==(b.returncode,b.stdout),(name,a,b)
 a=run('uniq',[str(p/'a'),str(p/'out')],b'');assert a.returncode==0 and (p/'out').read_bytes()==(p/'a').read_bytes()
print('PASS: 5 additional file-operand/output comparisons.')
