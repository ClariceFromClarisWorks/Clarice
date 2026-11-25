#define FTP_HOST_TEST
#include "../../src/commands/ftp.c"
#include <assert.h>
static int native_cancel;
static unsigned char wire[8192],saved[8192],source[8192];
static unsigned wp,wl,dp,bytes,sourcepos,len,ticks,calls;
static int live,files,temp,exists,committed,upload,pasv,unknown,shortdata,finalbad,stall,stop,connectfail,writefail,closefail,renamefail,createfail,deletefail,badpassive;
static int stor_disconnect,size421,status_error;
static char diagnostics[4096];
static void queue(const char *s){assert(wp==wl);wp=0;wl=strlen(s);memcpy(wire,s,wl);}
int app_columns(void){return 80;}int cli_isatty(int f){(void)f;return 0;}
void app_progress(const char *s,int d){(void)s;(void)d;assert(0);}
int app_yield(void){calls++;return stop==2?bytes>0:stop&&calls>80;}static unsigned pauses;
int cli_pause_ticks(unsigned long n){assert(n==1);pauses++;ticks+=n;return app_yield();}
unsigned long cli_ticks(void){return ++ticks;}
int checked(const char *a,char *b,char *c,int f){(void)f;snprintf(b,PATH_CAP,"Disk:%s",a);strcpy(c,a);return 0;}
int info(const char *s,CInfoPBRec *p){(void)s;p->hFileInfo.ioFlLgLen=len;return upload||exists?0:fnfErr;}int isdir(CInfoPBRec *p){(void)p;return 0;}
long cli_read(int f,void *p,unsigned long n){(void)f;(void)p;(void)n;return 0;}
int NetResolve(const char *s,unsigned long *ip,int(*p)(void),unsigned long t){(void)s;(void)p;(void)t;*ip=1;return 0;}
int NetConnect(NetConn *c,unsigned long ip,unsigned short port){if(native_cancel==1)return userCanceledErr;assert(ip==1);if(connectfail==1||(connectfail==2&&c==&data))return -108;c->open=1;live++;if(c==&control){assert(port==21);queue("220-Welcome\r\ntext\r\n220 Ready\r\n");}else assert(port==1234);return 0;}
int NetSend(NetConn *c,const void *s,unsigned short n){if(native_cancel==2)return userCanceledErr;if(c==&data){assert(bytes+n<=sizeof saved);memcpy(saved+bytes,s,n);bytes+=n;return 0;}
 assert(!memcmp((char*)s+n-2,"\r\n",2));if(!strncmp(s,"USER ",5))queue("331 Password\r\n");else if(!strncmp(s,"PASS ",5))queue("230 Welcome\r\n");else if(!strncmp(s,"TYPE I",6))queue("200 Binary\r\n");else if(!strncmp(s,"SIZE ",5)){char t[64];snprintf(t,sizeof t,"213 %u\r\n",len);queue(size421?"421 Server shutting down\r\n":unknown?"500 Unsupported\r\n":t);}else if(!strncmp(s,"EPSV",4)&&badpassive==2){char t[1025];memset(t,'x',1021);memcpy(t,"229 ",4);memcpy(t+1021,"(\r\n",4);queue(t);}else if(!strncmp(s,"EPSV",4))queue(pasv?"500 Unsupported\r\n":badpassive?"229 (|||99999|)\r\n":"229 (|||1234|)\r\n");else if(!strncmp(s,"PASV",4))queue("227 (203,0,113,9,4,210)\r\n");else if(!strncmp(s,"RETR ",5)||!strncmp(s,"STOR ",5))queue(stor_disconnect?"":finalbad?"150 Start\r\n426 Interrupted\r\n":"150 Start\r\n226 Complete\r\n");else assert(0);return 0;}
long NetAvailable(NetConn *c){if(c==&control)return wp<wl?(long)(wl-wp):connectionClosing;if(stall)return 0;if(status_error&&dp>=2048)return status_error;unsigned target=shortdata?len/2:len;return dp<target?(long)(target-dp):connectionClosing;}
long NetRecv(NetConn *c,void *p,unsigned short n,unsigned char t){(void)t;if(c==&control){/* Force parsing across arbitrary packet boundaries. */if(n>7)n=7;memcpy(p,wire+wp,n);wp+=n;}else{memcpy(p,source+dp,n);dp+=n;}return n;}
int NetClose(NetConn *c){if(c->open){c->open=0;live--;}return native_cancel==3?userCanceledErr:0;}
void say(const char *s){(void)s;}void errsay(const char *s){assert(strlen(diagnostics)+strlen(s)<sizeof diagnostics);strcat(diagnostics,s);}
int cli_native_create(const char *s,unsigned long t,unsigned long c){(void)s;(void)t;(void)c;if(createfail)return -34;assert(!temp);temp=1;return 0;}
int cli_native_open(const char *s,int a,int b,short *f){(void)s;(void)a;(void)b;*f=1;files++;return 0;}
int cli_native_read(short f,void *p,long *n){(void)f;if((unsigned long)*n>len-sourcepos)*n=len-sourcepos;memcpy(p,source+sourcepos,*n);sourcepos+=*n;return *n?0:eofErr;}
int cli_native_write(short f,const void *p,long *n){(void)f;if(writefail)return -34;assert(bytes+*n<=sizeof saved);memcpy(saved+bytes,p,*n);bytes+=*n;return 0;}
int cli_native_close(short f){(void)f;assert(files==1);files--;return closefail?ioErr:0;}
int cli_native_rename(const char *a,const char *b){(void)a;(void)b;if(renamefail)return ioErr;assert(temp&&!exists);temp=0;committed=1;return 0;}
int cli_native_delete(const char *s){(void)s;if(deletefail)return ioErr;temp=0;return 0;}
static void reset(void){assert(!live&&!files);wp=wl=dp=bytes=sourcepos=ticks=calls=0;stor_disconnect=size421=status_error=0;temp=exists=committed=upload=pasv=unknown=shortdata=finalbad=stall=stop=connectfail=writefail=closefail=renamefail=createfail=deletefail=badpassive=0;len=4096;diagnostics[0]=0;for(unsigned i=0;i<sizeof source;i++)source[i]=(unsigned char)(i*71);}
static int run(void){char *a[]={"ftp",upload?"put":"get","example.test","source","dest"};int r=clarice_main(5,a);assert(!live&&!files);return r;}
int main(void){
 for(int phase=1;phase<=3;phase++){reset();native_cancel=phase;assert(run()==130&&!live&&!files&&!temp);native_cancel=0;}

 for(int mode=0;mode<6;mode++){reset();upload=mode&1;pasv=mode==2;unknown=mode==3;if(mode>=4)len=0;assert(run()==0&&!temp&&bytes==len&&!memcmp(saved,source,len));if(!upload)assert(committed);}
 for(int mode=0;mode<12;mode++){reset();switch(mode){case 0:exists=1;break;case 1:shortdata=1;break;case 2:finalbad=1;break;case 3:stall=1;break;case 4:stop=stall=1;break;case 5:connectfail=1;break;case 6:writefail=1;break;case 7:closefail=1;break;case 8:renamefail=1;break;case 9:createfail=1;break;case 10:badpassive=1;break;case 11:badpassive=2;break;}int r=run();assert(r!=0&&!committed&&!temp);if(mode==4)assert(r==130);}
 reset();shortdata=deletefail=1;assert(run()!=0&&temp&&strstr(diagnostics,"recover temporary"));
 reset();upload=finalbad=1;assert(run()!=0&&strstr(diagnostics,"remote upload may be partial"));
 reset();upload=stor_disconnect=1;assert(run()!=0&&strstr(diagnostics,"remote upload may be partial"));
 reset();size421=1;assert(run()!=0&&strstr(diagnostics,"421 Server shutting down")&&!temp);
 reset();connectfail=2;assert(run()!=0&&!temp);
 reset();upload=1;stop=2;assert(run()==130&&strstr(diagnostics,"remote upload may be partial"));
 reset();unknown=1;status_error=-23012;assert(run()!=0&&!committed&&!temp&&bytes==2048&&strstr(diagnostics,"-23012"));
 for(int i=0;i<20;i++){reset();assert(run()==0&&!temp&&committed);}
 reset();char *help[]={"ftp","--help"};assert(clarice_main(2,help)==0&&!live&&!files);
 char *bad[]={"ftp","-P","65536","get","host","remote","local"};assert(clarice_main(7,bad)==2&&!live&&!files);
 char *auth[]={"ftp","-u","user","get","host","remote","local"};assert(clarice_main(7,auth)==2&&!live&&!files);
 puts("PASS FTP: binary/empty GET and PUT; fragmented/multiline replies; EPSV/PASV peer isolation; unknown size; truncation/final failure; existing destination; timeout/cancel; allocation/create/write/close/rename failures; cleanup diagnostics; repeated resource release.");
}
