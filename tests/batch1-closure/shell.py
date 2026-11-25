"""Focused closure semantics, dash comparison, spool faults and recovery."""
import subprocess,os,sys,json,re
from pathlib import Path
before='--before' in sys.argv
cases=[r"case b in [a\-c]) echo wrong;; *) echo literal;; esac",r"case - in [a\-c]) echo literal;; *) echo wrong;; esac",r"x=bword; printf '<%s>\n' ${x#[a\-c]}",r"x=wordb; printf '<%s>\n' ${x%[a\-c]}",r"set -- x '~'; printf '<%s>\n' $@",r"x='~'; printf '<%s>\n' $x",r"set -- '~' '~/file'; printf '<%s>\n' $@",r"HOME=/tmp; printf '<%s>\n' ~ ~/file; set -- x '~'; printf '<%s>\n' $@"]
rows=[]
for cmd in cases:
 ref=subprocess.run(['/bin/dash','-c',cmd],capture_output=True,text=True)
 p=subprocess.run(['work/sh-host',cmd],capture_output=True,text=True,env={**os.environ,'CLARICE_SHELL_ALLOC_STATS':'1'})
 same=(p.returncode,p.stdout)==(ref.returncode,ref.stdout)
 assert 'remaining=0' in p.stderr
 if not before:assert same,(cmd,p,ref.stdout)
 rows.append(dict(command=cmd,status=p.returncode,output=p.stdout,reference=ref.stdout,match=same))
for cmd,cancel in [('! echo BAD | cat',None),('! while :; do :; done | cat','8')]:
 p=subprocess.run(['work/sh-host',cmd]+([cancel] if cancel else []),capture_output=True,text=True,env={**os.environ,'CLARICE_SHELL_FAIL_REWIND':'1','CLARICE_SHELL_RECOVERY':'1','CLARICE_SHELL_ALLOC_STATS':'1'})
 if not before:assert p.returncode==(130 if cancel else 2),(cmd,p)
 assert 'STREAMS-RESTORED' in p.stdout and 'remaining=0' in p.stderr and 'BAD' not in p.stdout
 rows.append(dict(command=cmd,status=p.returncode,output=p.stdout,error=p.stderr))
Path('outputs/batch1-closure/shell-'+('before' if before else 'after')+'.json').write_text(json.dumps(rows,indent=2)+'\n')
print('REPRO' if before else 'PASS',len(rows),'closure shell cases',sum(not r.get('match',True) for r in rows),'dash mismatches')

if not before:
 count=0
 for cmd in cases[:4]+["! echo x | cat"]:
  base=subprocess.run(['work/sh-host',cmd],capture_output=True,text=True,env={**os.environ,'CLARICE_SHELL_ALLOC_STATS':'1'})
  total=int(re.search(r'ALLOC calls=(\d+)',base.stderr)[1])
  for i in range(1,total+1):
   p=subprocess.run(['work/sh-host',cmd],capture_output=True,text=True,env={**os.environ,'CLARICE_SHELL_ALLOC_STATS':'1','CLARICE_SHELL_FAIL_ALLOC':str(i),'CLARICE_SHELL_RECOVERY':'1'})
   assert p.returncode>=0 and 'remaining=0' in p.stderr and 'STREAMS-RESTORED' in p.stdout and 'AddressSanitizer' not in p.stderr,(cmd,i,p)
   count+=1
 print('PASS',count,'focused allocation faults with recovery')
