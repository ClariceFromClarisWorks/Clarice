/* Resource fork integrity excludes only bytes 16..127: File Manager recovery
 * scratch. Native copying demonstrably rewrites that area (including filename).
 * The 16-byte structural header, bytes 128 onward, data and map remain checked.
 * Source MacBinary CRC still covers EVERY byte. This is not authentication.
 * Apple Inside Macintosh: More Macintosh Toolbox pp. 1-53, 1-121; earlier
 * Resource Manager format reserves 112 bytes for system use after the header.
 */
#ifndef CLARICE_UPDATE_FORK_CRC_H
#define CLARICE_UPDATE_FORK_CRC_H
#include "crc32.h"
#include "package.h"
static int update_resource_layout(const unsigned char *h,uint32_t size){
 if(size<256)return -50;
 uint32_t data=update_be32(h),map=update_be32(h+4),dn=update_be32(h+8),mn=update_be32(h+12);
 if(data<256||map<256||data>size||map>size||dn>size-data||mn>size-map||mn<28)return -50;
 return data+dn<=map||map+mn<=data?0:-50;
}
static void update_fork_crc_bytes(UpdateCRC32 *c,int resource,uint32_t offset,const void *data,unsigned long n){
 const unsigned char *p=data;
 if(resource&&offset<128){
  if(offset<16){unsigned long take=16-offset;if(take>n)take=n;update_crc32_bytes(c,p,take);p+=take;n-=take;offset+=take;}
  unsigned long skip=128-offset;if(skip>n)skip=n;p+=skip;n-=skip;
 }
 update_crc32_bytes(c,p,n);
}
#endif
