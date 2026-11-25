/* Serial-1 native driver operations, with service-owned asynchronous storage. */
static void queue_bytes(const char *p,unsigned long n){if(n>QUEUE-used){failed=1;return;}memcpy(output+used,p,n);used+=n;}
static void serial_text(const char *p,unsigned long n){while(n--){if(*p=='\n')queue_bytes("\r",1);queue_bytes(p++,1);}}
#define telnet_send(t,p,n) queue_bytes(p,n)
#define telnet_send_text(t,p,n) serial_text(p,n)
#define clarice_telnet_enabled(t,o,d) 1
#define telnet_free(t) ((void)0)
#define TELNET_TELOPT_ECHO 1
static unsigned long NetParseIP(const char *p){return !strcmp(p,"modem")||!strcmp(p,"/dev/tty.modem")?1:!strcmp(p,"printer")||!strcmp(p,"/dev/tty.printer")?2:0;}
static int NetInit(void){return 0;}
static const char *NetErrStr(int e){(void)e;return "native serial driver error";}
static int release_stream(void){int e;
 if(!driver)return 0;
 if(driver->out&&send_pending){e=KillIO(driver->out);if(e)return e;if(((volatile ParamBlockRec*)&driver->sending)->ioParam.ioResult>0)return -1;send_pending=0;}
 if(driver->in&&driver->buffer_set){e=SerSetBuf(driver->in,0,0);if(e)return e;driver->buffer_set=0;}
 if(driver->in){e=CloseDriver(driver->in);if(e)return e;driver->in=0;}
 if(driver->out){e=CloseDriver(driver->out);if(e)return e;driver->out=0;}
 connected=opening_pending=send_pending=0;app_remote_indicator(1,0);return 0;
}
static int listen_again(void){int e;SerShk h;
 e=OpenDriver((const unsigned char*)(peer==2?"\004.BIn":"\004.AIn"),&driver->in);if(e){driver->in=0;return e;}
 e=OpenDriver((const unsigned char*)(peer==2?"\005.BOut":"\005.AOut"),&driver->out);if(e){driver->out=0;return e;}
 e=SerReset(driver->out,baud57600|data8|noParity|stop10);if(e)return e;
 memset(&h,0,sizeof h);h.xOn=17;h.xOff=19;e=SerHShake(driver->in,&h);if(e)return e;
 e=SerSetBuf(driver->in,(Ptr)driver->ring,sizeof driver->ring);if(e)return e;driver->buffer_set=1;return 0;
}
static void pump(void){
 if(!driver||failed)return;unsigned long now=TickCount();if(now==last_pump)return;last_pump=now;
 if(connected&&protected_login&&!session&&now-auth_started>3600){wipe(password,sizeof password);failed=1;return;}
 send_prompt();
 if(send_pending){int e=((volatile ParamBlockRec*)&driver->sending)->ioParam.ioResult;if(e>0){if(now-send_tick>300)failed=1;}else{send_pending=0;if(e||driver->sending.ioParam.ioActCount!=driver->sending.ioParam.ioReqCount)failed=1;}}
 if(!send_pending&&used&&!failed){memcpy(driver->send,output,used);memset(&driver->sending,0,sizeof driver->sending);driver->sending.ioParam.ioRefNum=driver->out;driver->sending.ioParam.ioBuffer=(Ptr)driver->send;driver->sending.ioParam.ioReqCount=used;used=0;
  if(PBWriteAsync(&driver->sending)<0)failed=1;else{send_pending=1;send_tick=now;}}
 if(closing&&!used&&!send_pending){failed=1;return;}
 receive_pending();if(received_count||used)return;
 SerStaRec status;long count=0;if(SerStatus(driver->in,&status)||status.cumErrs||SerGetBuf(driver->in,&count)){failed=1;return;}
 if(count){if(count>sizeof received)count=sizeof received;if(FSRead(driver->in,&count,received)){failed=1;return;}
  if(!connected){/* Return wakes a serial login; cable removal has no reliable carrier indication. */
   for(long i=0;i<count;i++)if(received[i]=='\r'||received[i]=='\n'){
    if(auth_retry&&(long)(now-auth_retry)<0)return;
    connected=1;after_cr=received[i]=='\r';engine=(void*)1;auth_started=now;app_remote_indicator(1,1);text("Clarice serial shell (unencrypted; one session)\nPassword: ");break;
   }return;
  }
  received_count=count;receive_pending();
 }
}
