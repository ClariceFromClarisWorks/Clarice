/* One installation per running System. Paths are native HFS names, never Unix
 * operands. The record is written once after confirmation, not on startup.
 * Existing installations are not relocated by changing this record. */
#ifndef CLARICE_LOCATIONS_H
#define CLARICE_LOCATIONS_H
#include "native-path.h"
#include "crc32.h"
#define LOCATION_BYTES 524
#define LOCATION_LEAF "Clarice Locations"
typedef struct {char core[256],data[256];} InstallLocations;
static int locations_valid(const InstallLocations *p){
 if(!recovery_path(p->core,0)||!recovery_path(p->data,1))return 0;
 size_t n=strlen(p->data),v=strchr(p->core,':')-p->core;
 if(n+sizeof("etc:clarice:Recovery Preparing:Recover Clarice")>256||
    (size_t)(strrchr(p->core,':')-p->core)+33>256)return 0;
 return n>=13&&!strcmp(p->data+n-13,"Clarice Data:")&&
  !strcmp(strrchr(p->core,':')+1,"Clarice")&&
  (size_t)(strchr(p->data,':')-p->data)==v&&!memcmp(p->core,p->data,v)&&
  strncmp(p->core,p->data,n);
}
static int locations_encode(const InstallLocations *p,unsigned char bytes[LOCATION_BYTES]){
 if(!locations_valid(p))return -50;memset(bytes,0,LOCATION_BYTES);
 memcpy(bytes,"CLLOC001",8);memcpy(bytes+8,p->core,strlen(p->core));memcpy(bytes+264,p->data,strlen(p->data));
 UpdateCRC32 crc;update_crc32_init(&crc);update_crc32_bytes(&crc,bytes,520);uint32_t sum=update_crc32_sum(&crc);for(unsigned i=0;i<4;i++)bytes[520+i]=sum>>(24-8*i);return 0;
}
static int locations_decode(const unsigned char bytes[LOCATION_BYTES],InstallLocations *p){
 if(memcmp(bytes,"CLLOC001",8)||!memchr(bytes+8,0,256)||!memchr(bytes+264,0,256))return -50;
 InstallLocations next;memcpy(next.core,bytes+8,256);memcpy(next.data,bytes+264,256);
 unsigned char canonical[LOCATION_BYTES];int e=locations_encode(&next,canonical);if(e||memcmp(canonical,bytes,LOCATION_BYTES))return -50;*p=next;return 0;
}
#endif
