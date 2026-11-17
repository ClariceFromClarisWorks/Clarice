"""Long elastic function names must not overflow fixed interactive completion."""
from pathlib import Path
import subprocess
r=Path(__file__).resolve().parents[2];s=(r/'src/shell.c').read_text();a=s.index('static void match_candidate(');b=s.index('\n',a)
w=r/'work/parser-safety';(w/'completion.c').write_text('#include <string.h>\n#include <assert.h>\n#define PATH_CAP 512\n'+s[a:b]+'''\nint main(void){char longname[4096],common[PATH_CAP];int count=0;memset(longname,'a',sizeof longname);longname[sizeof longname-1]=0;memset(common,0,sizeof common);match_candidate(longname,"a",common,&count);assert(count==0);match_candidate("alpha","a",common,&count);assert(count==1&&!strcmp(common,"alpha"));match_candidate("alpine","a",common,&count);assert(count==2&&!strcmp(common,"alp"));return 0;}
''')
subprocess.run(['cc','-fsanitize=address,undefined',str(w/'completion.c'),'-o',str(w/'completion')],check=True);subprocess.run([str(w/'completion')],check=True)
print('PASS long-name completion bounds and ordinary common-prefix completion')
