from pathlib import Path
import json
p=Path('outputs/external-tools/guest')
checks=[]
def eq(name,expected):
 actual=(p/name).read_text();checks.append(dict(file=name,passed=actual==expected,actual=actual,expected=expected))
for name,value in {'copy.txt':'alpha\nbeta\n','pipe.txt':'alpha\n','out.txt':'','error-status.txt':'1\n','ordered.txt':'','closed-status.txt':'1\n','after-closed.txt':'okay\n','closed-input-status.txt':'1\n','false-pipe.txt':'','last-status.txt':'0\n','false-status.txt':'1\n','no-path-status.txt':'127\n','usage-status.txt':'2\n','shadow.txt':'shadow\n','probe1.txt':'1\n','probe2.txt':'2\n','bad-status.txt':'126\n','missing-command-status.txt':'127\n'}.items():eq(name,value)
eq('interrupt.txt','130\n')
eq('repeat.txt','alpha\n')
for name in ['err.txt','combined.txt','order-err.txt']:
 actual=(p/name).read_text();checks.append(dict(file=name,passed='cat (Mac OS error -43)' in actual))
for name,words in {'bin-list.txt':['ls','df','cat'],'usage.txt':['usage: ls'],'df-human.txt':['Mounted on','MiB'],'explicit.txt':['shadow.txt'],'closed-input.txt':['cat read'],'missing-command.txt':['command not found']}.items():
 actual=(p/name).read_text();checks.append(dict(file=name,passed=all(w in actual for w in words)))
Path('outputs/external-tools/guest-results.json').write_text(json.dumps(checks,indent=2))
assert all(c['passed'] for c in checks),[c for c in checks if not c['passed']]
print(f'{len(checks)} guest output/status checks passed')
