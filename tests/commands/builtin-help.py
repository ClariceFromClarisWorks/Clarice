from pathlib import Path
import re,json,subprocess,tempfile
r=Path(__file__).resolve().parents[2];s=(r/'src/help.inc').read_text()
rows=[(json.loads(a),json.loads(b)) for a,b in re.findall(r'\{("(?:[^"\\]|\\.)*"),("(?:[^"\\]|\\.)*")\}',s)]
for n,t in rows:
 assert t.endswith('\n') and len(t.splitlines())>=2,n
 assert all(len(line)<=72 for line in t.splitlines()),n
for n,words in {'read':['-r','IFS','grow','NUL'],'export':['-p','memory'],'unset':['-v','-f'],'printf':['%b','%X','32-bit'],'test':['-L','-h','-ge'],'prefs':['scrollback','control','columns']}.items():
 t=dict(rows)[n];assert all(word in t for word in words),(n,t)
with tempfile.TemporaryDirectory() as d:
 p=Path(d);c='#include <stdio.h>\n#include <string.h>\nstatic void say(const char*s){fputs(s,stdout);}\nstatic void errsay(const char*s){fputs(s,stderr);}\n'+s+'\nint main(int argc,char **argv){return help_command(argc,argv);}\n'
 (p/'test.c').write_text(c);subprocess.run(['cc','-fsanitize=address,undefined',str(p/'test.c'),'-o',str(p/'test')],check=True)
 for n,t in rows:
  q=subprocess.run([str(p/'test'),n],capture_output=True,text=True);assert q.returncode==0 and q.stdout==t and not q.stderr,n
 q=subprocess.run([str(p/'test'),'no-such-builtin'],capture_output=True,text=True);assert q.returncode==1 and q.stderr and not q.stdout
print(f'PASS: {len(rows)} actual builtin help topics, stdout/status, 72-column text and corrected capability descriptions')
