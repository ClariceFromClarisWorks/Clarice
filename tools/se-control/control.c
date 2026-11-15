/* GPL-3.0-or-later. Isolated System 6 hardware-control utility. No trap/ROM patches or Clarice hooks.
 * ScreenBits capture follows Macinclaude's screen.inc approach (MIT).
 * Network adapter is its separately credited nettcp.c; see vendor/LICENSE.
 * Protocol: HIL2 token, GRAB, KEY char keycode modifiers, PING, QUIT (LF lines).
 * No remote file access, shell execution, persistent installation or auto-start.
 */
#include <Quickdraw.h>
#include <Windows.h>
#include <Fonts.h>
#include <Dialogs.h>
#include <Events.h>
#include <OSUtils.h>
#include <Memory.h>
#include <Files.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nettcp.h"
#include "mouse.h"
#include <Menus.h>
#define controlKey 0x1000

static WindowPtr window;
static NetConn conn;
static int done, selftest, observed, background, local_script;
static MousePlan *mouse;
static FILE *script,*logfile;
static unsigned long script_wait;
static char status[160] = "SE Control: standalone development helper";
static unsigned long last_command, capture_ticks;

static void draw_status(void) {
 SetPort(window); EraseRect(&window->portRect); TextFont(4); TextSize(9);
 MoveTo(8,18); DrawText(status,0,strlen(status));
 MoveTo(8,38); DrawString((ConstStringPtr)"\pClose this window to stop. No OS patches installed.");
 MoveTo(8,58); DrawString((ConstStringPtr)"\pSnapshot proof: ABC xyz 0123456789");
 Rect r={75,8,99,72}; PaintRect(&r); r.left=82;r.right=146;FrameRect(&r);
 MoveTo(8,118); DrawString((ConstStringPtr)"\pLast received character: "); if(observed)DrawChar(observed);
}
static void notice(const char *s) {strncpy(status,s,sizeof status-1);status[sizeof status-1]=0;draw_status();}
static int send_line(const char *s) {if(local_script){fputs(s,logfile);fflush(logfile);return 1;}return NetSend(&conn,s,strlen(s))==noErr;}

/* One immutable 21,888-byte snapshot; never stream a changing framebuffer. */
static unsigned char *snapshot(void) {
 BitMap *b=&qd.screenBits;
 if(b->bounds.right-b->bounds.left!=512 || b->bounds.bottom-b->bounds.top!=342 || (b->rowBytes&0x3fff)!=64)return NULL;
 unsigned long start=TickCount(); unsigned char *p=(unsigned char*)NewPtr(21888);
 if(!p)return NULL;
 memcpy(p,b->baseAddr,21888); capture_ticks=TickCount()-start;return p;
}
static int grab(void) {
 unsigned long started=TickCount();
 unsigned char *p=snapshot(); if(!p)return send_line("ERR capture\n");
 char h[128];snprintf(h,sizeof h,"FRAME 512 342 64 21888 %lu %ld\n",capture_ticks,(long)FreeMem());
 int ok=send_line(h);
 for(long i=0;ok&&i<21888;i+=1024){
  if(TickCount()-started>60UL*15){ok=0;break;}
  unsigned short n=21888-i>1024?1024:21888-i;ok=NetSend(&conn,p+i,n)==noErr;
 }
 DisposePtr((Ptr)p);return ok;
}
static int save_capture(const char *name) {
 unsigned char *p=snapshot();if(!p)return 0;
 FILE *f=fopen(name,"wb");int ok=0;
 if(f){ok=fwrite("P4\n512 342\n",1,11,f)==11 && fwrite(p,1,21888,f)==21888;if(fclose(f))ok=0;}
 DisposePtr((Ptr)p);return ok;
}
static OSErr key(unsigned int ch,unsigned int code,unsigned int mods) {
 volatile short *p=(volatile short*)0x082c;
 return hil_post(keyDown,((long)code<<8)|ch,mods|btnState,p[1],p[0]);
}
static int mouse_plan(int kind,int x,int y,int x2,int y2,int ticks) {
 if(!mouse||mouse->active||!(*(volatile unsigned char*)0x0172&0x80))return 0;
 if(x<0||x>511||y<0||y>341||x2<0||x2>511||y2<0||y2>341)return 0;
 mouse->index=mouse->count=mouse->error=mouse->cancel=0;
 if(kind==0){mouse->steps[0]=(MouseStep){x,y,0,1};mouse->count=1;}
 else if(kind==1||kind==2){
  for(int i=0;i<kind;i++){
   mouse->steps[mouse->count++]=(MouseStep){x,y,1,1+i*12};
   mouse->steps[mouse->count++]=(MouseStep){x,y,0,7+i*12};
  }
 }else{
  if(ticks<30||ticks>180)return 0;
  mouse->steps[mouse->count++]=(MouseStep){x,y,1,1};
  for(int i=1;i<=24;i++)mouse->steps[mouse->count++]=(MouseStep){x+(x2-x)*i/24,y+(y2-y)*i/24,1,6+(ticks-12)*i/24};
  mouse->steps[mouse->count++]=(MouseStep){x2,y2,0,ticks};
 }
 mouse->started=TickCount();mouse->active=1;PrimeTime((QElemPtr)&mouse->task,16);return 1;
}
static int command(const char *s) {
 last_command=TickCount();
 if(!strcmp(s,"GRAB"))return grab();
 if(!strcmp(s,"PING")){char b[150];snprintf(b,sizeof b,"PONG %lu %ld bg=%d active=%d down=%d error=%d state=%lu\n",TickCount(),(long)FreeMem(),background,mouse->active,mouse->down,mouse->error,(unsigned long)sizeof *mouse);return send_line(b);}
 if(!strcmp(s,"CANCEL")){mouse->cancel=1;return send_line("CANCELLING\n");}
 int x,y,x2,y2,duration;unsigned int count;char tail;
 if(sscanf(s,"MOVE %d %d %c",&x,&y,&tail)==2)return send_line(mouse_plan(0,x,y,x,y,0)?"QUEUED\n":"ERR mouse busy/range\n");
 if(sscanf(s,"CLICK %d %d %u %c",&x,&y,&count,&tail)==3&&count>=1&&count<=2)return send_line(mouse_plan(count,x,y,x,y,0)?"QUEUED\n":"ERR mouse busy/range\n");
 if(sscanf(s,"DRAG %d %d %d %d %d %c",&x,&y,&x2,&y2,&duration,&tail)==5)return send_line(mouse_plan(3,x,y,x2,y2,duration)?"QUEUED\n":"ERR mouse busy/range\n");
 if(!strcmp(s,"QUIT")){done=1;return send_line("BYE\n");}
 unsigned int ch,code,mods;char extra;
 if(sscanf(s,"KEY %u %u %u %c",&ch,&code,&mods,&extra)==3 && ch<256&&code<128 && !(mods&~(cmdKey|shiftKey|optionKey|controlKey))){
  if(mouse->active)return send_line("ERR mouse busy\n");
  OSErr e=key(ch,code,mods);return send_line(e?"ERR event post\n":"QUEUED\n");
 }
 return send_line("ERR command\n");
}
static void events(void) {
 EventRecord e={0};
 WaitNextEvent(everyEvent,&e,6,NULL);
 if(e.what==nullEvent)return;
 if(local_script&&logfile){fprintf(logfile,"EVENT %d %08lx %04x\n",e.what,(unsigned long)e.message,e.modifiers);fflush(logfile);}
 switch(e.what){
 case osEvt: if((e.message>>24)==1)background=!(e.message&1);break;
 case updateEvt: BeginUpdate((WindowPtr)e.message);draw_status();EndUpdate((WindowPtr)e.message);break;
 case activateEvt: if(e.modifiers&activeFlag)SetPort(window);break;
 case mouseDown:{WindowPtr w;short part=FindWindow(e.where,&w);
  if(part==inMenuBar){long m=MenuSelect(e.where);if((m>>16)==129&&(m&65535)==1)done=1;HiliteMenu(0);}
  else if(part==inGoAway&&w==window&&TrackGoAway(window,e.where))done=1;
  else if(part==inDrag&&w==window)DragWindow(window,e.where,&qd.screenBits.bounds);
  else if(part==inSysWindow)SystemClick(&e,w);
  else if(part==inContent&&w==window)SelectWindow(window);
  break;}
 case keyDown:
  if((e.modifiers&cmdKey)&&(e.message&255)=='q')done=1;
  else {observed=e.message&255;draw_status();}break;
 }
}
int main(void) {
 InitGraf(&qd.thePort);InitFonts();InitWindows();InitMenus();TEInit();InitDialogs(NULL);InitCursor();
 MenuHandle apple=NewMenu(128,(ConstStringPtr)"\p\024");AppendMenu(apple,(ConstStringPtr)"\pAbout SE Control");InsertMenu(apple,0);
 MenuHandle file=NewMenu(129,(ConstStringPtr)"\pFile");AppendMenu(file,(ConstStringPtr)"\pQuit/Q");InsertMenu(file,0);DrawMenuBar();
 mouse=(MousePlan*)NewPtrSysClear(sizeof *mouse);if(!mouse)return 1;mouse->task.tmAddr=(ProcPtr)hil_timer;InsTime((QElemPtr)&mouse->task);
 Rect bounds={45,16,190,496};window=NewWindow(NULL,&bounds,(ConstStringPtr)"\pSE Control",true,documentProc,(WindowPtr)-1,true,0);
 if(!window){RmvTime((QElemPtr)&mouse->task);DisposePtr((Ptr)mouse);return 1;}
 SetPort(window);draw_status();
 script=fopen("HIL Script","r");
 if(script){local_script=1;logfile=fopen("HIL-script-result.txt","w");if(!logfile){fclose(script);script=NULL;local_script=0;}else script_wait=TickCount()+120;}
 FILE *f=fopen("HIL Selftest","rb");if(f){fclose(f);selftest=1;}
 if(selftest){
  unsigned long start=TickCount();while(TickCount()-start<90)events();
  long idle=FreeMem();int before=save_capture("HIL-before.pbm");unsigned long first=capture_ticks;
  OSErr posted=key('x',7,0);start=TickCount();while(!observed&&TickCount()-start<120)events();
  unsigned long latency=TickCount()-start;int after=save_capture("HIL-after.pbm");
  f=fopen("HIL-result.txt","w");if(f){fprintf(f,"before=%d after=%d post=%d received=%d latency_ticks=%lu capture_ticks=%lu idle=%ld final=%ld\n",before,after,posted,observed,latency,first,idle,(long)FreeMem());fclose(f);}
  FlushVol(NULL,0);RmvTime((QElemPtr)&mouse->task);hil_release(mouse);DisposePtr((Ptr)mouse);DisposeWindow(window);return observed=='x'&&before&&after?0:1;
 }
 if(local_script)goto run;
 char host[32],token[65],extra;unsigned int port;
 f=fopen("SE Control.cfg","r");int valid=f&&fscanf(f,"%31s %u %64s %c",host,&port,token,&extra)==3;
 if(f)fclose(f);
 unsigned long ip=valid?NetParseIP(host):0;
 /* Numeric private/loopback destinations only. No DNS or inbound SE listener. */
 int local=(ip>>24)==10 || (ip>>16)==0xc0a8 || ((ip>>20)==0xac1) || (ip>>24)==127;
 if(!valid||!local||!port||port>65535||strlen(token)!=32){notice("Missing/invalid SE Control.cfg. See README.");while(!done)events();goto cleanup;}
 notice("Connecting to configured local host...");
 OSErr err=NetConnect(&conn,ip,port);
 if(err){char b[100];snprintf(b,sizeof b,"MacTCP connect error %d. Close to exit.",err);notice(b);while(!done)events();goto cleanup;}
 char hello[80];snprintf(hello,sizeof hello,"HIL2 %s\n",token);
 if(!send_line(hello))goto cleanup;
 notice("Connected. Background screen and input control.");
run:
 last_command=TickCount();
 char line[128];unsigned int used=0;
 while(!done){
  events();if(done)break;
  if(TickCount()-last_command>60UL*300){notice("Session idle timeout. Close or relaunch.");break;}
  if(local_script){
   if((long)(TickCount()-script_wait)>=0){char b[128];
    if(!fgets(b,sizeof b,script)){done=1;break;}b[strcspn(b,"\r\n")]=0;
    /* Offline fixture only: withhold all main-loop/event/host servicing.
       Time Manager must release without any subsequent network command. */
    if(!strcmp(b,"CHECK-CANCEL")||!strcmp(b,"CHECK-NOHOST")||!strcmp(b,"CHECK-EXIT")){
     int started=mouse_plan(3,430,260,450,280,180);
     unsigned long at=TickCount();while(TickCount()-at<15){}
     int held=mouse->down;
     if(!strcmp(b,"CHECK-CANCEL"))mouse->cancel=1;
     if(!strcmp(b,"CHECK-EXIT")){
      fprintf(logfile,"CHECK-EXIT started=%d held=%d\n",started,held);done=1;continue;
     }
     at=TickCount();while(mouse->active&&TickCount()-at<250){}
     fprintf(logfile,"%s started=%d held=%d active=%d down=%d button=%u elapsed=%lu error=%d\n",b,started,held,mouse->active,mouse->down,(unsigned int)*(volatile unsigned char*)0x0172,TickCount()-mouse->started,mouse->error);fflush(logfile);continue;
    }
    if(!strncmp(b,"WAIT ",5)){script_wait=TickCount()+strtoul(b+5,NULL,10);continue;}
    if(!strncmp(b,"SAVE ",5)){fprintf(logfile,"SAVE %s %d\n",b+5,save_capture(b+5));fflush(logfile);}
    else command(b);
   }continue;
  }
  long available=NetAvailable(&conn);if(available<0)break;
  if(available){char bytes[64];long n=NetRecv(&conn,bytes,available>64?64:available,1);if(n<=0)break;
   for(long i=0;i<n;i++){char c=bytes[i];if(c=='\n'){line[used]=0;used=0;if(!command(line)){done=1;break;}}
    else if(c=='\r')continue;
    else if(c<32||c>126||used==sizeof line-1){done=1;break;}
    else line[used++]=c;}
  }
 }
cleanup:
 RmvTime((QElemPtr)&mouse->task);hil_release(mouse);
 if(local_script&&logfile)fprintf(logfile,"CLEANUP active=%d down=%d button=%u mb_delta=%ld\n",mouse->active,mouse->down,(unsigned int)*(volatile unsigned char*)0x0172,(long)(*(volatile unsigned long*)0x016e - TickCount()));
 DisposePtr((Ptr)mouse);
 if(script)fclose(script);
 if(logfile)fclose(logfile);
 NetClose(&conn);FlushVol(NULL,0);DisposeWindow(window);return 0;
}
