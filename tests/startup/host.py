"""Compile the production startup directory loop/report gate with fault stubs."""
from pathlib import Path
import subprocess
r=Path(__file__).resolve().parents[2];s=(r/'src/namespace.inc').read_text()
loop=s[s.index('static const char *dirs[]='):s.index(' snprintf(part,sizeof part,"%.240setc:profile"')]
reports=s[s.index('static void migrate_reports(void)'):]
reports=reports[:reports.index('\n}')+2]
code=r'''
#include <assert.h>
#include <stdio.h>
#include <string.h>
#define PATH_CAP 512
#define fnfErr -43
#define dirNFErr -120
typedef struct {struct {int ioFlAttrib;} hFileInfo;} CInfoPBRec;
static char root_path[512]="Disk:System:Clarice Data:";
static int present,creates,checks,bad,fail,create_fail,migrates;
static void pstr(unsigned char *n,const char *s){n[0]=strlen(s);strcpy((char*)n+1,s);}
static int info(const char *s,CInfoPBRec *c){(void)s;checks++;c->hFileInfo.ioFlAttrib=bad?0:16;return fail?fail:present?0:fnfErr;}
static int create_dir(unsigned char *s,long *id){(void)s;*id=1;creates++;return create_fail;}
static int migrate_private_file(char *p,const char *f,const char *n,const char *old){(void)p;(void)f;(void)n;(void)old;migrates++;return 0;}
static int ensure_dirs(void){char part[512];unsigned char name[256];int e;long id;CInfoPBRec cat;
'''+loop+r'''
return 0;}
'''+reports+r'''
int main(void){present=1;assert(!ensure_dirs()&&creates==0&&checks==10);
 present=0;checks=0;assert(!ensure_dirs()&&creates==10&&checks==10);
 create_fail=-34;creates=0;assert(ensure_dirs()==-34&&creates==1);create_fail=-108;assert(ensure_dirs()==-108);create_fail=0;
 present=1;bad=1;creates=0;assert(ensure_dirs()==dirNFErr&&!creates);bad=0;
 fail=-108;assert(ensure_dirs()==-108&&!creates);fail=-44;assert(ensure_dirs()==-44&&!creates);
 fail=0;present=0;checks=0;migrate_reports();assert(checks==5&&!migrates);
 present=1;checks=0;migrate_reports();assert(checks==5&&migrates==5);
 fail=-36;migrate_reports();assert(migrates==10);
 puts("PASS existing/fresh directories, wrong-kind path, native OOM/read-only errors; absent/existing/error legacy-report gates; no allocation");}
'''
w=r/'work/startup-q1';w.mkdir(parents=True,exist_ok=True);(w/'host.c').write_text(code)
subprocess.run(['cc','-O1','-fsanitize=address,undefined',str(w/'host.c'),'-o',str(w/'host')],check=True)
subprocess.run([str(w/'host')],check=True)
app=(r/'src/app.c').read_text();start=app.index(' {Rect frame=(*((WindowPeek)win)->strucRgn)->rgnBBox;');end=app.index('\n',start)
placement=app[start:end]
code=r'''
#include <assert.h>
#include <stdio.h>
typedef struct {int top,left,bottom,right;} Rect;
typedef struct {Rect rgnBBox;} Region;
typedef struct {Region **strucRgn;} Window;
typedef Window *WindowPeek;
static struct {struct {Rect bounds;} screenBits;} qd;
static Region region,*rp=&region;static Window w={&rp},*win=&w;static Rect r;
static int bar=20,moves;static int menu_bar_height(void){return bar;}
static void MoveWindow(Window *w,int x,int y,int front){(void)w;(void)front;region.rgnBBox.left+=x-r.left;region.rgnBBox.top+=y-r.top;moves++;}
static void position(void){
'''+placement+r'''
}
int main(void){for(int screen=0;screen<2;screen++)for(int border=1;border<5;border++)for(int title=18;title<25;title++){
 qd.screenBits.bounds.left=screen*100;qd.screenBits.bounds.top=screen*40;bar=20+screen*4;
 r.left=qd.screenBits.bounds.left+3;r.top=qd.screenBits.bounds.top+bar+21;
 region.rgnBBox.left=r.left-border;region.rgnBBox.top=r.top-title;moves=0;position();
 assert(region.rgnBBox.left==qd.screenBits.bounds.left+2);assert(region.rgnBBox.top==qd.screenBits.bounds.top+bar+2);
 if(border==1&&title==19)assert(!moves);else assert(moves==1);
 }puts("PASS actual startup positioning: standard/no-move and alternate WDEF borders, menu heights and nonzero screens");}
'''
(w/'placement.c').write_text(code)
subprocess.run(['cc','-O1','-fsanitize=address,undefined',str(w/'placement.c'),'-o',str(w/'placement')],check=True)
subprocess.run([str(w/'placement')],check=True)
