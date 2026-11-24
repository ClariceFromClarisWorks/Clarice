"""Matched production rotation: measured read bytes/calls and host CPU time.
Native HFS model timing is comparative host evidence, not guest timing.
"""
from pathlib import Path
import subprocess,tempfile,shutil
r=Path(__file__).resolve().parents[2]
s=(r/'tests/recovery/native.c').read_text().replace('#include <assert.h>','#include <assert.h>\n#include <time.h>\nstatic unsigned long measured_reads,measured_bytes;')
s=s.replace('memcpy(p,fs[f->file].fork[f->fork]+f->pos,*n);','measured_reads++;measured_bytes+=*n;memcpy(p,fs[f->file].fork[f->fork]+f->pos,*n);')
needle='boundaries=0;assert(!recovery_rotate(&rotation));int total=boundaries;closed_all();'
s=s.replace(needle,'''measured_reads=measured_bytes=0;clock_t started=clock();for(int repeat=0;repeat<100;repeat++){reset();assert(!recovery_rotate(&rotation));closed_all();}printf("COST reads=%lu bytes=%lu cpu_ms=%.3f per100 rotations\\n",measured_reads,measured_bytes,1000.0*(clock()-started)/CLOCKS_PER_SEC);return 0;int total=0;''',1)
with tempfile.TemporaryDirectory() as d:
 t=Path(d);shutil.copytree(r/'src/update',t/'src/update');(t/'tests/recovery').mkdir(parents=True);(t/'tests/recovery/native.c').write_text(s)
 for label in ('before','after'):
  code=subprocess.check_output(['git','show','59e5116:src/update/recovery-native.h'],cwd=r,text=True) if label=='before' else (r/'src/update/recovery-native.h').read_text()
  (t/'src/update/recovery-native.h').write_text(code)
  subprocess.run(['cc','-O2','-Wno-multichar','-I'+str(r/'tests/recovery/include'),str(t/'tests/recovery/native.c'),'-o',str(t/'test')],check=True)
  result=subprocess.check_output([str(t/'test')],text=True);print(label,result.strip())
