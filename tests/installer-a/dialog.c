/* Run the production dialog controller with deterministic Toolbox events.
 * Namespace/backend entry counters model all installation writes. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
typedef void *DialogPtr,*WindowPtr,*Handle,*ControlHandle;
typedef struct {short top,left,bottom,right;} Rect;
typedef struct {short what;unsigned long message;unsigned short modifiers;} EventRecord;
#define pascal
enum {keyDown=3,autoKey=5,charCodeMask=255,cmdKey=256,ControlKey=4096,everyEvent=65535,userCanceledErr=-128,fBsyErr=-47};
static unsigned long ticks;
static int create_fail,disposed,initialised,installed,launched,init_error,install_error,cancel_in_backend,event_index,event_count,poll_empty;
static EventRecord events[16];
static char labels[14][256];
static int hilites[14];
static void *GetNewDialog(int a,void*b,void*c){(void)a;(void)b;(void)c;return create_fail?0:(void*)1;}
static void GetDialogItem(DialogPtr d,short i,short*t,Handle*h,Rect*r){(void)d;*t=0;*h=(void*)(intptr_t)i;*r=(Rect){0,0,16,100};}
static void SetDialogItemText(Handle h,const unsigned char*s){int i=(int)(intptr_t)h;memcpy(labels[i],s+1,s[0]);labels[i][s[0]]=0;}
static void SetDialogItem(DialogPtr d,short i,short t,Handle h,Rect*r){(void)d;(void)i;(void)t;(void)h;(void)r;}
#define NewUserItemUPP(p) (p)
static void FrameRect(Rect*r){(void)r;}
static void InsetRect(Rect*r,int x,int y){r->left+=x;r->right-=x;r->top+=y;r->bottom-=y;}
static void PaintRect(Rect*r){assert(r->left<=r->right);}
static void EraseRect(Rect*r){assert(r->left<=r->right);}
static void SetPort(void*p){(void)p;}
static void HideDialogItem(DialogPtr d,int i){(void)d;(void)i;}
static void ShowDialogItem(DialogPtr d,int i){(void)d;(void)i;}
static void ShowWindow(void*p){(void)p;}
static void DrawDialog(void*p){(void)p;}
static void DisposeDialog(void*p){assert(p);disposed++;}
static void HiliteControl(void*p,int v){hilites[(int)(intptr_t)p]=v;}
static int GetControlHilite(void*p){return hilites[(int)(intptr_t)p];}
static void SetControlValue(void*p,int v){(void)p;(void)v;}
static void SetControlTitle(void*p,const unsigned char*s){(void)p;(void)s;}
static unsigned long TickCount(void){return ticks+=2;}
static int WaitNextEvent(int mask,EventRecord*e,int sleep,void*r){(void)mask;(void)r;if(!sleep&&poll_empty){poll_empty=0;return 0;}assert(event_index<event_count);*e=events[event_index++];poll_empty=!sleep;return 1;}
static int IsDialogEvent(EventRecord*e){return e->what==1;}
static int DialogSelect(EventRecord*e,DialogPtr*d,short*i){*d=(void*)1;*i=e->message;return 1;}
static char *LMGetApplLimit(void){return (char*)(uintptr_t)0x200000;}
static void SetApplLimit(void*p){(void)p;}
static void MaxApplZone(void){}
static void MoreMasters(void){}
static int fixed,uninstalled,removed_data,uninstall_error;
int shell_uninstall_locations(char*c,char*d){(void)c;(void)d;return uninstall_error;}
int shell_uninstall(const char*c,const char*d,int all){(void)c;(void)d;uninstalled++;removed_data=all;return uninstall_error;}
int shell_install_locations(char*c,char*d,int*f){strcpy(c,"Disk:Clarice");strcpy(d,"Disk:System:Clarice Data:");*f=fixed;return 0;}
static void LMSetSFSaveDisk(short v){(void)v;}
static void LMSetCurDirStore(long d){(void)d;}
int shell_install_folder(const char*p,short*v,long*d){(void)p;*v=-1;*d=2;return 0;}
int shell_install_choice(short wd,int data,char*out){(void)wd;strcpy(out,data?"Disk:Chosen:Clarice Data:":"Disk:Chosen:Clarice");return 0;}
int shell_install_init(const char*c,const char*d){(void)c;(void)d;initialised++;return init_error;}
int shell_install_offline(char*,int);
static int update_native_launch(const char*p){assert(!strcmp(p,"Disk:Clarice"));launched++;return 0;}
#include "../../src/installer-ui.inc"
int shell_install_offline(char*p,int repair){(void)repair;installed++;strcpy(p,"Disk:Clarice");app_install_progress("Checking Core",3,4);return cancel_in_backend&&installer_poll()?userCanceledErr:install_error;}
static void reset(void){
 fixed=uninstalled=removed_data=uninstall_error=0;installer_uninstall=installer_remove_data=installer_finished=0;
 installer_dialog=0;installer_running=installer_cancel=installer_choice=0;installer_step=installer_steps=installer_used=0;installer_last_poll=0;
 ticks=0;create_fail=disposed=initialised=installed=launched=init_error=install_error=cancel_in_backend=event_index=event_count=poll_empty=0;
 memset(labels,0,sizeof labels);memset(hilites,0,sizeof hilites);installer_fixed=installer_location_error=installer_repair=0;picker_good=0;
}
static void event(short w,unsigned long m,unsigned short mods){events[event_count++]=(EventRecord){w,m,mods};}
int main(void){
 for(int n=0;n<5;n++){reset();if(!n)event(1,2,0);else event(keyDown,n==1?27:n==2?'.':n==3?'q':'c',n==4?ControlKey:cmdKey);assert(!installer_main(0));assert(disposed==1&&!initialised&&!installed&&!launched);}
 reset();picker_good=1;event(1,7,0);event(1,8,0);event(1,2,0);assert(!installer_main(0));assert(!initialised&&!installed&&!launched);assert(!strcmp(installer_core,"Disk:Chosen:Clarice")&&!strcmp(installer_data,"Disk:Chosen:Clarice Data:"));
 reset();installer_dialog=(void*)1;strcpy(installer_core,"Other:Clarice");strcpy(installer_data,"Disk:Clarice Data:");installer_locations();assert(!installer_ready());installer_dialog=0;
 reset();create_fail=1;assert(installer_main(0)==1&&!initialised&&!disposed);
 /* Poll receives a harmless event then no further event in the same slice. */
 reset();event(1,1,0);event(keyDown,'.',cmdKey);event(1,2,0);assert(installer_main(0)==1&&!initialised&&!installed&&!launched&&disposed==1);
 reset();event(keyDown,13,0);event(0,0,0);assert(!installer_main(0)&&initialised==1&&installed==1&&launched==1&&disposed==1);
 reset();init_error=-108;event(1,1,0);event(0,0,0);event(1,2,0);assert(installer_main(0)==1&&initialised==1&&!installed&&!launched&&disposed==1);
 reset();install_error=-34;event(1,1,0);event(0,0,0);event(1,2,0);assert(installer_main(0)==1&&initialised==1&&installed==1&&!launched&&disposed==1);
 reset();cancel_in_backend=1;event(1,1,0);event(0,0,0);event(1,2,0);event(1,2,0);assert(installer_main(0)==1&&initialised==1&&installed==1&&!launched&&disposed==1);
 reset();installer_dialog=(void*)1;installer_running=1;app_install_progress("bounded",99,4);assert(installer_step==4);char line[1024];memset(line,'x',sizeof line);line[1023]='\n';installer_text(line,sizeof line);assert(strlen(labels[5])==255&&installer_used==0);
 for(int full=0;full<2;full++){
  reset();fixed=1;event(1,12,0);if(full)event(1,13,0);event(1,2,0);assert(!installer_main(0)&&!uninstalled&&!initialised&&!installed);
  reset();fixed=1;event(1,12,0);if(full)event(1,13,0);event(1,1,0);event(1,2,0);assert(!installer_main(0)&&uninstalled==1&&removed_data==full&&!initialised&&!installed&&!launched);
 }
 puts("PASS uninstall native confirmations: app-only default, full Data opt-in, Cancel at both stages, no install/launch side effects");
 puts("PASS native confirmation: five no-write cancellation forms; dialog allocation failure; cancellation before and during backend; one-shot success/handoff; initialization and disk-full errors; bounded progress/text");
 return 0;
}
