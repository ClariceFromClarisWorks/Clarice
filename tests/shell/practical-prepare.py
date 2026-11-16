from pathlib import Path
import json,shlex
r=Path(__file__).resolve().parents[2];w=r/'work/shell-practical';fixtures=r/'tests/shell/practical'
rows=json.loads((r/'outputs/shell-practical/host.json').read_text());lines=['cd /mnt/Audit/practical']
for i,row in enumerate(rows):lines += [row['source']+f' > o{i} 2> e{i}',f'echo $? > s{i}']
errors=json.loads((r/'outputs/shell-practical/diagnostics.json').read_text())
for i,row in enumerate(errors):lines += ['sh -c '+shlex.quote(row['source'])+f' > d{i}.out 2> d{i}.err',f'echo $? > d{i}.status','echo RECOVERED >> recovered']
env='\n'.join(f'export E{i}='+ 'x'*512 for i in range(30))+'''\nawk 'BEGIN { n=0; for(k in ENVIRON) if(k ~ /^E[0-9]/)n++; print n, length(ENVIRON["E29"]) }'\n'''
(w/'environment.sh').write_text(env)
lines += ['for i in 1 2 3; do sh /mnt/Audit/practical/environment.sh >> env.out; echo $? >> env.status; status runtime >> memory; done','echo PRACTICAL_READY']
(w/'run.sh').write_text('\n'.join(lines)+'\n');(w/'profile').write_text('export PATH=/mnt/Audit/tools:/bin\n. /mnt/Audit/practical/run.sh\n')
stage=['set -e','hmount /src/work/parser-safety/audit.hda','hmkdir :practical 2>/dev/null || true']
for p in fixtures.iterdir():
 if p.suffix=='.sh' or p.name in ['Caps','ncurses_defs']:stage.append(f'hcopy -r /src/tests/shell/practical/{p.name} :practical:{p.name}')
stage += ['hcopy -r /src/tests/shell/practical/Caps ":practical:caps space"','hcopy -r /src/tests/shell/practical/ncurses_defs ":practical:definitions space"']
for name in ['run','environment']:stage.append(f'hcopy -r /src/work/shell-practical/{name}.sh :practical:{name}.sh')
stage+=['hcopy -r /src/work/build/awk.clx :tools:awk','humount','hmount /src/work/parser-safety/system.hda','hcopy -m /src/work/build/Clarice.bin :Clarice','hcopy -r /src/work/shell-practical/profile ":System Folder:Clarice Data:etc:profile"','humount'];(w/'stage.sh').write_text('\n'.join(stage)+'\n')
