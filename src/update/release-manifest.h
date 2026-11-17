/* Generated collection manifest; fixed-width encoding, no JSON parser or heap.
 * Names come from the same inventory used to build/package standard commands.
 * CRC detects transfer errors, not authentication. */
#ifndef CLARICE_RELEASE_MANIFEST_H
#define CLARICE_RELEASE_MANIFEST_H
#include <stdint.h>
#include <string.h>
#include "crc32.h"
#include "package.h"
static const char *const release_names[]={
#define CLARICE_COMMAND(name,image) name,
#include "../standard-commands.inc"
#undef CLARICE_COMMAND
};
#define RELEASE_COUNT (sizeof release_names/sizeof release_names[0])
#define RELEASE_MANIFEST_SIZE (60+32*RELEASE_COUNT)
static int release_manifest_check(const unsigned char *p,unsigned long size){
 if(size!=RELEASE_MANIFEST_SIZE||memcmp(p,"CLREL001",8)||!update_be32(p+8)||update_be32(p+12)!=RELEASE_COUNT)return -50;
 UpdateCRC32 crc;update_crc32_init(&crc);update_crc32_bytes(&crc,p,size-4);
 if(update_crc32_sum(&crc)!=update_be32(p+size-4))return -50;
 if(update_be32(p+48)<128||update_be32(p+48)>33554560UL)return -50;
 for(unsigned i=0;i<RELEASE_COUNT;i++){
  const unsigned char *r=p+56+i*32;unsigned n=strlen(release_names[i]);
  if(memcmp(r,release_names[i],n))return -50;
  for(unsigned j=n;j<24;j++)if(r[j])return -50;
  if(update_be32(r+24)<36||update_be32(r+24)>368688UL)return -50;
 }
 return 0;
}
#endif
