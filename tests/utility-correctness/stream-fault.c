/* Guest-only failure/cancellation injection around the real portable runtime. */
#include <stdlib.h>
#include <stdio.h>
extern void *cli_alloc(unsigned long);extern void cli_free(void *);extern int app_yield(void);extern void say(const char *);extern int runtime_main(int,char **);
static unsigned calls,live,fail,polls,cancel;
void *fixture_alloc(unsigned long n){if(fail&&++calls==fail)return NULL;void*p=cli_alloc(n);if(p)live++;return p;}
void fixture_free(void*p){if(p){live--;cli_free(p);}}
int fixture_yield(void){return (cancel&&++polls>=cancel)||app_yield();}
int clarice_main(int argc,char **argv){fail=argc>4?atoi(argv[4]):0;cancel=argc>5?atoi(argv[5]):0;int rc=runtime_main(argc,argv);char b[80];snprintf(b,sizeof b,"stream fixture: status=%d live=%u\n",rc,live);say(b);return live?99:rc;}
