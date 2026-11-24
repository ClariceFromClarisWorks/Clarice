"""The CLI must propagate the real layout result before persisting dimensions."""
from pathlib import Path
import subprocess,tempfile
s=Path('src/app.c').read_text();a=s.index(' /* Grid-only changes');b=s.index('\nint app_yield',a)
body=s[a:b]
code='''#include <assert.h>\n#include <string.h>\n#include <stdio.h>\nstruct Point{int h,v;};static int filled=1,saves,paint,accepted,moves;static struct {int portRect;}window,*win=&window;static int layout(void){return accepted;}static int font_layout(void){return accepted;}static void MoveWindow(void*w,int h,int v,int f){moves++;}static void SetPort(void*w){}static void InvalRect(void*r){}static void ValidRect(void*r){}static void save_preferences(void){saves++;}static void term_present(void){paint++;}static int call(char**argv){struct Point previous={0};'''+body+'''\nint main(){char *argv[]={"prefs","columns","60"};assert(call(argv)==1&&!saves&&!paint&&filled&&moves==1);accepted=1;assert(!call(argv)&&saves==1&&paint==1&&!filled);puts("PASS failed CLI layout returns nonzero before save/present; success persists");}'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d)/'test.c';p.write_text(code);subprocess.run(['cc','-fsanitize=address,undefined',str(p),'-o',d+'/test'],check=True);subprocess.run([d+'/test'],check=True)
