#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../src/update/package.h"
#include "../../src/update/vendor/sha256.h"
static void crc(unsigned char *h){unsigned c=update_crc(h,124);h[124]=c>>8;h[125]=c;}
int main(int argc,char **argv){assert(argc==2);FILE *f=fopen(argv[1],"rb");assert(f);fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);unsigned char h[128],copy[128];assert(fread(h,1,128,f)==128);fclose(f);UpdatePackage p;assert(!update_package(h,size,&p));assert(p.total_size==(unsigned long)size);
 for(unsigned i=0;i<128;i++){memcpy(copy,h,128);copy[i]^=1;assert(update_package(copy,size,&p));}
 for(int delta=-128;delta<=128;delta++)if(delta)assert(update_package(h,size+delta,&p));
 memcpy(copy,h,128);copy[1]=32;crc(copy);assert(update_package(copy,size,&p));
 memcpy(copy,h,128);copy[2]=':';crc(copy);assert(update_package(copy,size,&p));
 memcpy(copy,h,128);copy[65]='B';crc(copy);assert(update_package(copy,size,&p));
 memcpy(copy,h,128);memset(copy+87,255,4);crc(copy);assert(update_package(copy,size,&p));
 struct sha256 ctx;unsigned char digest[32];static const unsigned char expected[32]={0xba,0x78,0x16,0xbf,0x8f,0x01,0xcf,0xea,0x41,0x41,0x40,0xde,0x5d,0xae,0x22,0x23,0xb0,0x03,0x61,0xa3,0x96,0x17,0x7a,0x9c,0xb4,0x10,0xff,0x61,0xf2,0x00,0x15,0xad};sha256_init(&ctx);sha256_update(&ctx,"a",1);sha256_update(&ctx,"bc",2);sha256_sum(&ctx,digest);assert(!memcmp(digest,expected,32));
 puts("PASS: current application package, every header byte corruption, truncation/trailing bytes, names/type/fork bounds, SHA-256 known vector");}
