"""Exercise the production editor environment adapter with fault-owned allocation."""
from pathlib import Path
import subprocess
r=Path(__file__).resolve().parents[2];w=r/'work/heap-audit';w.mkdir(exist_ok=True)
s=(r/'src/ports/terminal/platform.c').read_text();s=s[s.index('extern int cli_environment'):s.index('/* Synchronous signal')]
head=r'''
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <setjmp.h>
#include <assert.h>
static jmp_buf escape;
static int calls,fail_at,cancel_at,status,entries=80;
static size_t length=4096,live,peak;
struct owned {struct owned *next;size_t n;};static struct owned *owned;
static void *take(size_t n){calls++;if(calls==cancel_at){status=130;longjmp(escape,1);}if(calls==fail_at){errno=ENOMEM;return NULL;}struct owned *p=malloc(sizeof *p+n);assert(p);p->next=owned;p->n=n;owned=p;live+=n;if(live>peak)peak=live;return p+1;}
static void drop(void *v){if(!v)return;struct owned *p=(struct owned*)v-1,**slot=&owned;while(*slot&&*slot!=p)slot=&(*slot)->next;assert(*slot);*slot=p->next;live-=p->n;free(p);}
static void *grow(void *v,size_t n){void *p=take(n);if(p&&v){struct owned *old=(struct owned*)v-1;memcpy(p,v,old->n<n?old->n:n);drop(v);}return p;}
static void port_exit(int n){status=n;longjmp(escape,1);}
int cli_environment(int i,char *p,unsigned long cap){if(i==entries)return 0;char name[32];snprintf(name,sizeof name,i==0?"TERM=":"ENV%d=",i);size_t n=strlen(name);if(n+length+1>cap)return -1;memcpy(p,name,n);memset(p+n,'x',length);p[n+length]=0;return 1;}
#define malloc take
#define free drop
#define realloc grow
#define setenv editor_setenv
#define unsetenv editor_unsetenv
'''
tail=r'''
static void cleanup(void){while(owned)drop(owned+1);assert(!live);environment=NULL;env_ready=0;}
static int run(void){calls=0;status=0;if(!setjmp(escape)){
 assert(!strcmp(port_curses_getenv("TERM"),"clarice"));
 for(int i=1;i<entries;i++){char name[32];snprintf(name,sizeof name,"ENV%d",i);assert(strlen(port_curses_getenv(name))==length);}
 assert(!port_curses_getenv("PRIVATE"));
 assert(!setenv("NEW","value",0));assert(!setenv("NEW","ignored",0));assert(!strcmp(port_curses_getenv("NEW"),"value"));
 assert(!setenv("NEW","replacement",1));assert(!unsetenv("NEW"));assert(!port_curses_getenv("NEW"));
 assert(setenv("bad=name","x",1)==-1);assert(unsetenv("")==-1);
 }int total=calls;cleanup();return total;}
int main(void){int total=run();assert(!status);size_t measured=peak;
 /* Fault every import allocation, including TERM override. Mutation ENOMEM
    returns -1 instead of exiting; test it separately below. */
 for(int i=1;i<=total-2;i++){fail_at=i;run();assert(status==2);}fail_at=0;
 for(int i=1;i<=total-2;i+=7){cancel_at=i;run();assert(status==130);}cancel_at=0;
 for(int i=0;i<20;i++)run();assert(!status);
 env_init();fail_at=calls+1;assert(setenv("ENV1","lost",1)==-1);assert(strlen(port_curses_getenv("ENV1"))==length);fail_at=0;cleanup();
 entries=3;length=4;run();assert(!status);
 printf("PASS editor environment: 80 exports x 4096 bytes; %d import failure points; cancellation, replacement, 20 repeats; peak=%zu remaining=%zu\n",total-2,measured,live);
}
'''
(w/'environment.c').write_text(head+s+tail)
subprocess.run(['cc','-fsanitize=address,undefined','-g',str(w/'environment.c'),'-o',str(w/'environment')],check=True)
p=subprocess.run([str(w/'environment')],capture_output=True,text=True)
if p.returncode: raise RuntimeError(p.stdout+p.stderr)
print(p.stdout,end='')
