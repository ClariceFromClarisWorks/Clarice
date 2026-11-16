/* Native Serial Driver text client. No SCC register access or terminal emulator. */
#ifndef SERIAL_HOST_TEST
#include "../sys6.h"
#include <Serial.h>
#include <Devices.h>
#include <Files.h>
#include <Errors.h>
#endif
#include <string.h>
#include <stdio.h>
extern unsigned long cli_ticks(void);
extern int cli_isatty(int);
extern int app_serial_tty(int);
extern int app_serial_key(void);
extern long cli_write(int,const void *,unsigned long);
/* CLX BSS belongs to this invocation, not the idle terminal. */
static char input_buffer[4096];
static int output(const char *s){return cli_write(1,s,strlen(s))<0;}
static int failure(const char *what,int error){char b[128];snprintf(b,sizeof b,"serial: %s (Mac error %d)\n",what,error);cli_write(2,b,strlen(b));return 1;}
static const char usage[]="usage: serial [-b 4800|9600|19200|38400|57600]\n/dev/tty.modem|/dev/tty.printer [--test]\n";
int clarice_main(int argc,char **argv){
 int probe=0,baud=baud57600,port=-1,e=0,result=1,buffer_set=0,escape=0,leaving=0,pending=0;
 short in=0,out=0;unsigned long start=0,sent_at=0;unsigned matched=0;
 char bytes[256],tx[64];ParamBlockRec write_pb;const char *reason="driver I/O";
 static const char reply[]="CLARICE-PONG";
 memset(&write_pb,0,sizeof write_pb);
 if(argc==2&&!strcmp(argv[1],"--help")){
  output(usage);output("Default 57600 baud, 8N1, no hardware/software flow control; remote echo.\nCtrl+] then q exits locally; Ctrl+] twice sends Ctrl+]. Ctrl+C\ninterrupts remote; Cmd-period cancels locally.\n--test sends CLARICE-PING and waits 15 seconds for CLARICE-PONG.\nmodem/printer aliases and legacy positional baud are accepted.\nText only: no VT100, Telnet, SSH or file transfer. No carrier detection:\nan unplugged three-wire cable may remain silent; exit locally and\nreconnect.\nPrinter port must be free of AppleTalk/other applications.\n");return 0;
 }
 for(int i=1;i<argc;i++){
  const char *arg=argv[i];
  if(!strcmp(arg,"--test")){probe=1;continue;}
  if(!strcmp(arg,"/dev/tty.modem")||!strcmp(arg,"modem")){if(port!=-1)goto usage_error;port=0;continue;}
  if(!strcmp(arg,"/dev/tty.printer")||!strcmp(arg,"printer")){if(port!=-1)goto usage_error;port=1;continue;}
  if(!strcmp(arg,"-b")){if(++i==argc)goto usage_error;arg=argv[i];}
  if(!strcmp(arg,"4800"))baud=baud4800;else if(!strcmp(arg,"9600"))baud=baud9600;
  else if(!strcmp(arg,"19200"))baud=baud19200;else if(!strcmp(arg,"38400"))baud=baud38400;
  else if(!strcmp(arg,"57600"))baud=baud57600;else goto usage_error;
 }
 if(port<0)goto usage_error;
 if(!probe&&(!cli_isatty(0)||!cli_isatty(1))){output("serial: interactive session requires terminal input and output\n");return 2;}
 e=OpenDriver((const unsigned char*)(port?"\004.BIn":"\004.AIn"),&in);if(e){in=0;goto end;}
 e=OpenDriver((const unsigned char*)(port?"\005.BOut":"\005.AOut"),&out);if(e){out=0;goto end;}
 e=SerReset(out,baud|data8|noParity|stop10);if(e)goto end;
 {SerShk handshake;memset(&handshake,0,sizeof handshake);handshake.xOn=17;handshake.xOff=19;e=SerHShake(in,&handshake);if(e)goto end;}
 e=SerSetBuf(in,input_buffer,sizeof input_buffer);if(e)goto end;buffer_set=1;
 app_activity_interactive(1);start=cli_ticks();
 if(!probe){app_serial_tty(1);output("Serial session; remote echo. Ctrl+] then q exits; Ctrl+C interrupts remote; Cmd-period cancels locally.\n");}
 else {memcpy(tx,"CLARICE-PING\r",13);write_pb.ioParam.ioReqCount=13;}
 while(!app_yield()){
  long count=0;SerStaRec status;
  e=SerStatus(in,&status);if(e)goto end;
  if(status.cumErrs){reason="receive overrun/framing/parity error; data lost";e=ioErr;goto end;}
  /* Only request bytes already buffered: never block the UI waiting for input. */
  e=SerGetBuf(in,&count);if(e)goto end;
  if(count>0){if(count>(long)sizeof bytes)count=sizeof bytes;e=FSRead(in,&count,bytes);if(e)goto end;
   if(count<=0){e=ioErr;goto end;}
   if(probe){for(long i=0;i<count;i++){if(bytes[i]==reply[matched])matched++;else matched=bytes[i]==reply[0]?1:0;if(matched==sizeof reply-1){result=0;output("SERIAL_TEST PASS: host replied CLARICE-PONG\n");goto end;}}}
   else if(cli_write(1,bytes,count)!=count){e=ioErr;goto end;}
  }
  if(pending){
   /* Device Manager updates ioResult from interrupt context. */
   e=((volatile ParamBlockRec *)&write_pb)->ioParam.ioResult;
   if(e>0){e=0;if(cli_ticks()-sent_at>=300){reason="transmit timeout";e=ioErr;goto end;}continue;}
   pending=0;if(e)goto end;
   if(write_pb.ioParam.ioActCount!=write_pb.ioParam.ioReqCount){reason="short serial write";e=ioErr;goto end;}
   write_pb.ioParam.ioReqCount=0;
  }
  if(leaving){result=0;goto end;}
  if(probe&&cli_ticks()-start>=900){reason="probe timeout (15 seconds)";e=ioErr;goto end;}
  if(!probe){
   int c;count=0;
   while(count<(long)sizeof tx&&(c=app_serial_key())!=-1){
    if(c==-2){reason="keyboard queue overflow; session stopped";e=ioErr;goto end;}
    if(escape){escape=0;if(c=='q'){leaving=1;break;}if(c!=29)tx[count++]=29;if(count==(long)sizeof tx){/* room is reserved below */e=ioErr;goto end;}}
    else if(c==29){escape=1;continue;}
    tx[count++]=(char)(c==8?127:c);
    if(count>=(long)sizeof tx-1)break;
   }
   write_pb.ioParam.ioReqCount=count;
  }
  if(write_pb.ioParam.ioReqCount){
   write_pb.ioParam.ioRefNum=out;write_pb.ioParam.ioBuffer=tx;write_pb.ioParam.ioActCount=0;
   e=PBWriteAsync(&write_pb);if(e<0)goto end;e=0;pending=1;sent_at=cli_ticks();
  }
 }
 result=130;
end:
 app_serial_tty(0);app_activity_interactive(0);
 /* Cancel pending DMA/interrupt access before stack/image storage is released. */
 if(out&&pending&&((volatile ParamBlockRec *)&write_pb)->ioParam.ioResult>0){int cleanup=KillIO(out);if(!e&&cleanup){e=cleanup;reason="cancel output";}}
 /* Restore the receive buffer before closing either half of the shared port. */
 if(in){int cleanup;if(buffer_set){cleanup=SerSetBuf(in,0,0);if(!e&&cleanup){e=cleanup;reason="restore receive buffer";}}cleanup=CloseDriver(in);if(!e&&cleanup){e=cleanup;reason="close input";}}
 if(out){int cleanup=CloseDriver(out);if(!e&&cleanup){e=cleanup;reason="close output";}}
 if(e==userCanceledErr)return 130;if(e)return failure(reason,e);return result;
usage_error:
 cli_write(2,usage,strlen(usage));return 2;
}
