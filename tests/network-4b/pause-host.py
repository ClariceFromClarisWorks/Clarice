"""Compile the actual runtime pause function with an instrumented event boundary."""
from pathlib import Path
import subprocess
source=Path('src/app.c').read_text();start=source.index('int cli_pause_ticks(');end=source.index('\n',start)
body=source[start:end]
harness=r'''
#include <assert.h>
#include <stdio.h>
typedef int EventRecord;
static unsigned long ticks;static int dirty,cancelled,events,services,draws,ui;
#define everyEvent 0
unsigned long TickCount(void){return ticks;}
int app_yield(void){services++;return cancelled;}
int shell_session_ui_begin(void){ui++;return 1;}
void shell_session_ui_end(int n){assert(n==1&&ui==1);ui--;}
void draw(void){draws++;dirty=0;}
int WaitNextEvent(int mask,EventRecord *ev,int sleep,void *r){(void)mask;(void)ev;(void)r;assert(ui==1&&sleep==1);ticks++;events++;return 1;}
void handle(EventRecord *e){(void)e;assert(ui==1);}
'''+body+r'''
int main(void){assert(!cli_pause_ticks(3)&&ticks==3&&events==3&&services==3&&!ui);
 cancelled=dirty=1;assert(cli_pause_ticks(6)==1&&ticks==4&&events==4&&draws==1&&!ui);
 assert(cli_pause_ticks(1)==1&&ticks==5&&events==5&&!ui);
 puts("PASS runtime pause: service/event progress, one-tick wait even after cancellation, bounded return, restored UI context");}
'''
p=Path('work/network-4b/pause-host.c');p.write_text(harness)
subprocess.run(['cc','-fsanitize=address,undefined',str(p),'-o',str(p.with_suffix(''))],check=True)
subprocess.run([str(p.with_suffix(''))],check=True)
