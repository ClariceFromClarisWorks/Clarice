"""Run the production thumb tracker with a held button and a pending key."""
from pathlib import Path
import subprocess,sys
r=Path(__file__).resolve().parents[2];s=(Path(sys.argv[1]) if len(sys.argv)>1 else r/'src/app.c').read_text();out=r/'work/terminal-correctness'
a=s.index('static void drag_history(');b=s.index('\n}',a)+2
helper=s[s.index('static int tracking_key_pending'):s.index('/* Track the view with the thumb')] if 'static int tracking_key_pending' in s else ''
head=r'''
#include <assert.h>
#include <stdio.h>
typedef struct{int top,bottom;}Rect;
typedef struct{Rect contrlRect;}Control;
static Control c={{0,200}},*cp=&c,**history_bar=&cp;
typedef struct{int v,h;}Point;
typedef struct{int what;}EventRecord;
#define keyDownMask 8
#define autoKeyMask 32
static int cycles,scrollback,dirty,queued;
static int EventAvail(int mask,EventRecord *e){(void)e;assert(mask==(keyDownMask|autoKeyMask));return queued&&cycles>=3;}
static void GetMouse(Point*p){p->v++;}
static int StillDown(void){return cycles<20;}
static int scroll_limit(void){return 100;}
static int GetControlValue(Control**h){(void)h;return 50;}
static void selection_clear(void){}
static unsigned long TickCount(void){return cycles;}
static void draw(void){dirty=0;}
static void SystemTask(void){cycles++;}
'''
body=r'''
int main(void){Point p={0,0};queued=1;drag_history(p);assert(cycles<=4&&queued);puts("PASS pending cancellation/ordinary key ends thumb tracking without consuming it");
cycles=0;queued=0;drag_history(p);assert(cycles==20);puts("PASS no key preserves tracking until button release");}
'''
p=out/'tracking.c';p.write_text(head+helper+s[a:b]+body)
subprocess.run(['cc','-fsanitize=address,undefined',str(p),'-o',str(out/'tracking')],check=True);subprocess.run([str(out/'tracking')],check=True)
# Held arrows and selection use the identical restricted predicate.
assert s.count('while(StillDown()&&!tracking_key_pending())')==2
