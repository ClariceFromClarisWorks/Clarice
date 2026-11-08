import subprocess,os,json
cases=[
"cat <<EOF\none\nEOF\necho after; cat <<EOF\ntwo\nEOF\necho end\n",
"x=world; cat <<EOF\nhello $x\nEOF\n",
"x=world; cat <<'EOF'\nhello $x\nEOF\n",
"set -- a b; shift; echo \"$1\"",
"false && echo no || echo yes",
"true || echo no && echo yes",
"echo $((-2+3)) $((1 ? 7 : 0))",
"set -- a 'b c'; echo \"$#\"; printf '<%s>\\n' \"$@\"",
"echo 'a b' \"\" x\\ y",
"x=world; echo hello \"$x\"",
"false; echo $?; true && echo yes; false || echo fallback",
"if false; then echo no; elif true; then echo yes; else echo no; fi",
"for x in a 'b c' d; do echo \"$x\"; done",
"i=0; while test \"$i\" -lt 3; do echo \"$i\"; i=$((i+1)); done",
"i=0; until test \"$i\" -eq 3; do i=$((i+1)); done; echo \"$i\"",
"for x in a b c; do if test \"$x\" = b; then continue; fi; echo \"$x\"; done",
"for x in a b c; do echo \"$x\"; break; done",
"f() { echo \"$1\"; return 7; }; f 'a b'; echo $?",
"case abc in a*) echo matched;; *) echo no;; esac",
"x=old; (x=new; echo \"$x\"); echo \"$x\"",
"x=$(echo hello; echo world); echo \"$x\"",
"echo first | cat | cat",
"printf '%s:%d\\n' hello 4 world 5",
"echo ${x:-default}; echo ${x:=saved}; echo \"$x\"",
"x=abcabc; echo ${x#a*} ${x##a*} ${x%c*} ${x%%c*}",
"echo $((2+3*4)) $((0 && 1/1)) $((1 ? 7 : 1/1))",
"f() { for x; do echo \"$x\"; done; }; f a 'b c'",
"x='a b'; printf '<%s>\\n' $x; printf '<%s>\\n' \"$x\"",
"f() { printf '<%s>\\n' \"$@\"; }; f 'a b' '' c",
"# comment\necho okay # trailing comment\n",
"echo file > work/sh-test.txt; cat < work/sh-test.txt; echo append >> work/sh-test.txt; cat work/sh-test.txt",
]
results=[]
for c in cases:
 a=subprocess.run(['work/sh-host',c],capture_output=True,text=True,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0'},timeout=5)
 b=subprocess.run(['/bin/sh','-c',c],capture_output=True,text=True,timeout=5)
 ok=a.stdout==b.stdout and a.returncode==b.returncode
 results.append({'source':c,'pass':ok,'actual':a.stdout,'expected':b.stdout,'status':a.returncode,'expected_status':b.returncode,'stderr':a.stderr})
 if not ok:print(json.dumps(results[-1]))
open(os.environ.get('SYS6_TEST_REPORT','outputs/update1/host-shell-results.json'),'w').write(json.dumps(results,indent=2)+'\n')
print(sum(r['pass'] for r in results),'/',len(results),'passed')

assert all(r["pass"] for r in results), "shell comparison failed"

# Cancellation must unwind the complete active script, not continue its tail.
cancellation=[]
for source in ["while true; do echo active; done; echo BAD", "sh -c 'while true; do echo nested; done'; echo BAD", "while true; do true; done | cat; echo BAD"]:
 r=subprocess.run(['work/sh-host',source,'50'],capture_output=True,text=True,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0'},timeout=5)
 cancellation.append({'source':source,'status':r.returncode,'pass':r.returncode==130 and 'BAD' not in r.stdout})
assert all(x['pass'] for x in cancellation), cancellation
print('3 / 3 cancellation cases passed')
if os.environ.get('SYS6_TEST_REPORT'):
 from pathlib import Path
 Path(os.environ['SYS6_TEST_REPORT']).with_name('host-cancellation-results.json').write_text(json.dumps(cancellation,indent=2)+'\n')
