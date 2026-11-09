from pathlib import Path
import subprocess,os,tempfile,random,time
root=Path(__file__).resolve().parents[2];base=root/'work/pipelines-host'
def run(name,args=(),data=b'',ref=False,env=None):
 return subprocess.run([str(base/(('reference-' if ref else '')+name)),*args],input=data,capture_output=True,env=env,timeout=60)
cases=[]
for data in [b'',b'x',b'x\n',b'a\nb\nc\n',b'a\nb\nc',b'x'*9000+b'\ny\nz\n']:
 for args in [[],['-n','1'],['-n','0'],['-n','+2'],['-c','3'],['-c','+2']]:cases.append(('tail',args,data))
for args in [[],['-n'],['-r'],['-u'],['-f'],['-k','2,2'],['-t',':','-k','2,2n'],['-c'],['-C']]:cases.append(('sort',args,b'z:20\na:2\na:2\nb:3\n'))
for name,args,data in cases:
 a=run(name,args,data);b=run(name,args,data,True);assert (a.returncode,a.stdout)==(b.returncode,b.stdout),(name,args,a,b)
 assert b'ERROR: AddressSanitizer' not in a.stderr and b'runtime error:' not in a.stderr,a.stderr
with tempfile.TemporaryDirectory(dir='/tmp',prefix='clx-') as td:
 d=Path(td);env=dict(os.environ,CLARICE_TEST_TMP=td)
 data=b''.join(('%06d item\n'%n).encode() for n in random.Random(4).sample(range(30000),30000))
 for args in [[],['-r'],['-n'],['-u']]:
  a=run('sort',args,data,env=env);b=run('sort',args,data,True);assert (a.returncode,a.stdout)==(b.returncode,b.stdout),(args,a.returncode,a.stderr)
  assert not list(d.glob('clarice-sort-*'))
 for threshold in ['10','500','2500']:
  a=run('sort',[],data,env=dict(env,CLARICE_CANCEL_AFTER=threshold));assert a.returncode==130,(threshold,a)
  assert not list(d.glob('clarice-sort-*'))
 a=run('sort',[],data,env=dict(env,CLARICE_FAIL_WRITE='1'));assert a.returncode!=0 and not list(d.glob('clarice-sort-*'))
 for count in ['2','20']:
  a=run('sort',[],data,env=dict(env,CLARICE_FAIL_FILE_WRITE=count));assert a.returncode!=0 and not list(d.glob('clarice-sort-*')),(a.returncode,a.stderr)
 (d/'same').write_bytes(data);a=run('sort',['-o',str(d/'same'),str(d/'same')],env=env);assert a.returncode==0 and (d/'same').read_bytes()==run('sort',[],data,True).stdout
 for flags in [[],['-a']]:
  (d/'a').write_bytes(b'old\n');a=run('tee',[*flags,str(d/'a'),str(d/'b')],b'new\n');assert a.returncode==0 and a.stdout==b'new\n';assert (d/'a').read_bytes()==(b'old\nnew\n' if flags else b'new\n')
 (d/'follow').write_bytes(b'initial\n')
 p=subprocess.Popen([str(base/'tail'),'-f',str(d/'follow')],stdout=subprocess.PIPE,stderr=subprocess.PIPE,env=dict(env,CLARICE_CANCEL_AFTER='150'))
 time.sleep(.3)
 with (d/'follow').open('ab') as fp:fp.write(b'appended\n')
 out,err=p.communicate(timeout=8);assert p.returncode==130 and out==b'initial\nappended\n',(p.returncode,out,err)
 assert run('tee',['-i'],b'x').returncode==2
 assert run('tail',['-m','1'],b'x').returncode==2
 assert run('sort',['-c'],b'').returncode==0
 assert run('tail',['-n','10000'],b'x'*200000).returncode!=0
print('PASS: 45 upstream tail/sort comparisons; four 360 KB spilling sorts; three spill cancellations; disk-output failure cleanup; same-file sort; tee overwrite/append; tail follow append/cancel; explicit unsupported options and allocation bound.')
