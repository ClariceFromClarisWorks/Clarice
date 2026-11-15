/* Opt-in focused guest checks; no production service. */
static short usability_ref=-1;
static int usability_failures;
static void usability_record(const char *label,int ok){char b[180];snprintf(b,sizeof b,"%s %s\n",ok?"PASS":"FAIL",label);long n=strlen(b);FSWrite(usability_ref,&n,b);if(!ok)usability_failures++;}
static void usability_command(unsigned char c){EventRecord ev;memset(&ev,0,sizeof ev);ev.what=keyDown;ev.message=c;ev.modifiers=cmdKey;handle(&ev);}
static void usability_reset(void){busy=cancelled=interrupted=input_blocked=0;input[0]=continued[0]=0;cursor=0;ahead_head=ahead_count=0;select_start=select_end=-1;}
#include "ux-guest.inc"
static void usability_probe(void){char path[PATH_CAP];unsigned char pn[256];shell_temp_path(path,"usability.txt");pstr(pn,path);HCreate(0,0,pn,'CLRC','TEXT');if(HOpen(0,0,pn,fsWrPerm,&usability_ref))return;SetEOF(usability_ref,0);
 usability_record("launch window in front",FrontWindow()==win);usability_reset();draw();
 memcpy(last_screen[0],"copyme",6);select_start=0;select_end=6;usability_command('c');Handle h=NewHandle(0);long off,n=GetScrap(h,'TEXT',&off);usability_record("Command-C selected text",n==6&&!memcmp(*h,"copyme",6));DisposeHandle(h);select_start=select_end=-1;
 strcpy(input,"ab");cursor=1;ZeroScrap();PutScrap(2,'TEXT',"XY");usability_command('v');usability_record("Command-V at insertion point",!strcmp(input,"aXYb")&&cursor==3);
 usability_reset();busy=1;key('a',0);key('b',0);key(8,0);key('c',0);busy=0;while(ahead_count){unsigned at=ahead_head;ahead_head=(ahead_head+1)%LINE_CAP;ahead_count--;key(ahead_keys[at],ahead_mods[at]);}usability_record("foreground ordered typeahead editing",!strcmp(input,"ac"));
 usability_reset();busy=1;ZeroScrap();PutScrap(3,'TEXT',"a\rb");usability_command('v');usability_record("foreground paste queued atomically",ahead_count==3&&!input[0]);busy=0;unsigned long commands=shell_commands;while(ahead_count){unsigned at=ahead_head;ahead_head=(ahead_head+1)%LINE_CAP;ahead_count--;key(ahead_keys[at],ahead_mods[at]);}usability_record("queued paste newline never submits",!strcmp(input,"a\nb")&&shell_commands==commands);
 usability_reset();busy=1;for(int i=0;i<LINE_CAP;i++)key('x',0);ZeroScrap();PutScrap(1,'TEXT',"y");usability_command('v');usability_record("busy paste overflow fails closed",input_blocked&&!ahead_count&&cancelled);
 usability_reset();busy=1;key('z',0);int e=app_screen_open();unsigned char c=0;long got=editor_screen&&app_screen_pending()?app_screen_read(&c,1):0;usability_record("loading typeahead transferred to editor",!e&&got==1&&c=='z'&&!ahead_count);app_screen_close();usability_reset();
 force_draw=dirty=1;draw();busy=activity_command=1;activity_interactive=0;activity_output_tick=TickCount()-301;activity_tick(TickCount());usability_record("pinwheel inverse background",activity_visible&&GetPixel(activity_rect.right-1,activity_rect.top)==!inverse);activity_hide();busy=activity_command=0;
 ux_probe();
 char b[100];snprintf(b,sizeof b,"failures=%d heap=%ld\n",usability_failures,FreeMem());n=strlen(b);FSWrite(usability_ref,&n,b);FSClose(usability_ref);usability_ref=-1;FlushVol(0,0);usability_reset();force_draw=dirty=1;term_puts("USABILITY-PROBE-DONE\n");
}
