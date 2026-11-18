#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../../src/update/release-tar.h"
static FILE *input;
static int read_at(void *c,unsigned long pos,void *p,unsigned n){(void)c;if(fseek(input,pos,SEEK_SET)||fread(p,1,n,input)!=n)return -39;return 0;}
static int member(void *c,unsigned i,unsigned long pos,unsigned long size,uint32_t expected){(void)c;(void)i;UpdateCRC32 crc;unsigned char b[4096];update_crc32_init(&crc);while(size){unsigned n=size>sizeof b?sizeof b:size;int e=read_at(0,pos,b,n);if(e)return e;update_crc32_bytes(&crc,b,n);size-=n;pos+=n;}return update_crc32_sum(&crc)==expected?0:-50;}
int main(int argc,char **argv){if(argc!=2)return 2;input=fopen(argv[1],"rb");if(!input)return 2;fseek(input,0,SEEK_END);long size=ftell(input);ReleaseArchive a={0};a.length=size;a.read=read_at;a.member=member;int e=release_archive(&a);fclose(input);if(e){fprintf(stderr,"invalid release archive: %d\n",e);return 1;}puts("PASS: native restricted tar reader verified all bundle members");return 0;}
