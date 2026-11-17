#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../../src/update/fork-crc.h"
static uint32_t crc(const unsigned char *b,int resource,unsigned chunk){UpdateCRC32 c;update_crc32_init(&c);for(unsigned off=0;off<512;){unsigned n=512-off;if(n>chunk)n=chunk;update_fork_crc_bytes(&c,resource,off,b+off,n);off+=n;}return update_crc32_sum(&c);}
int main(void){unsigned char b[512]={0},x[512];b[2]=1;b[6]=1;b[7]=128;b[11]=128;b[15]=128;assert(!update_resource_layout(b,sizeof b));uint32_t base=crc(b,1,512);for(unsigned chunk=1;chunk<513;chunk++)assert(crc(b,1,chunk)==base);for(unsigned i=0;i<512;i++){memcpy(x,b,sizeof x);x[i]^=1;assert((crc(x,1,7)==base)==(i>=16&&i<128));assert(crc(x,0,7)!=crc(b,0,512));}memcpy(x,b,sizeof x);x[2]=0;assert(update_resource_layout(x,sizeof x));memcpy(x,b,sizeof x);x[4]=255;assert(update_resource_layout(x,sizeof x));assert(update_resource_layout(b,255));puts("PASS: all chunk boundaries, only OS scratch excluded; every other byte/data-fork mutation detected; layout bounds");}
