"""Focused POSIX/ash-family behaviour, compared with dash."""
from pathlib import Path
import subprocess,os,json,shlex
r=Path(__file__).resolve().parents[2];out=r/'outputs/utility-correctness';w=r/'work/utility-correctness'
cases=['set --; printf \'<%s>\\n\' "$@"', 'IFS=:; x=a::b:; set -- $x; printf \'%s\\n\' "$#"; printf \'<%s>\\n\' "$@"', 'IFS=:; set -- a b; printf \'<%s>\\n\' "$*"', 'x=old; x=new true; echo $x', 'x=$(false); echo $?', 'false; x=a; echo $?', 'sh -c \'echo "$0:$1:$#"\' name argument', 'set -- a b; shift 2; echo $#', 'x=; echo ${x-default} ${x:-default}; echo ${x+yes} ${x:+yes}', "case '*' in \\*) echo yes;; *) echo no;; esac", "printf '%s\\n' a\\ b", 'printf \'%s\\n\' "a$(printf \'b\\n\\n\')c"', 'f() { return 4; }; f; echo $?', 'echo $((0 && 1/0))', 'printf \'a\\\\b\\n\' | { read -r x; printf \'%s\\n\' "$x"; }', 'printf \'  a b  \\n\' | { read x; printf \'<%s>\\n\' "$x"; }', 'IFS=; set -- a b; printf "<%s>\\n" "$*"', 'x=$(false) y=$(true); echo $?', 'x=$(true) y=$(false); echo $?', 'x=old; x=new :; echo "$x"', 'set -- outer; sh -c \'set -- inner; echo "$1"\' name arg; echo "$1"', 'printf \'  a b  \\n\' | { IFS= read -r x; printf \'<%s>\\n\' "$x"; }', 'printf \'last\' | { read -r x; echo status:$?; echo "$x"; }', 'printf \'a\\\\\\nb\\n\' | { read x; echo "$x"; }']

for ifs in [':',': ',' ']:
 for value in ['a::b:',':a:',':',' a : : b ', '  a b  ']:
  cases.append(f'IFS={shlex.quote(ifs)}; x={shlex.quote(value)}; set -- $x; echo "$#"; printf "<%s>\\n" "$@"')
rows=[]
for source in cases:
 a=subprocess.run([str(r/'work/sh-host'),source],capture_output=True,text=True,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0'},timeout=5)
 b=subprocess.run(['/bin/dash','-c',source,'sh'],capture_output=True,text=True,timeout=5)
 ok=a.stdout==b.stdout and a.returncode==b.returncode and 'AddressSanitizer' not in a.stderr and 'runtime error:' not in a.stderr
 rows.append(dict(source=source,passed=ok,actual=a.stdout,expected=b.stdout,status=a.returncode,expected_status=b.returncode,stderr=a.stderr))
 if not ok:print(rows[-1])
(out/'posix-focused.json').write_text(json.dumps(rows,indent=2)+'\n')
assert all(x['passed'] for x in rows)
for base in range(0,len(rows),5):
 body=[];expected=[]
 for i,row in enumerate(rows[base:base+5],base):
  body += [f'echo CASE_{i}',f'sh -c {shlex.quote(row["source"])}','echo STATUS:$?']
  expected.append(f'CASE_{i}\n'+row['expected']+f'STATUS:{row["expected_status"]}\n')
 (w/f'posix-{base//5}.sh').write_text('\n'.join(body)+'\n');(w/f'posix-{base//5}.expected').write_text(''.join(expected))
print(f'PASS {len(rows)} focused dash comparisons; prepared {len(range(0,len(rows),5))} guest batches')
