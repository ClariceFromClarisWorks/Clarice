#include <StandardFile.h>
/* Offline installer only: native confirmation precedes namespace creation.
 * Existing backend yields service this dialog, never the terminal event loop. */
static DialogPtr installer_dialog;
static int installer_running,installer_cancel,installer_choice;
static int installer_fixed,installer_location_error,installer_repair,installer_same_volume;
static char installer_core[256],installer_data[256];
extern int shell_install_locations(char *,char *,int *);
extern int shell_install_choice(short,int,char *);
extern int shell_install_folder(const char *,short *,long *);
extern int shell_install_init(const char *,const char *);
extern int shell_install_offline(char *,int);
static unsigned installer_step,installer_steps;
static unsigned long installer_last_poll;
static char installer_line[256];
static unsigned installer_used;
static void installer_label(short item,const char *text){
 short type;Handle handle;Rect box;unsigned char value[256];
 if(!installer_dialog)return;
 size_t n=strlen(text);if(n>255)n=255;value[0]=n;memcpy(value+1,text,n);
 GetDialogItem(installer_dialog,item,&type,&handle,&box);SetDialogItemText(handle,value);
}
static pascal void installer_bar(DialogPtr dialog,short item){
 short type;Handle handle;Rect box,fill;
 if(!installer_running)return;
 GetDialogItem(dialog,item,&type,&handle,&box);FrameRect(&box);InsetRect(&box,2,2);
 fill=box;fill.right=fill.left+(long)(box.right-box.left)*installer_step/(installer_steps?installer_steps:1);
 PaintRect(&fill);box.left=fill.right;EraseRect(&box);
}
void app_install_progress(const char *phase,unsigned step,unsigned total){
 if(step>total)step=total;installer_step=step;installer_steps=total;
 installer_label(5,phase);char text[80];
 snprintf(text,sizeof text,"Installation steps: %u of %u",step,total);installer_label(6,text);
 SetPort(installer_dialog);installer_bar(installer_dialog,4);
}
static void installer_text(const char *text,long length){
 while(length-->0){unsigned char c=*text++;
  if(c=='\n'||c=='\r'){if(installer_used){installer_line[installer_used]=0;installer_label(5,installer_line);installer_used=0;}}
  else if(c>=32&&installer_used<sizeof installer_line-1)installer_line[installer_used++]=c;
 }
}
static void installer_locations(void){
 installer_label(9,installer_core);installer_label(10,installer_data);
 const char *a=strchr(installer_core,':'),*b=strchr(installer_data,':');
 int same=a&&b&&a-installer_core==b-installer_data&&!memcmp(installer_core,installer_data,a-installer_core);
 installer_same_volume=same;
 short type;Handle handle;Rect box;GetDialogItem(installer_dialog,1,&type,&handle,&box);HiliteControl((ControlHandle)handle,installer_location_error||!same?255:0);
 if(!same)installer_label(5,"Choose Core and Clarice Data on the same disk.");
 else installer_label(5,installer_fixed?"Existing locations retained; user data will not move.":"Defaults are recommended; choose Install to confirm your choices.");
}
static void installer_choose(int data){
 if(installer_running||installer_fixed||installer_location_error)return;
 SFReply reply;Point point={40,30};short volume;long directory;
 int e=shell_install_folder(data?installer_data:installer_core,&volume,&directory);
 if(e){installer_label(5,"Cannot open the selected folder. No files changed.");return;}
 LMSetSFSaveDisk(-volume);LMSetCurDirStore(directory);
 /* Original Standard File is available on System 6. Its reply contains a
    working-directory reference; selection itself never creates a file. */
 SFPutFile(point,(const unsigned char*)(data?"\pChoose folder for Clarice Data":"\pChoose folder for Clarice"),(const unsigned char*)(data?"\pClarice Data":"\pClarice"),0,&reply);
 SetPort(installer_dialog);DrawDialog(installer_dialog);
 if(reply.good){char path[256];int e=shell_install_choice(reply.vRefNum,data,path);
  if(e){installer_label(5,"Cannot use that folder. Please choose another.");return;}
  strcpy(data?installer_data:installer_core,path);
 }
 installer_locations();
}
static int installer_ready(void){return !installer_location_error&&installer_same_volume;}
static void installer_event(EventRecord *event){
 if(event->what==keyDown||event->what==autoKey){unsigned char c=event->message&charCodeMask;
  if(c==27||((event->modifiers&cmdKey)&&(c=='.'||c=='q'))||((event->modifiers&ControlKey)&&(c==3||c=='c'))){installer_cancel=1;installer_choice=2;return;}
  if(!installer_running&&installer_ready()&&(c==13||c==3)){installer_choice=1;return;}
 }
 DialogPtr dialog;short item;
 if(IsDialogEvent(event)&&DialogSelect(event,&dialog,&item)&&dialog==installer_dialog){
  if(item==2){installer_cancel=1;installer_choice=2;}
  else if(item==1&&!installer_running&&installer_ready())installer_choice=1;
  else if(item==7||item==8)installer_choose(item==8);
  else if(item==11&&!installer_running&&installer_fixed){short type;Handle handle;Rect box;installer_repair=!installer_repair;GetDialogItem(installer_dialog,11,&type,&handle,&box);SetControlValue((ControlHandle)handle,installer_repair);}
 }
}
static int installer_poll(void){
 unsigned long now=TickCount();if(now-installer_last_poll<2)return installer_cancel;
 installer_last_poll=now;EventRecord event;
 for(int n=0;n<8;n++){if(!WaitNextEvent(everyEvent,&event,0,0))break;installer_event(&event);}
 return installer_cancel;
}
static int installer_main(unsigned long launch){
 (void)launch;installer_dialog=GetNewDialog(152,0,(WindowPtr)-1);if(!installer_dialog)return 1;
 SetPort(installer_dialog);short type;Handle handle;Rect box;
 GetDialogItem(installer_dialog,4,&type,&handle,&box);SetDialogItem(installer_dialog,4,type,(Handle)NewUserItemUPP(installer_bar),&box);
 ShowWindow(installer_dialog);DrawDialog(installer_dialog);
 installer_location_error=shell_install_locations(installer_core,installer_data,&installer_fixed);
 installer_locations();
 for(int item=7;item<=8;item++){GetDialogItem(installer_dialog,item,&type,&handle,&box);HiliteControl((ControlHandle)handle,installer_fixed||installer_location_error?255:0);}
 GetDialogItem(installer_dialog,11,&type,&handle,&box);HiliteControl((ControlHandle)handle,installer_fixed?0:255);
 if(installer_location_error)installer_label(5,"Cannot read installation locations. Cancel and check Clarice Locations in the System Folder.");
#ifdef CLARICE_INSTALL_DIAGNOSTICS
 {char text[100];snprintf(text,sizeof text,"Ready %lu ticks; free %ld; largest %ld",TickCount()-launch,FreeMem(),MaxBlock());installer_label(6,text);}
#endif
 EventRecord event;while(!installer_choice){if(WaitNextEvent(everyEvent,&event,6,0))installer_event(&event);}
 if(installer_choice==2){DisposeDialog(installer_dialog);installer_dialog=0;return 0;}
 installer_running=1;GetDialogItem(installer_dialog,1,&type,&handle,&box);HiliteControl((ControlHandle)handle,255);
 installer_label(3,"Installing Clarice. Please do not reset the Macintosh.\rCancel keeps completed tools for retry.");
 for(int item=7;item<=11;item++){if(item==9||item==10)continue;GetDialogItem(installer_dialog,item,&type,&handle,&box);HiliteControl((ControlHandle)handle,255);}
 app_install_progress("Preparing Clarice Data",0,1);
 /* Keep the existing working-stack allowance, but only after confirmation. */
 SetApplLimit(LMGetApplLimit()-65536);MaxApplZone();MoreMasters();

 char target[256];int e=installer_poll()?userCanceledErr:shell_install_init(installer_core,installer_data);
 if(!e)e=shell_install_offline(target,installer_repair);
 if(!e)app_install_progress("Installation complete. Opening Clarice...",installer_steps,installer_steps);
 if(!e)e=update_native_launch(target);
 if(e){char message[220];
  if(e==130||e==userCanceledErr)strcpy(message,"Installation cancelled.\rCompleted tools are kept. Run the installer again to resume.");
  else if(e==fBsyErr)strcpy(message,"Quit Clarice, then run the installer again.\rThe running application was not replaced.");
  else snprintf(message,sizeof message,"Installation stopped (Mac OS %d). Recovery files kept.\rCorrect the error below, then rerun the installer.",e);
  installer_label(3,message);installer_label(6,"");
  installer_running=0;GetDialogItem(installer_dialog,2,&type,&handle,&box);SetControlTitle((ControlHandle)handle,(const unsigned char*)"\pClose");
  installer_choice=0;while(!installer_choice){if(WaitNextEvent(everyEvent,&event,6,0))installer_event(&event);}
 }
 DisposeDialog(installer_dialog);installer_dialog=0;return e?1:0;
}
