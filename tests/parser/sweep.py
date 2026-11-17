from pathlib import Path
import subprocess,re,json,sys
r=Path(__file__).resolve().parents[2]
cases=[x['source'] for x in json.loads((r/'outputs/utility-correctness/posix-focused.json').read_text())]
cases += [p.read_text() for p in sorted((r/'tests/shell/scenarios').glob('*.sh'))]
cases += ['echo `echo hi`',"echo 'unterminated", 'echo "a${x:-b}"', 'cat <<EOF\n$x $(echo hi)\nEOF\n', 'cat <<\'EOF\nraw\nEOF\n', 'case a in a|b) echo yes;; esac', 'if true; then :; elif false; then :; else :; fi', ': > a 2>&1', 'f() { :; } > x', 'echo ${v%"*"}', 'echo ${v:-"bad}', 'for x in a; do', 'case a in a |', 'f( nope', 'while :; do', 'echo "unterminated', 'echo `unterminated']
rows=[]
mode=sys.argv[2] if len(sys.argv)>2 else 'parse'
(r/'outputs/parser-safety').mkdir(parents=True,exist_ok=True)
for idx,source in enumerate(cases):
 if len(sys.argv)>1 and idx<int(sys.argv[1]):continue
 p=subprocess.run([str(r/'work/parser-safety/parse'),'all',source,'single',mode],capture_output=True,timeout=30)
 if mode=='copy' and p.returncode==2:continue
 if p.returncode:print('FAIL',idx,repr(source),p.stdout.decode(),p.stderr.decode()[:4500],flush=True);sys.exit(1)
 n=int(re.search(rb'calls=(\d+)',p.stdout)[1])
 rows.append(dict(case=idx,allocations=n,trials=n*2,passed=True));print('PASS',idx,n,flush=True)
(r/'outputs/parser-safety'/(mode+'-sweep.json')).write_text(json.dumps(rows,indent=2)+'\n')
