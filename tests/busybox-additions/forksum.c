/* Test-only resource-fork fingerprint. Normalizes ONLY the 112 system-reserved
 * bytes at 16..127; preserves header, application area, resource data and map.
 * Apple Inside Macintosh: More Macintosh Toolbox, Creating an Empty Resource Fork.
 * Never included in the standard catalog. */
#include "compat.h"
#include <stdint.h>
extern int resolve_ex(const char *,char *,char *,int);
extern int cli_native_open(const char *,int,int,short *);
extern int cli_native_read(short,void *,long *),cli_native_close(short),app_yield(void);
static int compare_resources(const char *a,const char *b){
 char paths[2][512],data[2][4096];short refs[2]={-1,-1};int result=0;unsigned long offset=0,reserved=0,other=0;
 for(int i=0;i<2;i++)if(resolve_ex(i?b:a,paths[i],0,1)||cli_native_open(paths[i],1,0,&refs[i])){result=2;goto done;}
 for(;;){long n[2]={4096,4096};int e[2];for(int i=0;i<2;i++)e[i]=cli_native_read(refs[i],data[i],&n[i]);
  if((e[0]&&e[0]!=-39)||(e[1]&&e[1]!=-39)){result=2;break;}
  if(n[0]!=n[1]){printf("resource length differs at %lu\n",offset);result=1;break;}
  for(long i=0;i<n[0];i++)if(data[0][i]!=data[1][i]){if(offset+i>=16 && offset+i<128)reserved++;else {if(!other)printf("resource content differs at %lu\n",offset+i);other++;}}
  offset+=n[0];if(!n[0]){printf("resource compared %lu bytes: reserved differences=%lu other differences=%lu\n",offset,reserved,other);result=other?1:0;break;}if(app_yield()){result=130;break;}
 }
 done:for(int i=0;i<2;i++)if(refs[i]>=0&&cli_native_close(refs[i])&&!result)result=2;return result;
}
int upstream_main(int argc,char **argv){
 if(argc==3)return compare_resources(argv[1],argv[2]);
 if(argc!=2)return 2;
 char native[512],b[4096];short fd=-1;int e=resolve_ex(argv[1],native,0,1);
 if(!e)e=cli_native_open(native,1,0,&fd);if(e)return 1;
 uint32_t hash=2166136261UL;unsigned long size=0;
 for(;;){long n=sizeof(b);e=cli_native_read(fd,b,&n);int eof=e==-39;if(e&&!eof)break;
  for(long i=0;i<n;i++)hash=(hash^((size+i>=16 && size+i<128)?0:(unsigned char)b[i]))*16777619UL;size+=n;
  if(app_yield()){e=130;break;}if(eof||!n){e=0;break;}}
 int close_error=cli_native_close(fd);if(e||close_error)return e==130?130:1;
 printf("resource %lu %08lx\n",size,(unsigned long)hash);return 0;
}
