"""Run the actual local read loops and service registry at mocked native boundaries."""
from pathlib import Path
import subprocess,re
names=re.findall(r'CLX_IMPORT\("([^"\n]+)"',Path("src/clx-imports.inc").read_text())
assert names==sorted(names), "runtime imports must remain sorted for binary lookup"
app=Path('src/app.c').read_text(); editor=Path('src/editor-screen.inc').read_text()
read=app[app.index('long app_terminal_read('):app.index('/* Even after cancellation')]
screen=editor[editor.index('long app_screen_read('):editor.index('void app_screen_flush_input(')]
harness=r'''
#include <assert.h>
#include <string.h>
#include <stdio.h>
typedef int EventRecord;
#define everyEvent 0
static int tty_offset,tty_count,tty_ready,cursor,tty_reading,scrollback,dirty,cancelled,done;
static char input[1024],tty_buffer[1024];
static struct {unsigned count,head;unsigned char keys[128];} screen_state,*editor_screen=&screen_state;
static int polls,events,draws,mode,remote_connected,remote_typed,executions;
void shell_service_poll(int execute){assert(execute==0);polls++;if(polls==1)remote_connected=1;if(polls==2)remote_typed=1;if(polls==3)remote_connected=0;}
void activity_hide(void){} void draw(void){draws++;dirty=0;}
int WaitNextEvent(int mask,EventRecord *e,int sleep,void *p){(void)mask;(void)e;(void)p;assert(sleep==(mode<2?6:1));events++;assert(polls==events);return 1;}
void handle(EventRecord *e){(void)e;if(events!=3)return;if(mode&1){cancelled=1;return;}if(mode==0){memcpy(tty_buffer,"ok\n",3);tty_count=3;tty_ready=1;}else{editor_screen->keys[0]='q';editor_screen->count=1;}}
'''+read+screen+r'''
int main(void){char out[4];for(mode=0;mode<4;mode++){
 polls=events=draws=remote_typed=remote_connected=cancelled=tty_offset=tty_count=tty_ready=0;screen_state.count=screen_state.head=0;dirty=1;
 long n=mode<2?app_terminal_read(out,sizeof out):app_screen_read(out,sizeof out);
 assert(polls==3&&events==3&&remote_typed&&!remote_connected&&!executions);
 assert(mode&1?n==-1:mode==0?n==3&&!memcmp(out,"ok\n",3):n==1&&out[0]=='q');assert(!tty_reading);
 }puts("PASS actual canonical/editor waits: bounded non-executing service polls, connect/type/disconnect progress, normal input and cancellation, no reentrant execution");}
'''
p=Path('work/network-4c/input-host.c');p.write_text(harness)
subprocess.run(['cc','-fsanitize=address,undefined',str(p),'-o',str(p.with_suffix(''))],check=True)
subprocess.run([str(p.with_suffix(''))],check=True)
registry=r'''
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../../src/service.h"
typedef char *Ptr;static char result[256];static int calls,disposed,state,stops;
static void say(const char *s){strcat(result,s);}int shell_session_active(void){return 0;}void DisposePtr(Ptr p){assert(p);disposed++;}
'''+Path('src/service.inc').read_text()+r'''
static void poll(int execute){assert(!execute);calls++;}static int stop(void){stops++;return 0;}static int status(void){return state;}
int main(void){ClariceService s={poll,stop};for(int i=0;i<10000;i++)shell_service_poll(0);assert(!calls&&!disposed);shell_service_status(0);assert(!strcmp(result,"Telnet service disabled\n"));
 service_loading=1;assert(!cli_service_register_status_slot(0,&s,status));service_image=(Ptr)1;
 int states[]={0,1,-36,2};const char *names[]={"listening\n","connected\n","retrying (Mac OS -36)\n","retrying\n"};
 for(int i=0;i<4;i++){result[0]=0;state=states[i];shell_service_status(0);assert(!strcmp(result+15,names[i]));}
 shell_service_poll(0);assert(calls==1);assert(!cli_service_stop_slot(0)&&disposed==1&&!service_status[0]);
 service_registered=0;assert(!cli_service_register_slot(0,&s));service_image=(Ptr)1;result[0]=0;shell_service_status(0);assert(strstr(result,"status unavailable"));assert(!cli_service_stop_slot(0));
 puts("PASS service status: disabled idle has zero callbacks; active/native-error/retry/legacy status; old registration ABI and stop cleanup");}
'''
p=Path('work/network-4c/registry-host.c');p.write_text(registry)
subprocess.run(['cc','-fsanitize=address,undefined',str(p),'-o',str(p.with_suffix(''))],check=True)
subprocess.run([str(p.with_suffix(''))],check=True)
