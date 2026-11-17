/* Test-only admission failure; production cli_alloc is unchanged. */
#include <stdlib.h>
extern void *cli_alloc(unsigned long);
extern int runtime_main(int,char**);
static unsigned long budget;
void *env_test_alloc(unsigned long n){if(n>budget)return NULL;budget-=n;return cli_alloc(n);}
int clarice_main(int argc,char **argv){budget=argc>1?(unsigned long)atoi(argv[1]):65536;return runtime_main(argc,argv);}
