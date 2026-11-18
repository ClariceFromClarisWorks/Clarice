/* Server-role proof using unchanged libtelnet, not a second protocol parser.
   Two queued endpoints let fragmented wire delivery exercise both directions. */
#include <string.h>
#include <stdio.h>
#include "../../src/commands/vendor/libtelnet/libtelnet.h"
#ifdef REMOTE_GUEST
extern void say(const char *);
#define PRINT(s) say(s)
#else
#include <stdlib.h>
#define PRINT(s) fputs(s,stdout)
static unsigned blocks,peak,bytes;
typedef struct {unsigned long n;} Allocation;
void *cli_alloc(unsigned long n){Allocation *p=malloc(sizeof *p+n);if(!p)return 0;p->n=n;blocks++;bytes+=n;if(bytes>peak)peak=bytes;return p+1;}
void cli_free(void *v){if(v){Allocation *p=(Allocation*)v-1;bytes-=p->n;blocks--;free(p);}}
#endif
extern int clarice_telnet_enabled(telnet_t *,unsigned char,int);
typedef struct {telnet_t *t;char wire[4096],data[4096];unsigned used,received;int ip,error,naws;} Peer;
static void event(telnet_t *t,telnet_event_t *ev,void *user){
 Peer *p=user;(void)t;
 if(ev->type==TELNET_EV_SEND){if(ev->data.size>sizeof p->wire-p->used){p->error=1;return;}memcpy(p->wire+p->used,ev->data.buffer,ev->data.size);p->used+=ev->data.size;}
 if(ev->type==TELNET_EV_DATA){if(ev->data.size>sizeof p->data-p->received){p->error=1;return;}memcpy(p->data+p->received,ev->data.buffer,ev->data.size);p->received+=ev->data.size;}
 if(ev->type==TELNET_EV_IAC&&ev->iac.cmd==TELNET_IP)p->ip++;
 if(ev->type==TELNET_EV_SUBNEGOTIATION&&ev->sub.telopt==TELNET_TELOPT_NAWS&&ev->sub.size==4)p->naws++;
 if(ev->type==TELNET_EV_ERROR)p->error=1;
}
static int pump(Peer *a,Peer *b){
 for(unsigned rounds=0;rounds<4096;rounds++){
  if(!a->used&&!b->used)return a->error||b->error;
  if(a->used){char c=a->wire[0];memmove(a->wire,a->wire+1,--a->used);telnet_recv(b->t,&c,1);}
  if(b->used){char c=b->wire[0];memmove(b->wire,b->wire+1,--b->used);telnet_recv(a->t,&c,1);}
 }return 1;
}
static const telnet_telopt_t server_options[]={
 {TELNET_TELOPT_ECHO,TELNET_WILL,TELNET_DONT},
 {TELNET_TELOPT_SGA,TELNET_WILL,TELNET_DO},
 {TELNET_TELOPT_BINARY,TELNET_WILL,TELNET_DO},
 {TELNET_TELOPT_NAWS,TELNET_WONT,TELNET_DO},{-1,0,0}};
static const telnet_telopt_t client_options[]={
 {TELNET_TELOPT_ECHO,TELNET_WONT,TELNET_DO},
 {TELNET_TELOPT_SGA,TELNET_WILL,TELNET_DO},
 {TELNET_TELOPT_BINARY,TELNET_WILL,TELNET_DO},
 {TELNET_TELOPT_NAWS,TELNET_WILL,TELNET_DONT},{-1,0,0}};
static int run(void){
 Peer a={0},b={0};int failed=0;unsigned char binary[256];
#define CHECK(c) do{if(!(c)){failed=__LINE__;goto done;}}while(0)
 a.t=telnet_init(server_options,event,0,&a);b.t=telnet_init(client_options,event,0,&b);CHECK(a.t&&b.t);
 telnet_negotiate(a.t,TELNET_WILL,TELNET_TELOPT_ECHO);
 telnet_negotiate(a.t,TELNET_WILL,TELNET_TELOPT_SGA);
 telnet_negotiate(a.t,TELNET_WILL,TELNET_TELOPT_BINARY);
 telnet_negotiate(a.t,TELNET_DO,TELNET_TELOPT_BINARY);
 telnet_negotiate(a.t,TELNET_DO,TELNET_TELOPT_NAWS);
 CHECK(!pump(&a,&b));CHECK(clarice_telnet_enabled(a.t,TELNET_TELOPT_ECHO,1));
 CHECK(clarice_telnet_enabled(a.t,TELNET_TELOPT_BINARY,0)&&clarice_telnet_enabled(a.t,TELNET_TELOPT_BINARY,1));
 for(int i=0;i<1000;i++){const char duplicate[]={TELNET_IAC,TELNET_DO,TELNET_TELOPT_ECHO};telnet_recv(a.t,duplicate,sizeof duplicate);CHECK(!pump(&a,&b));}
 for(unsigned i=0;i<256;i++)binary[i]=i;
 telnet_send(a.t,(char*)binary,256);telnet_send(b.t,(char*)binary,256);CHECK(!pump(&a,&b));
 CHECK(a.received==256&&b.received==256&&!memcmp(a.data,binary,256)&&!memcmp(b.data,binary,256));
 telnet_iac(b.t,TELNET_IP);CHECK(!pump(&a,&b)&&a.ip==1&&!b.ip);
 const char dimensions[]={0,80,0,24};telnet_subnegotiation(b.t,TELNET_TELOPT_NAWS,dimensions,4);CHECK(!pump(&a,&b)&&a.naws==1);
done:
 if(a.t)telnet_free(a.t);if(b.t)telnet_free(b.t);return failed;
}
#ifdef REMOTE_GUEST
int clarice_main(int argc,char **argv){(void)argc;(void)argv;
#else
int main(void){
#endif
 for(int i=0;i<20;i++){int e=run();if(e){char text[80];snprintf(text,sizeof text,"FAIL server-role protocol line %d\n",e);PRINT(text);return 1;}}
#ifndef REMOTE_GUEST
 if(blocks||bytes)return 2;char text[100];snprintf(text,sizeof text,"Host two-endpoint peak: %u B; retained: %u B\n",peak,bytes);PRINT(text);
#endif
 PRINT("PASS libtelnet server role: fragmented negotiation, duplicate options, bidirectional binary/IAC, interrupt, NAWS, 20 repeats\n");return 0;
}
