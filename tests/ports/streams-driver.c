#define main host_driver_unused
#include "host.c"
#undef main
#include <assert.h>
static int handles(void){int n=0;for(int i=0;i<512;i++)if(fcntl(i,F_GETFD)!=-1)n++;return n;}
int main(int argc,char **argv){int base=handles();setenv("CLARICE_TEST_STATS","1",1);
 for(int i=0;i<3;i++){polls=0;assert(clarice_main(argc,argv)==0);assert(handles()==base&&!host_owned);}
 const char *budgets[]={"2500","6000","9000"};
 for(int i=0;i<3;i++){polls=0;setenv("CLARICE_TEST_ALLOC_BUDGET",budgets[i],1);assert(clarice_main(argc,argv)==2);assert(handles()==base&&!host_owned);}unsetenv("CLARICE_TEST_ALLOC_BUDGET");
 for(int i=1;i<180;i+=7){char b[16];snprintf(b,sizeof b,"%d",i);polls=0;setenv("CLARICE_CANCEL_AFTER",b,1);int rc=clarice_main(argc,argv);assert(rc==130||rc==0);assert(handles()==base&&!host_owned);}unsetenv("CLARICE_CANCEL_AFTER");
 polls=0;assert(!clarice_main(argc,argv)&&handles()==base&&!host_owned);puts("PASS elastic streams/descriptors: reuse, fdopen ownership, repeated invocation, low memory and cancellation without handles/allocations retained");return 0;}
