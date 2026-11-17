"""Generate commands for existing disposable images; does not mutate live disks."""
from pathlib import Path
import json
r=Path(__file__).resolve().parents[2];w=r/'work/shell-posix2'
rows=json.loads((r/'outputs/shell-posix2/host.json').read_text());lines=[]
for i,row in enumerate(rows):
 (w/f'p{i}.sh').write_text(row['source']+'\n')
 lines += [f'sh /mnt/Audit/posix/p{i}.sh > /mnt/Audit/posix/o{i} 2> /mnt/Audit/posix/e{i}',f'echo $? > /mnt/Audit/posix/s{i}']
(w/'native-env.sh').write_text('zzlocal=private; zzexport=public; export zzexport; awk \'BEGIN { print "local:" ENVIRON["zzlocal"]; print "export:" ENVIRON["zzexport"] }\'\n')
lines += ['sh /mnt/Audit/posix/native-env.sh > /mnt/Audit/posix/env.out 2> /mnt/Audit/posix/env.err','echo $? > /mnt/Audit/posix/env.status','for i in 1 2 3; do sh /mnt/Audit/posix/p36.sh > /mnt/Audit/posix/repeat; sh /mnt/Audit/posix/p38.sh >> /mnt/Audit/posix/repeat; status runtime >> /mnt/Audit/posix/memory; done','echo POSIX_READY']
(w/'run.sh').write_text('\n'.join(lines)+'\n')
(w/'profile').write_text('export PATH=/mnt/Audit/tools:/bin\ncd /mnt/Audit/run\n'+''.join(f'. /mnt/Audit/elastic/b{i}.sh\n' for i in range(11))+'. /mnt/Audit/posix/run.sh\n')
stage=['set -e','hmount /src/work/parser-safety/audit.hda','hls :posix >/dev/null 2>&1 || hmkdir :posix']
for name in [f'p{i}' for i in range(len(rows))]+['native-env','run']:
 stage.append(f'hcopy -r /src/work/shell-posix2/{name}.sh :posix:{name}.sh')
stage+=['humount','hmount /src/work/parser-safety/system.hda','hcopy -m /src/work/build/Clarice.bin :Clarice','hcopy -r /src/work/shell-posix2/profile ":System Folder:Clarice Data:etc:profile"','humount']
(w/'stage.sh').write_text('\n'.join(stage)+'\n')
print('Generated stage.sh; run with existing hfsutils only while Snow is stopped.')
