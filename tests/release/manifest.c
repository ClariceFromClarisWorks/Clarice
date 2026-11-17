#include <assert.h>
#include <stdio.h>
#include "../../src/update/release-manifest.h"
static void put(unsigned char *p,uint32_t n){p[0]=n>>24;p[1]=n>>16;p[2]=n>>8;p[3]=n;}
static void seal(unsigned char *p){UpdateCRC32 c;update_crc32_init(&c);update_crc32_bytes(&c,p,RELEASE_MANIFEST_SIZE-4);put(p+RELEASE_MANIFEST_SIZE-4,update_crc32_sum(&c));}
int main(void){unsigned char p[RELEASE_MANIFEST_SIZE]={0};memcpy(p,"CLREL001",8);put(p+8,42);put(p+12,RELEASE_COUNT);put(p+48,4096);
for(unsigned i=0;i<RELEASE_COUNT;i++){memcpy(p+56+i*32,release_names[i],strlen(release_names[i]));put(p+80+i*32,100);}
seal(p);assert(!release_manifest_check(p,sizeof p));assert(release_manifest_check(p,sizeof p-1));
for(unsigned i=0;i<sizeof p;i++){p[i]^=1;assert(release_manifest_check(p,sizeof p));p[i]^=1;}
p[56]='.';seal(p);assert(release_manifest_check(p,sizeof p));
printf("PASS: %lu-command manifest, every-byte corruption, truncation and invalid names; %lu bytes transient storage\n",(unsigned long)RELEASE_COUNT,(unsigned long)sizeof p);}
