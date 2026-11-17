/* Runs unchanged through the host shim and the real guest runtime. */
#include "compat.h"
#include <stdint.h>
int upstream_main(int argc,char **argv){
 (void)argc;(void)argv;
 unsigned char *p=malloc(128UL*1024);if(!p)return 10;
 for(unsigned long i=0;i<128UL*1024;i++)p[i]=(unsigned char)i;
 errno=0;void *q=realloc(p,SIZE_MAX);if(q||errno!=ENOMEM)return 11;
 for(unsigned long i=0;i<128UL*1024;i++)if(p[i]!=(unsigned char)i)return 12;
 free(p);
 /* Run out of guest memory gracefully, then release every successful block. */
 void *blocks[48];int n=0;while(n<48&&(blocks[n]=malloc(16384)))n++;
 if(!n)return 13;
 while(n)free(blocks[--n]);
 p=malloc(128UL*1024);if(!p)return 14;free(p);
 puts("PASS demand allocation >96KB, failed realloc, exhaustion and recovery");
 return 0;
}
