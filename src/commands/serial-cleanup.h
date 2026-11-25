/* Foreground Serial Driver ownership. Never leave a stack PB or unload a CLX
 * receive buffer while the driver can still use it. A broken driver may keep
 * cleanup waiting; cancellation is not permission to free live I/O storage.
 * No callbacks, retained allocations or idle service are needed. */
static void serial_cleanup_error(const char *operation,int error){
 char text[112];int n=snprintf(text,sizeof text,"Serial %s failed (Mac error %d); waiting for safe cleanup.\n",operation,error);
 cli_write(2,text,n);
}
static void serial_cleanup_wait(void){
 long ignored;
 app_yield();SystemTask();Delay(1,&ignored);
}
static int serial_drain(short ref,ParamBlockRec *pb){
 int first=0;
 while(((volatile ParamBlockRec*)pb)->ioParam.ioResult>0){
  int e=KillIO(ref);if(e&&!first){first=e;serial_cleanup_error("cancel",e);}
  if(((volatile ParamBlockRec*)pb)->ioParam.ioResult>0)serial_cleanup_wait();
 }
 return first;
}
static int serial_detach(short ref,int *attached){
 int first=0;
 while(*attached){
  int e=SerSetBuf(ref,0,0);
  if(!e)*attached=0;else{if(!first){first=e;serial_cleanup_error("detach",e);}serial_cleanup_wait();}
 }
 return first;
}
static int serial_close_driver(short *ref){
 int first=0;
 while(*ref){
  int e=CloseDriver(*ref);
  if(!e)*ref=0;else{if(!first){first=e;serial_cleanup_error("close",e);}serial_cleanup_wait();}
 }
 return first;
}
