/* IEEE CRC-32 for accidental corruption detection, not authentication.
 * Table belongs to the foreground operation, never the idle shell. */
#ifndef CLARICE_UPDATE_CRC32_H
#define CLARICE_UPDATE_CRC32_H
#include <stdint.h>
typedef struct {uint32_t value,table[256];} UpdateCRC32;
static void update_crc32_init(UpdateCRC32 *c){c->value=0xffffffffUL;for(unsigned i=0;i<256;i++){uint32_t v=i;for(unsigned bit=0;bit<8;bit++)v=(v>>1)^((v&1)?0xedb88320UL:0);c->table[i]=v;}}
static void update_crc32_bytes(UpdateCRC32 *c,const void *data,unsigned long n){const unsigned char *p=data;uint32_t v=c->value;while(n--)v=c->table[(v^*p++)&255]^(v>>8);c->value=v;}
static uint32_t update_crc32_sum(const UpdateCRC32 *c){return c->value^0xffffffffUL;}
#endif
