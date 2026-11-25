"""Exercise production menu dispatch: dialog dismissal must not invent damage."""
from pathlib import Path
import os,subprocess
root=Path(__file__).resolve().parents[2];s=Path(os.environ.get('CLARICE_TEST_SOURCE',root/'src/app.c')).read_text()
a=s.index('static void menu(long choice){');b=s.index('/* Moving a fixed selection',a)
code=r'''
#include <assert.h>
#include <stdio.h>
typedef int MenuHandle;
static int dirty,force_draw,busy,editor_screen,serial_input,font_id,font_size=9,inverse,dialogs;
static struct {int portRect;}window,*win=&window;
#define welcome() ((void)0)
#define GetMenuHandle(n) (n)
#define GetMenuItemText(a,b,c) ((void)0)
#define OpenDeskAcc(n) ((void)0)
#define SetPort(n) ((void)0)
#define app_clear() ((void)0)
#define key(a,b) ((void)0)
#define cmdKey 256
#define app_quit() ((void)0)
#define copy_selection() ((void)0)
#define paste() ((void)0)
#define history_move(n) ((void)0)
#define app_run_checks() ((void)0)
#define app_metrics(n) ((void)0)
#define GetFNum(n,p) (*(p)=0)
#define unchanged_preferences() ((void)0)
#define font_layout() ((void)0)
#define InvalRect(n) ((void)0)
#define save_preferences() ((void)0)
#define fill_window() ((void)0)
#define HiliteMenu(n) ((void)0)
static void preferences_window(void){dialogs++;}
'''+s[a:b]+r'''
int main(void){
 menu((129L<<16)|4);assert(dialogs==1&&!dirty&&!force_draw);
 dirty=force_draw=1;menu((129L<<16)|4);assert(dialogs==2&&dirty&&force_draw);
 puts("PASS Preferences menu keeps clean state clean and preserves genuine pending damage");
}
'''
p=root/'work/terminal-closure/menu.c';p.write_text(code)
subprocess.run(['cc','-fsanitize=address,undefined',str(p),'-o',str(p.with_suffix(''))],check=True)
subprocess.run([str(p.with_suffix(''))],check=True)
