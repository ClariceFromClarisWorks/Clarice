from pathlib import Path
import subprocess,json,re
r=Path(__file__).resolve().parents[2];rows=[]
for source in ['1+2*3','x=(1+2)*-3','a?b:c','a<<2|b^c&d','a==b||c>=d&&e','1+','a?b:', 'x+=4','(' * 50+'1'+')'*50,'+'.join(['1']*100),'|'.join(['1']*100)]:
 p=subprocess.run([str(r/'work/parser-safety/parse'),'all',source,'single','arithmetic'],capture_output=True,timeout=30)
 assert p.returncode==0,(source,p.returncode,p.stdout,p.stderr)
 n=int(re.search(rb'trials=(\d+)',p.stdout)[1]);rows.append(dict(source=source,trials=n,passed=True))
(r/'outputs/parser-safety/arithmetic.json').write_text(json.dumps(rows,indent=2)+'\n')
print('PASS',sum(x['trials'] for x in rows),'arithmetic failure/recovery trials')
