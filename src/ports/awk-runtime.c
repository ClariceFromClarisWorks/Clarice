#define PORT_RUNTIME
#include "awk-compat.h"
extern int cli_environment(int,char *,unsigned long);
extern int cli_path_kind(const char *);
extern unsigned long cli_unix_time(void);
/* Snapshot only this invocation's exported values. The runtime owns all
 * allocations and reclaims them on normal exit, cancellation or port_exit. */
static char **environment;
char **port_environ;
void port_awk_init(void){size_t capacity=128,slots=8,n=0;char *buffer=port_malloc(capacity);
 environment=port_malloc(slots*sizeof *environment);port_environ=environment;
 if(!buffer||!environment)port_exit(2);
 environment[0]=NULL;
 for(int i=0;;i++){
  int result=cli_environment(i,buffer,capacity);
  while(result<0){if(capacity>SIZE_MAX/2)port_exit(2);capacity*=2;char *p=port_realloc(buffer,capacity);if(!p)port_exit(2);buffer=p;result=cli_environment(i,buffer,capacity);}
  if(!result)break;
  if(n+1==slots){if(slots>SIZE_MAX/2/sizeof *environment)port_exit(2);slots*=2;char **p=port_realloc(environment,slots*sizeof *environment);if(!p)port_exit(2);environment=p;port_environ=p;}
  environment[n]=port_strdup(buffer);if(!environment[n])port_exit(2);environment[++n]=NULL;
 }
 port_free(buffer);
}
char *port_getenv(const char *name){size_t n=strlen(name);for(size_t i=0;environment&&environment[i];i++)if(!strncmp(name,environment[i],n)&&environment[i][n]=='=')return environment[i]+n+1;return NULL;}
static void unsupported(const char *feature){port_fprintf(port_standard(2),"awk: %s is not supported by the Clarice runtime\n",feature);port_exit(2);}
int port_system(const char *s){(void)s;unsupported("system()");return -1;}
FILE *port_popen(const char *s,const char *m){(void)s;(void)m;unsupported("command pipes inside awk");return NULL;}
int port_pclose(FILE *f){(void)f;unsupported("pclose()");return -1;}
int port_stat(const char *path,struct stat *st){int kind=cli_path_kind(path);if(kind<0)return -1;memset(st,0,sizeof *st);st->st_mode=kind?S_IFDIR:S_IFREG;return 0;}
time_t port_time(time_t *out){time_t t=(time_t)cli_unix_time();if(out)*out=t;return t;}
static unsigned long seed=1;
void port_srandom(unsigned long n){seed=n&0x7fffffffUL;}
long port_random(void){seed=(seed*1103515245UL+12345UL)&0x7fffffffUL;return (long)seed;}

void port_assert(const char *file,int line,const char *func,const char *expr){port_fprintf(port_standard(2),"awk: assertion %s at %s:%d (%s)\n",expr,file,line,func);port_exit(2);}
