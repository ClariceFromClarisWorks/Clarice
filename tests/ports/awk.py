"""Compare the bounded port with pristine One True Awk, not the host /usr/bin/awk."""
from pathlib import Path
import os,subprocess,tempfile,json
root=Path(__file__).resolve().parents[2]
port=root/'work/awk-host/awk';reference=root/'work/awk-host/reference-awk'
results=[]
with tempfile.TemporaryDirectory() as tmp:
 d=Path(tmp);body=b'apple 2 1.5\npear 3 2.25\napple 4 1.5\n'
 (d/'data').write_bytes(body);(d/'extra').write_bytes(b'one\ntwo\n');(d/'prog').write_text('{sum += $2} END {print sum}')
 cases=[
 ['{print $1, $2}'],['{s += $2*$3} END {printf "%.2f\\n",s}'],['$2>2 {print NR, NF, $0}'],
 ['BEGIN {FS="[ ]+"; OFS=":"} {print $1,$2}'],['/apple/ {n++} END {print n}'],
 ['{a[$1]+=$2} END {print a["apple"],a["pear"];delete a["apple"]; print ("apple" in a)}'],
 ['function twice(x) {return 2*x} {print twice($2)}'],['BEGIN {for(i=0;i<4;i++) print i; while(i>2) i--; do i--; while(i>0); print i}'],
 ['BEGIN {print substr("abcdef",2,3), index("abcdef","de"), length("abc"); print toupper("abc"),tolower("XYZ")}'],
 ['{gsub(/p/,"P",$1);sub(/a/,"A",$1);print $1}'],['BEGIN {n=split("a:b:c",a,":"); print n,a[2]; print match("abc123",/[0-9]+/),RSTART,RLENGTH}'],
 ['NR==2 {next} {print $1}'],['{print $1;exit 7} END {print "end"}'],['-v','x=7','BEGIN {print x+1}'],
 ['-F',':','{print NF,$2}'],['-f','prog','data'],['{print FNR,NR,FILENAME} END {print ARGC,ARGV[1]}','data','extra'],
 ['BEGIN {while ((getline x < "extra")>0) print x;close("extra")}'],
 ['{print $1 > "out"} END {close("out"); while ((getline x < "out")>0) print x;close("out")}'],
 ['BEGIN {printf "%d %.3f %s %x\\n",42,sqrt(2),"yes",255; print int(-2.9),sin(0),cos(0),log(1),exp(0),atan2(0,1); print 2^3,5%2}'],
 ['BEGIN {print ENVIRON["AWK_TEST"]}'],['BEGIN {srand(1);a=rand();srand(1);print a==rand(),a>=0 && a<1}'],
 ['--csv','{print NF,$1,$2}'],['{print $1}'],['BEGIN {print "unterminated}'],['{print $1}','absent'],
 ['BEGIN {print (3>2?"yes":"no"), ("1"+2), sprintf("%04d",7)}'],
 ['BEGIN {RS=""} {print NR,NF}'],['BEGIN {FS=""} {print NF,$2}'],['BEGIN {print "ok";fflush()}']]
 for args in cases:
  env={**os.environ,'LC_ALL':'C','HOME':'/','PWD':'/','AWK_TEST':'present'}
  a=subprocess.run([str(port),*args],cwd=tmp,input=body,capture_output=True,env=env,timeout=15)
  b=subprocess.run([str(reference),*args],cwd=tmp,input=body,capture_output=True,env=env,timeout=15)
  assert b'AddressSanitizer' not in a.stderr and b'runtime error:' not in a.stderr,a.stderr
  ok=(a.stdout,a.returncode,bool(a.stderr))==(b.stdout,b.returncode,bool(b.stderr))
  assert ok,(args,a.stdout,a.stderr,a.returncode,b.stdout,b.stderr,b.returncode)
  results.append({'args':args,'status':a.returncode,'passed':True})
 for label,prog,data,env,status in [
 ('cancel loop','BEGIN {while(1) {x++}}',b'',{'CLARICE_CANCEL_AFTER':'4'},130),
 ('owned allocation cap','BEGIN {for(i=0;i<100000;i++) a[i]=i}',b'',{},2),
 ('text cap','{print length($0)}',b'a'*17000,{},2),
 ('recursive calls','function f(n){return f(n+1)} BEGIN {f(0)}',b'',{},2),
 ('unsupported system','BEGIN {system("echo bad")}',b'',{},2),
 ('unsupported command pipe','BEGIN {"echo bad" | getline x}',b'',{},2),
 ('write failure','{print}',body,{'CLARICE_FAIL_WRITE':'1'},2)]:
  a=subprocess.run([str(port),prog],cwd=tmp,input=data,capture_output=True,env={**os.environ,**env},timeout=15)
  assert a.returncode==status,(label,a.returncode,a.stderr)
  assert b'AddressSanitizer' not in a.stderr and b'runtime error:' not in a.stderr,a.stderr
  results.append({'test':label,'status':status,'passed':True})
(root/'outputs/awk').mkdir(exist_ok=True)
(root/'outputs/awk/host-tests.json').write_text(json.dumps(results,indent=2)+'\n')
print(f'PASS: {len(cases)} upstream awk comparisons; cancellation, allocation/text/recursion bounds, unsupported processes, write failures')
