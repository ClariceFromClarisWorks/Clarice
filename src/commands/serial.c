/* Initial modem-port text client; Serial Driver owns the SCC hardware. */
#include "../sys6.h"
#include <Serial.h>
#include <Devices.h>
#include <Files.h>
#include <Errors.h>
#include <string.h>
#include <stdio.h>
extern unsigned long cli_ticks(void);
extern int cli_isatty(int);
extern int app_serial_input(int);
extern int app_serial_key(void);
extern long cli_write(int,const void *,unsigned long);
static char input_buffer[1024];
static int output(const char *s){return cli_write(1,s,strlen(s))<0;}
static int failure(const char *what,int error){char b[100];snprintf(b,sizeof b,"serial: %s (Mac error %d)\n",what,error);cli_write(2,b,strlen(b));return 1;}
static int send_bytes(short ref,const char *p,long n){while(n){long count=n>32?32:n;int e=FSWrite(ref,&count,p);if(e)return e;if(count<=0)return ioErr;p+=count;n-=count;if(app_yield())return userCanceledErr;}return 0;}
int clarice_main(int argc,char **argv){
 int probe=0,baud=baud9600;short in=0,out=0;int e=0,result=1,buffer_set=0;unsigned long start;char bytes[128];
 if(argc==2&&!strcmp(argv[1],"--help")){output("usage: serial modem [4800|9600|19200] [--test]\nModem port A, 8 data bits, no parity, 1 stop bit; no flow control.\n--test sends CLARICE-PING and waits up to 15 seconds for CLARICE-PONG.\nText session: remote echo; Ctrl+C or Cmd-period disconnects.\nInitial driver access only: no ANSI/VT, Telnet, SSH or file transfer.\n");return 0;}
 if(argc<2||strcmp(argv[1],"modem")){output("usage: serial modem [4800|9600|19200] [--test]\nUse serial --help for limits.\n");return 2;}
 for(int i=2;i<argc;i++){if(!strcmp(argv[i],"--test"))probe=1;else if(!strcmp(argv[i],"4800"))baud=baud4800;else if(!strcmp(argv[i],"9600"))baud=baud9600;else if(!strcmp(argv[i],"19200"))baud=baud19200;else return 2;}
 if(!probe&&(!cli_isatty(0)||!cli_isatty(1))){output("serial: interactive session requires terminal input and output\n");return 2;}
 e=OpenDriver((const unsigned char*)"\p.AIn",&in);if(e){in=0;goto end;}
 e=OpenDriver((const unsigned char*)"\p.AOut",&out);if(e){out=0;goto end;}
 e=SerReset(out,baud|data8|noParity|stop10);if(e)goto end;
 {SerShk handshake;memset(&handshake,0,sizeof handshake);handshake.xOn=17;handshake.xOff=19;e=SerHShake(in,&handshake);if(e)goto end;}
 e=SerSetBuf(in,input_buffer,sizeof input_buffer);if(e)goto end;buffer_set=1;
 if(probe){e=send_bytes(out,"CLARICE-PING\r",13);if(e)goto end;}
 else {output("Serial modem session; remote echo. Ctrl+C or Cmd-period disconnects.\n");app_serial_input(1);}
 app_activity_interactive(1);start=cli_ticks();unsigned matched=0;static const char reply[]="CLARICE-PONG";
 while(!app_yield()){
  long count=0;e=SerGetBuf(in,&count);if(e)goto end;
  if(count>0){if(count>(long)sizeof bytes)count=sizeof bytes;e=FSRead(in,&count,bytes);if(e)goto end;
   if(probe){for(long i=0;i<count;i++){if(bytes[i]==reply[matched])matched++;else matched=bytes[i]==reply[0]?1:0;if(matched==sizeof reply-1){result=0;output("SERIAL_TEST PASS: host replied CLARICE-PONG\n");goto end;}}}
   else if(cli_write(1,bytes,count)<0){e=ioErr;goto end;}
  }
  if(probe&&cli_ticks()-start>=900){output("SERIAL_TEST timeout: no host reply within 15 seconds\n");goto end;}
  if(!probe){int c;while((c=app_serial_key())>=0){char ch=c;e=send_bytes(out,&ch,1);if(e)goto end;}}
 }
 result=130;
end:
 app_serial_input(0);app_activity_interactive(0);
 if(in){KillIO(in);if(buffer_set){int cleanup=SerSetBuf(in,0,0);if(!e&&cleanup)e=cleanup;}CloseDriver(in);}
 if(out){KillIO(out);CloseDriver(out);}
 if(e==userCanceledErr)return 130;if(e)return failure("driver I/O",e);return result;
}
