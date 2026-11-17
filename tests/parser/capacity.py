from pathlib import Path
import subprocess,json,tempfile
r=Path(__file__).resolve().parents[2];w=r/'work/parser-safety';rows=[]
cases={
 'script': '#'+('padding '*4096)+'\necho large-script\n',
 'tokens': '; '.join([':']*600)+'; echo many-tokens\n',
 'functions': '\n'.join(f'function_name_longer_than_thirty_one_characters_{i}() {{ echo f{i}; }}' for i in range(48))+'\nfunction_name_longer_than_thirty_one_characters_0\nfunction_name_longer_than_thirty_one_characters_47\n( function_name_longer_than_thirty_one_characters_0 )\n',
 'arguments': 'set -- '+' '.join('arg'+str(i) for i in range(200))+'; echo "$#" "${200}"\n',
 'recover': 'if then\n',
 'nested': '(' * 50 + ':' + ')' * 50,
 'andor': ' && '.join([':']*100),
 'arithmetic': 'echo $(('+'+'.join(['1']*100)+'))',
}
for name,source in cases.items():
 (w/(name+'.sh')).write_text(source)
 with tempfile.TemporaryDirectory() as td:
  p=subprocess.run([str(r/'work/sh-host'),source],cwd=td,capture_output=True,timeout=10)
  if name in ['recover','nested','andor','arithmetic']:
   ok=p.returncode==2 and b'Sanitizer' not in p.stderr
  else:
   d=subprocess.run(['dash','-c',source],cwd=td,capture_output=True)
   ok=(p.returncode,p.stdout,p.stderr)==(d.returncode,d.stdout,d.stderr)
  rows.append(dict(case=name,bytes=len(source),status=p.returncode,stdout=p.stdout.decode(),stderr=p.stderr.decode(),passed=ok))
  assert ok,rows[-1]
print('PASS',len(rows),'capacity and stack-safety cases')
(r/'outputs/parser-safety/capacity.json').write_text(json.dumps(rows,indent=2)+'\n')
