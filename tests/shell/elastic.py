"""Batch-one demand-grown shell storage and portable read/parameter semantics."""
from pathlib import Path
import subprocess,json,os
r=Path(__file__).resolve().parents[2];w=r/'work/shell-elastic';w.mkdir(exist_ok=True)
cases=[]
def add(name,source,data=b''):cases.append((name,source,data))
add('80 variables','read '+ ' '.join('v'+str(i) for i in range(80))+'; echo "$v0:$v79"; unset v1; v79=changed; echo "$v79"',b' '.join(str(i).encode() for i in range(80))+b'\n')
add('long variable name','long_variable_name_beyond_thirty_one_characters=yes; echo "$long_variable_name_beyond_thirty_one_characters"')
for size in [0,1,127,128,2047,2048,4096,8192,65536]:
 add(f'read {size}', 'IFS= read -r x; printf "%s\\n" "$x"; read x; echo "next:$x"',b'a'*size+b'\nnext\n')
add('large substitution','IFS= read -r x; y=$(printf "%s\\n\\n" "$x"); printf "%s:%s\\n" "${#y}" "$y"',b'x'*8192+b'\n')
add('100 arguments','set -- '+ ' '.join(str(i) for i in range(100))+'; echo "$#:$1:${99}"; printf "<%s>\\n" "$@"')
add('subshell arguments','set -- outer one; (set -- inner two three; shift; echo "$#:$1"); echo "$#:$1:$2"')
add('substitution arguments','set -- outer one; x=$(set -- inner; echo "$1"); echo "$x:$#:$1:$2"')
add('function zero and restore','set -- outer; f() { echo "$0:$1:$#"; set -- changed; return 4; }; f inner; echo "$?:$1"')
add('nested expansion lifetime','x=abc; y=${x%$(printf c)}; echo "$x:$y"')
for ifs in [':', ': ', ' ', '']:
 for data in [b':a:\n',b'a:b:c:\n',b'::\n',b'  a : : b  \n',b'\\ a b\\ c d\n',b'last']:
  add(f'read ifs {ifs!r} {data!r}',f'IFS="{ifs}"; read a b; echo $?; printf "<%s>\\n" "$a" "$b"',data)
rows=[]
for i,(name,source,data) in enumerate(cases):
 a=subprocess.run([str(r/'work/sh-host'),source],input=data,capture_output=True,timeout=15)
 b=subprocess.run(['/bin/dash','-c',source,'sh'],input=data,capture_output=True,timeout=15)
 ok=(a.returncode,a.stdout)==(b.returncode,b.stdout) and not a.stderr
 rows.append(dict(name=name,passed=ok,status=a.returncode,expected_status=b.returncode,output_bytes=len(a.stdout),stderr=a.stderr.decode(errors='replace')))
 if not ok:print(name,a.stdout[:200],b.stdout[:200],a.stderr.decode())
 (w/f'case{i}.sh').write_text(source+'\n');(w/f'in{i}').write_bytes(data);(w/f'expected{i}').write_bytes(b.stdout)
(r/'outputs/shell-elastic/host.json').write_text(json.dumps(rows,indent=2)+'\n')
assert all(x['passed'] for x in rows)
print('PASS',len(rows),'elastic shell/dash comparisons')
