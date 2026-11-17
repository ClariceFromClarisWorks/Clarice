from pathlib import Path
import json,shutil,shlex
r=Path.cwd();w=r/'work/parser-safety';old=r/'work/shell-elastic'
for name,src in [('audit.hda',old/'audit.hda'),('system.hda',r/'work/utility-correctness/system.hda')]:
 if not (w/name).exists():shutil.copyfile(src,w/name)
c=json.loads((old/'plus.snoww').read_text());c['scsi_targets'][0]={'Disk':str(w/'system.hda')};c['scsi_targets'][1]={'Disk':str(w/'audit.hda')};c['pram_path']=str(w/'plus.pram');shutil.copyfile(r/'work/utility-correctness/plus.pram',w/'plus.pram');(w/'plus.snoww').write_text(json.dumps(c))
source='original() { :; }; for x in one two; do printf "%s\\n" "$x"; done > /tmp/parser-output; value=$(printf abc); ( original ); echo $((1+2*3))'
(w/'faults.sh').write_text('original() { :; }\n'+f'parsercheck 0 0 {shlex.quote(source)}\n'+'for sticky in 0 1; do i=1; while test "$i" -le 220; do\n'+f'parsercheck "$i" "$sticky" {shlex.quote(source)}\n'+'original || echo LOST_FUNCTION\ni=$((i+1))\ndone; done\necho FAULTS_DONE\n')
lines=[]
for name in ['script','tokens','functions','arguments','recover','nested','andor','arithmetic']:
 lines += [f'sh /mnt/Audit/parser/{name}.sh > /mnt/Audit/parser/{name}.out 2> /mnt/Audit/parser/{name}.err',f'echo $? > /mnt/Audit/parser/{name}.status']
lines+=['sh /mnt/Audit/parser/faults.sh > /mnt/Audit/parser/faults.out 2> /mnt/Audit/parser/faults.err','for i in 1 2 3 4 5; do sh /mnt/Audit/parser/functions.sh > /mnt/Audit/parser/repeat; status runtime >> /mnt/Audit/parser/memory; done','echo PARSER_READY']
(w/'run.sh').write_text('\n'.join(lines)+'\n')
(w/'profile').write_text('export PATH=/mnt/Audit/tools:/bin\ncd /mnt/Audit/run\n'+''.join(f'. /mnt/Audit/elastic/b{i}.sh\n' for i in range(11))+'. /mnt/Audit/parser/run.sh\n')
stage=['set -e','hmount /src/work/parser-safety/audit.hda','hmkdir :parser']
for name in ['script','tokens','functions','arguments','recover','nested','andor','arithmetic','faults','run']:
 stage.append(f'hcopy -r /src/work/parser-safety/{name}.sh :parser:{name}.sh')
stage+=['humount','hmount /src/work/parser-safety/system.hda','hcopy -m /src/work/build/Clarice.bin :Clarice','hcopy -r /src/work/parser-safety/profile ":System Folder:Clarice Data:etc:profile"','humount']
(w/'stage.sh').write_text('\n'.join(stage)+'\n')
