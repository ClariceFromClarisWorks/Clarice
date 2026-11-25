"""Actual CLI preference dispatcher: no-op, validation and failed-save retry."""
from pathlib import Path
import subprocess,os
r=Path(__file__).resolve().parents[2];s=Path(os.environ.get('CLARICE_TEST_SOURCE',r/'src/app.c')).read_text();out=r/'work/terminal-performance';out.mkdir(exist_ok=True)
a=s.index('int app_preferences(');b=s.index('\nint app_yield',a)
head=r'''
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "control-keys.h"
typedef struct {int v,h;}Point;
typedef int MenuHandle;
static struct {int portRect;}w,*win=&w;
static int font_id=4,font_size=9,columns=66,visible=23,inverse,control_mode=1,retained=128,filled,scrollback;
static int layouts,saves,paints,preferences_unsaved,fail_save,fail_layout;
static long current_line,oldest_line;
static void term_puts(const char *p){(void)p;}
static void term_write(const char*p,long n){(void)p;(void)n;}
static void GetFontName(int id,unsigned char*p){(void)id;p[0]=6;memcpy(p+1,"Monaco",6);}
static void SetPort(void*p){(void)p;}
static void LocalToGlobal(Point*p){(void)p;}
static MenuHandle GetMenuHandle(int n){return n;}
static int CountMItems(MenuHandle m){(void)m;return 0;}
static void GetMenuItemText(MenuHandle m,int i,unsigned char*p){(void)m;(void)i;GetFontName(4,p);}
static void pstr(unsigned char*p,const char*s){p[0]=strlen(s);memcpy(p+1,s,p[0]);}
static int installed_font(unsigned char*p,short *id){*id=4;return p[0]==6&&!memcmp(p+1,"Monaco",6);}
static int fill_window(void){filled=!filled;layouts++;return 1;}
#define COLS 132
#define TEXT_LEFT 4
#define TEXT_TOP 0
static int cell_w=6,line_h=11;
static void geometry_limits(int *w,int *h){*w=416;*h=264;}

static void grid_window(int c,int r,int cw,int lh){(void)cw;(void)lh;columns=c;visible=r-1;}
static int layout(void){layouts++;return !fail_layout;}
static int font_layout(void){layouts++;return !fail_layout;}
static void MoveWindow(void*p,int h,int v,int f){(void)p;(void)h;(void)v;(void)f;}
static void InvalRect(void*p){(void)p;}
static void ValidRect(void*p){(void)p;}
static void save_preferences(void){saves++;preferences_unsaved=fail_save;}
static void term_present(void){paints++;}
static void preferences_window(void){}
'''
grid=s[s.index('static void grid_dimensions'):s.index('static void grid_window')] if 'static void grid_dimensions' in s else ''
number=s[s.index('static int preference_number'):s.index('#include "preferences-format.h"')]
helper=s[s.index('static int unchanged_preferences'):s.index('\n}',s.index('static int unchanged_preferences'))+2] if 'static int unchanged_preferences' in s else ''
main=r'''
static int prefs(char*k,char*v){char *a[]={"prefs",k,v};return app_preferences(3,a);}
int main(void){
 assert(!prefs("size","9")&&!prefs("columns","66")&&!prefs("rows","24")&&!prefs("font","Monaco")&&!prefs("colours","light")&&!prefs("control","option")&&!prefs("scrollback","128"));
 assert(!layouts&&!saves&&!paints);
 assert(!prefs("columns","999")&&!prefs("rows","999"));assert(!layouts&&!saves&&!paints);
 columns=26;visible=8;assert(!prefs("columns","1")&&!prefs("rows","1"));assert(!layouts&&!saves&&!paints);columns=66;visible=23;
 assert(prefs("size","0")==2&&prefs("colours","bogus")==2&&prefs("control","bogus")==2&&prefs("font","missing")==2);assert(!saves&&!layouts);
 preferences_unsaved=1;assert(!prefs("size","9")&&saves==1&&!layouts);assert(!prefs("size","9")&&saves==1);
 fail_save=1;assert(prefs("size","12")!=0&&preferences_unsaved&&layouts==1);int old=saves;
 assert(prefs("size","12")!=0&&saves==old+1&&layouts==1);fail_save=0;
 assert(!prefs("size","12")&&!preferences_unsaved&&layouts==1);old=saves;
 assert(!prefs("size","12")&&saves==old&&layouts==1);
 fail_layout=1;assert(!prefs("size","12")&&layouts==1);assert(prefs("rows","20")!=0&&saves==old);
 puts("PASS actual preference dispatcher: seven unchanged settings, minimum/maximum clamped no-ops, validation, first save, failed-save retry, unchanged under memory pressure, changed-layout failure");
}
'''
p=out/'prefs-host.c';p.write_text(head+grid+number+helper+s[a:b]+main)
subprocess.run(['cc','-fsanitize=address,undefined','-Isrc',str(p),'-o',str(out/'prefs-host')],check=True);subprocess.run([str(out/'prefs-host')],check=True)
