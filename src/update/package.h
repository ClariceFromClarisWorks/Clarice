/* Strict bounded MacBinary package inspection; no file I/O or allocation.
 * Package names are descriptive only: never use them as destination paths.
 */
#ifndef CLARICE_UPDATE_PACKAGE_H
#define CLARICE_UPDATE_PACKAGE_H
#include <stdint.h>
#include <string.h>
typedef struct {uint32_t data_size,resource_size,resource_offset,total_size,created,modified;uint16_t flags;} UpdatePackage;
static uint32_t update_be32(const unsigned char *p){return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3];}
static uint16_t update_crc(const unsigned char *p,unsigned n){uint16_t c=0;while(n--){c^=(uint16_t)*p++<<8;for(int i=0;i<8;i++)c=(c<<1)^((c&0x8000)?0x1021:0);}return c;}
static int update_package_kind(const unsigned char *h,uint32_t size,UpdatePackage *p,const char creator[4]){
 if(size<128||h[0]||!h[1]||h[1]>31||h[74]||h[82]||h[99]||h[120]||h[121])return -50;
 /* Retro68 emits classic headers with a CRC. Extended/secondary headers,
    comments and MacBinary III are deliberately not accepted by this format. */
 for(unsigned i=0;i<h[1];i++)if(h[2+i]<32||h[2+i]==':'||h[2+i]==127)return -50;
 for(unsigned i=102;i<124;i++)if(h[i])return -50;
 if(h[126]||h[127]||update_crc(h,124)!=((unsigned)h[124]<<8|h[125]))return -50;
 if(memcmp(h+65,"APPL",4)||memcmp(h+69,creator,4))return -50;
 p->data_size=update_be32(h+83);p->resource_size=update_be32(h+87);
 /* A package is bounded to 16 MiB per fork, not loaded into RAM. */
 if(p->data_size>16777216UL||!p->resource_size||p->resource_size>16777216UL)return -50;
 p->resource_offset=128+((p->data_size+127)&~127UL);
 p->total_size=p->resource_offset+((p->resource_size+127)&~127UL);
 if(p->total_size!=size)return -50;
 p->flags=((unsigned)h[73]<<8)|h[101];p->created=update_be32(h+91);p->modified=update_be32(h+95);return 0;
}
static int update_package(const unsigned char *h,uint32_t size,UpdatePackage *p){return update_package_kind(h,size,p,"CLRC");}
#endif
