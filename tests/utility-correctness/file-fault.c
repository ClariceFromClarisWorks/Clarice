/* Disposable guest fault injection; never linked into normal commands. */
#include <stdlib.h>
#include <stdio.h>
extern void *cli_alloc(unsigned long);extern void cli_free(void *);extern int app_yield(void);extern void say(const char *);extern int files_main(int,char **);
extern int cli_native_write(short,const void *,long *);
static unsigned calls,live,fail,cancel,polls;
int fixture_write(short r,const void*p,long*n){if(fail==1000){*n=0;return -34;}return cli_native_write(r,p,n);}
void *fixture_alloc(unsigned long n){if(fail&&++calls==fail)return NULL;void*p=cli_alloc(n);if(p)live++;return p;}
void fixture_free(void*p){if(p){live--;cli_free(p);}}
int fixture_yield(void){return (cancel&&++polls>=cancel)||app_yield();}
int clarice_main(int argc,char **argv){if(argc<4)return 2;fail=atoi(argv[1]);cancel=atoi(argv[2]);char *name=argv[3];argv[3]=name;int rc=files_main(argc-3,argv+3);char b[80];snprintf(b,sizeof b,"fixture: status=%d live=%u\n",rc,live);say(b);return live?99:rc;}
