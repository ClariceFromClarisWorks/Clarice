#include <assert.h>
#include <stdio.h>
#include "../../src/update/crc32.h"
int main(void){UpdateCRC32 c;update_crc32_init(&c);assert(update_crc32_sum(&c)==0);update_crc32_bytes(&c,"123456789",9);assert(update_crc32_sum(&c)==0xcbf43926);update_crc32_init(&c);unsigned char b[1024];for(unsigned i=0;i<1024;i++)b[i]=i;for(unsigned i=0;i<64;i++)update_crc32_bytes(&c,b,sizeof b);assert(update_crc32_sum(&c)==0xb11de6a1);puts("PASS: IEEE CRC-32 empty, known vector, incremental 64 KB guest benchmark");}
