/* Optional-module authentication/configuration; no Core crypto or idle cache. */
#include "../../remote-settings.h"
extern int cli_file_open(const char *,const char *);
extern long cli_file_read(int,void *,unsigned long);
extern long cli_file_write(int,const void *,unsigned long);
extern int cli_file_close(int);
extern void *cli_replace_begin(const char *,char *,unsigned long);
extern int cli_replace_commit(void *);
extern void cli_replace_abort(void *);
extern unsigned long cli_mac_time(void);
#define AUTH_ROUNDS 1000
#define AUTH_PATH "/etc/clarice/telnet.conf"
static unsigned char salt[16],verifier[32];
static int protected_login,auth_pending,auth_overflow,auth_cancel;
static unsigned auth_length;
static char password[128];
static unsigned long auth_started,auth_retry;
static void wipe(void *p,size_t n){volatile unsigned char *v=p;while(n--)*v++=0;}
static int auth_yield(void){if(app_yield())auth_cancel=1;return auth_cancel;}
#include "pbkdf2.inc"
static int derive(const char *p,unsigned char *out){auth_cancel=0;memset(out,0,32);pbkdf2_sha256((const uint8_t*)p,strlen(p),salt,16,AUTH_ROUNDS,out,32);return auth_cancel?-1:0;}
static void hex(char *out,const unsigned char *in,int n){const char *digits="0123456789abcdef";for(int i=0;i<n;i++){out[2*i]=digits[in[i]>>4];out[2*i+1]=digits[in[i]&15];}out[2*n]=0;}
static int unhex(unsigned char *out,const char *in,int n){if(strlen(in)!=(size_t)n*2)return -1;for(int i=0;i<n;i++){unsigned x=0;for(int j=0;j<2;j++){char c=in[2*i+j];int v=c>='0'&&c<='9'?c-'0':c>='a'&&c<='f'?c-'a'+10:-1;if(v<0)return -1;x=x*16+v;}out[i]=x;}return 0;}
static int config_load(ClariceRemoteSettings *s){
 char data[256],ip[16],a[33],b[65],extra;unsigned p=0,rounds=0;int ref=cli_file_open(AUTH_PATH,"r");if(ref<0)return -1;
 long n=cli_file_read(ref,data,sizeof data-1);int e=cli_file_close(ref);if(n<0||n>=(long)sizeof data-1||e||memchr(data,0,n))return -1;data[n]=0;
 if(sscanf(data,"CLARICE-TELNET-1 %15s %u %u %32s %64s %c",ip,&p,&rounds,a,b,&extra)!=5||!p||p>65535||rounds!=AUTH_ROUNDS||!NetParseIP(ip)||unhex(salt,a,16)||unhex(verifier,b,32))return -1;
 strcpy(s->peer,ip);s->port=p;return 0;
}
static int config_save(ClariceRemoteSettings *s){
 char data[256],target[512],a[33],b[65];hex(a,salt,16);hex(b,verifier,32);
 int n=snprintf(data,sizeof data,"CLARICE-TELNET-1 %s %u %u %s %s\n",s->peer,s->port,AUTH_ROUNDS,a,b);
 void *save=cli_replace_begin(AUTH_PATH,target,sizeof target);if(!save)return -1;
 int ref=cli_file_open(target,"w"),e=0;if(ref<0)e=-1;else{if(cli_file_write(ref,data,n)!=n)e=-1;if(cli_file_close(ref))e=-1;}
 if(e){cli_replace_abort(save);return -1;}return cli_replace_commit(save);
}
static int configure(void){
 ClariceRemoteSettings s;memset(&s,0,sizeof s);s.port=2323;s.enabled=cli_service_enabled();int existing=!config_load(&s);
 int result=app_remote_settings(&s);if(result<=0){wipe(s.password,sizeof s.password);return result<0?1:0;}
 if(!s.enabled&&!existing&&!s.password[0])return cli_service_stop()?1:0;
 if(!NetParseIP(s.peer)||(s.enabled&&!existing&&!s.password[0])){wipe(s.password,sizeof s.password);const char *m="telnetd: valid peer and password required; settings unchanged.\n";cli_write(2,m,strlen(m));return 1;}
 if(s.password[0]){
  /* Salt uniqueness, not a secret/random-key claim: time/ticks + previous salt. */
  struct sha256 h;unsigned char digest[32];unsigned long time=cli_mac_time(),ticks=TickCount();sha256_init(&h);sha256_update(&h,salt,16);sha256_update(&h,&time,sizeof time);sha256_update(&h,&ticks,sizeof ticks);sha256_update(&h,s.peer,strlen(s.peer));sha256_sum(&h,digest);memcpy(salt,digest,16);wipe(&h,sizeof h);wipe(digest,sizeof digest);
  result=derive(s.password,verifier);wipe(s.password,sizeof s.password);if(result)return 130;existing=1;
 }
 if(cli_service_stop()){const char *m="telnetd: current service busy; retry after its command finishes.\n";cli_write(2,m,strlen(m));return 1;}
 if(existing&&config_save(&s)){const char *m="telnetd: settings save failed; service stopped, previous file retained or recoverable.\n";cli_write(2,m,strlen(m));return 1;}
 if(!s.enabled)return 0;
 peer=NetParseIP(s.peer);port=s.port;protected_login=1;return 3; /* start below */
}
