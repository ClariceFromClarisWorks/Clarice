/* Clarice's small, binary-only E-Kermit adapter. No packet bytes reach term_write. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "../ports/ekermit/transport.h"
#ifdef X_OK
#undef X_OK
#endif
#include "../ports/ekermit/upstream/cdefs.h"
#include "../ports/ekermit/upstream/kermit.h"
#include "../ports/ekermit/files.h"
#include "../help-output.h"
static void message(int fd,const char *format,...){
#ifndef KERMIT_HOST
 if(remote_wire)return; /* Never mix diagnostics with protocol bytes. */
#endif
 char text[1024];va_list ap;va_start(ap,format);int n=vsnprintf(text,sizeof text,format,ap);va_end(ap);
 if(n<0)return;if(n>=(int)sizeof text)n=sizeof text-1;
#ifdef KERMIT_HOST
 (void)write(fd,text,n);
#else
 extern long cli_write(int,const void *,unsigned long);cli_write(fd,text,n);
#endif
}
#define progress_clock ticks
#ifdef KERMIT_HOST
#include <sys/ioctl.h>
#define progress_tty() isatty(2)
static int progress_columns(void){struct winsize w;return !ioctl(2,TIOCGWINSZ,&w)&&w.ws_col?w.ws_col:80;}
static void progress_row(const char *s,int done){message(2,"\r\033[K%s%s",s,done?"\n":"");}
#else
extern int cli_isatty(int),app_columns(void);
#define progress_tty() (!remote_wire&&cli_isatty(2))
#define progress_row app_progress
#define progress_columns app_columns
#endif
#include "progress.h"
static unsigned long transfer_size,transfer_bytes;
static int transfer_size_known;
static struct k_data protocol;static struct k_response response;
static unsigned char in_file[512],out_file[512],rx[256];static int rxpos,rxlen,io_failed;
static int sending;
static int transmit(struct k_data *k,UCHAR *p,int n){(void)k;if(io_failed||wire_write(p,n)){io_failed=1;return X_ERROR;}return X_OK;}
static int pending(struct k_data *k){(void)k;return -1;}
static int valid_name(const UCHAR *p){int n=0;if(!p||!*p||!strcmp((const char *)p,".")||!strcmp((const char *)p,".."))return 0;while(*p){if(++n>=FN_MAX||*p<32||*p>=127||*p=='/'||*p=='\\'||*p==':')return 0;p++;}return 1;}
static int open_file(struct k_data *k,UCHAR *name,int mode){(void)k;if(opened||!valid_name(name)||mode!=(sending?1:2)||file_open(sending)){io_failed=1;return X_ERROR;}opened=1;
#ifdef KERMIT_HOST
 if(sending){struct stat st;if(!fstat(file_fd,&st)&&st.st_size>=0&&(unsigned long long)st.st_size<=0x7fffffffUL){transfer_size=st.st_size;transfer_size_known=1;}}
#else
 if(sending){CInfoPBRec pb;if(!info(native_path,&pb)){transfer_size=pb.hFileInfo.ioFlLgLen;transfer_size_known=1;}}
#endif
 return X_OK;}
static int read_file(struct k_data *k){int n=file_read(k->zinbuf,k->zinlen);if(n<0){io_failed=1;return -1;}if(!n)return -1;k->zinptr=k->zinbuf+1;k->zincnt=n-1;return k->zinbuf[0];}
static int write_file(struct k_data *k,UCHAR *p,int n){(void)k;if(file_write(p,n)){io_failed=1;return X_ERROR;}return X_OK;}
static int close_file(struct k_data *k,UCHAR c,int mode){(void)k;(void)mode;
 if(c=='D'||io_failed||!opened){io_failed=1;return X_ERROR;}
 if(sending){if(file_close()){io_failed=1;return X_ERROR;}}
 else if(file_finish(1)){io_failed=1;return X_ERROR;}
 completed=1;return X_OK;
}
static int read_packet(struct k_data *k,UCHAR *p,int cap){unsigned long start=ticks();int n=0,active=0;
 while(ticks()-start<120){if(yield()){cancelled=1;return -1;}
  if(rxpos==rxlen){rxlen=wire_read(rx,sizeof rx);rxpos=0;if(rxlen<0){io_failed=1;return -1;}if(!rxlen)continue;}
  unsigned char c=rx[rxpos++];if(c==k->r_soh){n=0;active=1;continue;}if(!active)continue;
  if(c==k->r_eom||c==10){p[n]=0;return n;}
  if(n>=cap){active=0;n=0;continue;}p[n++]=c;
 }return 0;
}
static const char help[]=
  "Usage:\n"
  "  kermit [options] send FILE\n"
  "  kermit [options] receive NEWFILE\n"
  "Transfer one binary data fork using E-Kermit 1.8, CRC-16 and short packets. Use MacBinary for complete Macintosh files.\n"
  "Options:\n"
  "  -d DEVICE\t/dev/tty.modem (default) or /dev/tty.printer.\n"
  "  -b 57600\t57600 baud, 8N1, no flow control.\n"
  "Receive refuses existing files. Exit serial before a local transfer.\n"
  "Remote transfers use the current session. Wait for Kermit ready. Telnet requires binary negotiation.\n"
  "Ctrl+C or Command-period cancels. Reconnect serial for text afterward.\n"
  "E-Kermit: Frank da Cruz / Columbia University, 3-clause BSD.\n"
#ifdef KERMIT_HOST
  "  --pace-ms 0..20\tDelay each transmitted byte, including ACKs. Use 2 for the tested physical SE/USB cable without flow control.\n"
#endif
 ;
#ifdef KERMIT_HOST
int main(int argc,char **argv){
#else
int clarice_main(int argc,char **argv){
#endif
 const char *device="/dev/tty.modem",*path=0;int i=1,status=X_ERROR,result=1,e,timeouts=0;unsigned long began=ticks();UCHAR *names[2]={(UCHAR *)"data.bin",0};
 if(argc==2&&!strcmp(argv[1],"--help"))return help_output(1,help);
 while(i<argc&&argv[i][0]=='-'){if(i+1==argc)goto usage;
  #ifdef KERMIT_HOST
  if(!strcmp(argv[i],"--pace-ms")){char *end;long ms=strtol(argv[++i],&end,10);if(*end||ms<0||ms>20)goto usage;host_pace_ns=(unsigned long)ms*1000000;}
  else
#endif
  if(!strcmp(argv[i],"-d"))device=argv[++i];else if(!strcmp(argv[i],"-b")&&!strcmp(argv[i+1],"57600"))++i;else goto usage;i++;}
 if(i+2!=argc)goto usage;
 if(!strcmp(argv[i],"send"))sending=1;else if(strcmp(argv[i],"receive"))goto usage;path=argv[i+1];
 /* Sender filename is advisory; receiver always requires an explicit NEWFILE. */
 if((e=file_prepare(path,sending))){message(2,"kermit: cannot prepare file (%d); receive requires a new destination\n",e);goto end;}
 if((e=transport_open(device))){message(2,"kermit: cannot open serial device (%d)\n",e);goto end;}
 memset(&protocol,0,sizeof protocol);memset(&response,0,sizeof response);
 protocol.binary=1;protocol.xfermode=1;protocol.bct=3;protocol.bctf=1;
 protocol.zinbuf=in_file;protocol.zinlen=sizeof in_file;protocol.obuf=out_file;protocol.obuflen=sizeof out_file;
 protocol.txd=transmit;protocol.ixd=pending;protocol.openf=open_file;protocol.readf=read_file;protocol.writef=write_file;protocol.closef=close_file;protocol.filelist=names;
 status=kermit(K_INIT,&protocol,0,0,"",&response);protocol.retry=5;progress_begin();
 message(1,"Kermit %s; binary mode, Ctrl+C cancels.\n",sending?"send":"receive");
 if(sending&&status==X_OK)status=kermit(K_SEND,&protocol,0,0,"",&response);
 while(status==X_OK&&!io_failed&&!cancelled){short slot;UCHAR *p=getrslot(&protocol,&slot);if(!p)break;
  int n=read_packet(&protocol,p,P_PKTLEN+1);if(n<0)break;
  if(n==0){if(++timeouts>=5)break;}else timeouts=0;
  if(ticks()-began>36000)break;
  status=kermit(K_RUN,&protocol,slot,n,"",&response);
  transfer_bytes=response.sofar+(sending?0:protocol.obufpos);
  progress_update(transfer_bytes,transfer_size,transfer_size_known,0);
 }
 if(status==X_DONE&&completed&&!io_failed&&!cancelled)result=0;
 if(cancelled)result=130;
end:
 if(progress_enabled){progress_update(transfer_bytes,transfer_size,transfer_size_known,result?-1:1);progress_enabled=0;}
 e=file_close();if(e)result=1;
 if(owned&&file_finish(0)) {message(2,"kermit: temporary cleanup failed: %s\n",temporary);result=1;}

#ifndef KERMIT_HOST
 remote_transfer_failed=result!=0;
#endif
 e=transport_close();if(e){message(2,"kermit: serial cleanup failed (%d)\n",e);result=1;}
 message(2,"kermit: %s%s\n",result==0?"transfer complete":result==130?"cancelled":"transfer failed",completed&&result?" (complete file may already be present)":"");
 return result;
usage:
#ifdef KERMIT_HOST
 help_output(2,help);
#else
 if(!remote_wire)help_output(2,help);
#endif
 return 2;
}
