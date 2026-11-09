#define PORT_RUNTIME
#include "awk-compat.h"
extern int cli_environment(int,char *,unsigned long);
extern int cli_path_kind(const char *);
extern unsigned long cli_unix_time(void);
static char *environment[19];
char **port_environ=environment;
void port_awk_init(void){char b[164];int n=0;for(int i=0;i<18;i++)if(cli_environment(i,b,sizeof b)>0){environment[n]=port_strdup(b);if(!environment[n++])port_exit(2);}environment[n]=NULL;}
char *port_getenv(const char *name){size_t n=strlen(name);for(int i=0;environment[i];i++)if(!strncmp(name,environment[i],n)&&environment[i][n]=='=')return environment[i]+n+1;return NULL;}
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
