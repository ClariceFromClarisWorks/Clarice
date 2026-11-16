from pathlib import Path
import subprocess,json,tempfile,os
r=Path(__file__).resolve().parents[2];w=r/'work/shell-posix2';w.mkdir(exist_ok=True)
cases=[]
for args in ['',"a", "'a b' '' c"]:
 for expr in ['"$@"','"pre$@post"','pre"$@"post','"$@$@"','"""$@"','"$@"""']:
  cases.append(('quoted-at',f'set -- {args}; set -- {expr}; echo "$#"; printf "<%s>\\n" "$@"'))
for expr in ['$x""','""$x','$x""suffix']:
 cases.append(('empty-quote',f'x="a "; set -- {expr}; echo "$#"; printf "<%s>\\n" "$@"'))
cases += [('scalar-at', 'IFS=:; set -- a b; v="$@"; echo "$v"')]
cases += [
 ('local-export', "x=private; sh -c 'echo local:${x-unset}'; export x; sh -c 'echo exported:$x'; x=changed; sh -c 'echo reassigned:$x'"),
 ('export-unset', "export notset; echo ${notset+set}; sh -c 'echo child:${notset+set}'; notset=value; sh -c 'echo child:$notset'"),
 ('prefix-export', "x=old; x=new sh -c 'echo child:$x'; echo parent:$x; sh -c 'echo still:${x-unset}'"),
 ('export-assignment', "value='two words *'; export x=$value; sh -c 'echo \"$x\"'"),
 ('export-subshell', "x=parent; (export x; sh -c 'echo inner:$x'); sh -c 'echo outer:${x-unset}'"),
 ('unset-export', "export x=old; unset x; x=new; sh -c 'echo ${x-unset}'"),
 ('unset-function', "f() { echo yes; }; f; unset -f f; f; echo status:$?"),
 ('function-inheritance', "f() { echo parent; }; sh -c 'f'; echo child:$?; f"),
 ('export-roundtrip', "x=\"a'b\"; export x; export -p > exp; unset x; . ./exp; echo \"$x\""),
]

cases += [
 ('at-separate-groups', 'set --; set -- "$@""$@"; echo "$#"'),
 ('at-braced-group', 'set --; set -- "${@}${@}"; echo "$#"'),
 ('function-exports', "x=local; f() { x=changed; export y=visible; sh -c 'echo ${x-unset}:$y'; }; f; echo $x:$y"),
 ('function-prefix', "x=old; f() { echo $x; x=inner; other=kept; export y=value; }; x=new f; echo $x:$other:$y; sh -c 'echo ${x-unset}:$y'"),
 ('function-args', 'set -- parent; f() { printf "<%s>\\n" "$@"; set -- changed; return 7; }; f "" "two words"; echo "$?:$1"'),
 ('source-scope', '''printf '%s\n' 'x=sourced; export y=visible; f() { echo function; }; return 6; echo BAD' > scope; x=parent; . ./scope; echo "$?:$x"; f; sh -c 'echo "${x-unset}:$y"' '''),
 ('source-args', '''printf '%s\n' 'printf "<%s>\n" "$@"; set -- changed; return 4' > scope; set -- parent; . ./scope; echo "$?:$1"'''),
 ('subshell-scope', "export x=outer; f() { echo original; }; (x=inner; export y=yes; f() { echo changed; }; sh -c 'echo $x:$y'; f); echo $x:${y-unset}; f"),
 ('printf-strings', '''printf '[%8.3s][%-5s][%c]\n' abcdef xy Z; printf '%s:%s\n' one two three'''),
 ('printf-integers', '''printf '%d %i %u %o %x %X\n' -12 0x20 42 10 31 31; printf '[%+06d][%#08x][%#.0o][%.0d][%.5d]\n' 12 31 0 0 -12'''),
 ('printf-escapes', r'''printf '\101\t%s\n' ok; printf '%b' 'a\tb\012c'; printf '%bAFTER' 'end\cignored'; echo done'''),
 ('printf-binary', r'''printf 'A\000B'; printf '%b' 'C\0000D'; printf '%c' ''; printf '%5.3b' 'a\tbcd' '''),
 ('printf-missing', '''printf '%s:%d:%c\n'; printf -- '-%s-\n' ok'''),
 ('printf-character-number', '''printf '%d %x\n' "'A" '"B' '''),
 ('printf-invalid', '''printf '%d\n' nope; echo status:$?; printf '%q' bad; echo status:$?'''),
 ('test-negation', '''test ! 2 -eq 3; echo $?; [ ! abc = def ]; echo $?; test ! ! ''; echo $?'''),
 ('test-groups', '''test '(' 2 -lt 3 ')'; echo $?; test ! '(' -z foo ')'; echo $?; [ '(' '' ')' ]; echo $?'''),
 ('test-operands', '''test ! = !; echo $?; [ -n = -n ]; echo $?; test '(' = ')'; echo $?; test !; echo $?'''),
 ('test-numbers', '''test '' -eq 0; echo $?; test ' 2 ' -eq +2; echo $?; test 12x -lt 99; echo $?; test 999999999999999999999999999999999 -eq 1; echo $?'''),
 ('test-files', '''printf x > item; test -s item; echo $?; test -f item; echo $?; test -d .; echo $?; test -e missing; echo $?; [ -z "" ]; echo $?'''),
]
cases += [
 ('at-count', 'set -- a "" c; echo "${#}:${?}"'),
 ('at-fallback', 'set -- "a b" "" c; printf "<%s>\\n" "${x:-$@}"; x=set; printf "<%s>\\n" "${x:+$@}"'),
 ('at-empty-fallback', 'set --; printf "<%s>\\n" "${x:-$@}"'),
]
rows=[]
for i,(name,source) in enumerate(cases):
 (w/f'p{i}.sh').write_text(source+'\n')
 with tempfile.TemporaryDirectory() as td:
  env={**os.environ,'CLARICE_SHELL_ALLOC_STATS':'1'}
  a=subprocess.run([str(r/'work/sh-host'),source],cwd=td,env=env,capture_output=True)
  b=subprocess.run(['dash','-c',source],cwd=td,capture_output=True)
  ok=a.returncode==b.returncode and a.stdout==b.stdout and b'remaining=0' in a.stderr
  rows.append(dict(name=name,source=source,passed=ok,status=a.returncode,stdout=a.stdout.decode(),expected=b.stdout.decode(),stderr=a.stderr.decode()))
  if not ok:print('FAIL',i,rows[-1])
(r/'outputs/shell-posix2/host.json').write_text(json.dumps(rows,indent=2)+'\n')
assert all(x['passed'] for x in rows)
print('PASS',len(rows),'POSIX expansion cases')
