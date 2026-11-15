"""Focused compound redirection ownership, failures and cancellation recovery."""
from pathlib import Path
import os,subprocess,tempfile,json
r=Path(__file__).resolve().parents[2];rows=[]
programs=[
 '{ printf bad; } > missing/file',
 '{ printf bad; } > first > missing/file',
 '{ printf bad; } 1>&- ',
 '{ while true; do printf x; done; } > output',
 'while true; do { printf x; continue; } >> output; done',
 '{ for x in a b c; do { echo "$x"; } > inner; done; } > outer',
 'f() { { echo saved; return 7; } > output; }; f',
]
for source in programs:
 for cancel in ([0] if 'while true' not in source else [6,10,20]):
  with tempfile.TemporaryDirectory() as td:
   p=subprocess.run([str(r/'work/sh-host'),source,str(cancel)],cwd=td,capture_output=True,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0','CLARICE_SHELL_RECOVERY':'1'},timeout=5)
   expected=130 if cancel else 7 if source.startswith('f()') else 0 if source.startswith('{ for') else 1 if '1>&-' in source else 2
   ok=p.returncode==expected and p.stdout.endswith(b'STREAMS-RESTORED\n') and b'Sanitizer' not in p.stderr and b'leaked native' not in p.stderr and not list(Path(td).glob('*sh-*'))
   rows.append(dict(source=source,cancel_at=cancel,status=p.returncode,expected=expected,passed=ok))
   if not ok:print(rows[-1],p.stdout,p.stderr)
(r/'outputs/utility-correctness/compound-failures.json').write_text(json.dumps(rows,indent=2)+'\n')
assert all(x['passed'] for x in rows)
print(f'PASS {len(rows)} compound failure/control-flow/cancellation ownership checks')
