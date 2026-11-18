/* Bounded transport adapter. Native setup/cleanup follows Serial-1. */
#ifdef KERMIT_HOST
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <termios.h>
#include <signal.h>
#include <time.h>
static unsigned long host_pace_ns;
static int wire=-1;static struct termios saved;static int saved_ok;
static volatile sig_atomic_t cancelled;
static void interrupt(int sig){(void)sig;cancelled=1;}
static unsigned long ticks(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec*60UL+t.tv_nsec/16666667;}
static int yield(void){return cancelled!=0;}
static int transport_open(const char *path){
 struct termios t;wire=open(path,O_RDWR|O_NOCTTY|O_NONBLOCK);if(wire<0)return -1;
 if(tcgetattr(wire,&saved))return -1;saved_ok=1;t=saved;cfmakeraw(&t);t.c_cflag|=CLOCAL|CREAD;t.c_cflag&=~(PARENB|CSTOPB|CSIZE);t.c_cflag|=CS8;
 t.c_iflag&=~(IXON|IXOFF|IXANY);
#ifdef CRTSCTS
 t.c_cflag&=~CRTSCTS;
#endif
 cfsetspeed(&t,B57600);if(tcsetattr(wire,TCSANOW,&t))return -1;
 signal(SIGINT,interrupt);signal(SIGTERM,interrupt);return 0;
}
static int wire_read(unsigned char *buf,int n){int r=read(wire,buf,n);if(r<0&&(errno==EAGAIN||errno==EINTR)){struct timeval t={0,1000};select(0,0,0,0,&t);return 0;}return r<0?-1:r;}
static int wire_write(const unsigned char *p,int n){unsigned long start=ticks();while(n){if(yield()||ticks()-start>300)return -1;int w=write(wire,p,host_pace_ns?1:n);if(w<0&&(errno==EAGAIN||errno==EINTR))continue;if(w<=0)return -1;p+=w;n-=w;if(host_pace_ns){struct timespec pause={0,(long)host_pace_ns};nanosleep(&pause,0);}}return 0;}
static int transport_close(void){int e=0;if(wire>=0){/* Give the PTY bridge time to consume the final ACK before closing. */
 struct timespec grace={0,250000000};if(tcdrain(wire))e=-1;nanosleep(&grace,0);
 if(saved_ok&&tcsetattr(wire,TCSANOW,&saved))e=-1;if(close(wire))e=-1;wire=-1;}return e;}
#else
#include "../../commands/mac-files.h"
#include <Serial.h>
#include <Devices.h>
extern long cli_remote_wire(int,void *,unsigned long);
static int remote_wire,remote_transfer_failed;
static short serial_in,serial_out;static char receive_buffer[4096];static int buffer_set;
static unsigned long ticks(void){return cli_ticks();}
static int cancelled;
static int yield(void){if(app_yield())cancelled=1;if(!remote_wire)while(app_serial_key()>=0){}return cancelled;}
static int transport_open(const char *path){
 if(cli_remote_wire(0,0,0)>0){if(cli_remote_wire(1,0,0))return -1;remote_wire=1;return 0;}
 int port=!strcmp(path,"/dev/tty.printer"),e;SerShk h;
 if(strcmp(path,"/dev/tty.modem")&&strcmp(path,"/dev/tty.printer"))return paramErr;
 e=OpenDriver((const unsigned char *)(port?"\004.BIn":"\004.AIn"),&serial_in);if(e){serial_in=0;return e;}
 e=OpenDriver((const unsigned char *)(port?"\005.BOut":"\005.AOut"),&serial_out);if(e){serial_out=0;return e;}
 e=SerReset(serial_out,baud57600|data8|noParity|stop10);if(e)return e;
 memset(&h,0,sizeof h);h.xOn=17;h.xOff=19;e=SerHShake(serial_in,&h);if(e)return e;
 e=SerSetBuf(serial_in,receive_buffer,sizeof receive_buffer);if(e)return e;buffer_set=1;app_serial_input(1);return 0;
}
static int wire_read(unsigned char *buf,int cap){if(remote_wire)return cli_remote_wire(2,buf,cap);long n=0;SerStaRec s;int e=SerStatus(serial_in,&s);if(e||s.cumErrs)return -1;e=SerGetBuf(serial_in,&n);if(e)return -1;if(n>cap)n=cap;if(!n)return 0;e=FSRead(serial_in,&n,buf);return e?-1:(int)n;}
static int wire_write(const unsigned char *p,int n){
 if(remote_wire)return cli_remote_wire(3,(void*)p,n)==n?0:-1;
 ParamBlockRec pb;unsigned long start=ticks();int e;memset(&pb,0,sizeof pb);
 pb.ioParam.ioRefNum=serial_out;pb.ioParam.ioBuffer=(char *)p;pb.ioParam.ioReqCount=n;
 e=PBWriteAsync(&pb);if(e<0)return -1;
 while(((volatile ParamBlockRec *)&pb)->ioParam.ioResult>0){if(yield()||ticks()-start>=300){KillIO(serial_out);return -1;}}
 return pb.ioParam.ioResult||pb.ioParam.ioActCount!=n?-1:0;
}
static int transport_close(void){if(remote_wire){int e=cli_remote_wire(4,0,remote_transfer_failed);remote_wire=0;return e;}int e=0,c;app_serial_input(0);if(serial_in){if(buffer_set)e=SerSetBuf(serial_in,0,0);c=CloseDriver(serial_in);if(!e)e=c;serial_in=0;}if(serial_out){c=CloseDriver(serial_out);if(!e)e=c;serial_out=0;}return e;}
#endif
