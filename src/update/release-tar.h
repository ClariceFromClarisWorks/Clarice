/* Restricted release envelope around upstream microtar. The source library stays
 * unchanged. Validate fixed fields before its sscanf/strcpy conversion; never
 * expose the general extractor to archive-supplied destination paths. */
#ifndef CLARICE_RELEASE_TAR_H
#define CLARICE_RELEASE_TAR_H
#include <limits.h>
#include "vendor/microtar/microtar.h"
#include "release-manifest.h"
typedef struct {
 void *context;unsigned long length;
 int (*read)(void *,unsigned long,void *,unsigned);
 int (*member)(void *,unsigned,unsigned long,unsigned long,uint32_t);
 unsigned char manifest[RELEASE_MANIFEST_SIZE];
 unsigned char header[512];unsigned long header_offset;
} ReleaseArchive;
static int release_octal(const unsigned char *p,unsigned n,unsigned long *out){
 unsigned long v=0;unsigned i=0;int digits=0;
 while(i<n&&p[i]==' ')i++;
 for(;i<n&&p[i]>='0'&&p[i]<='7';i++){if(v>(0xffffffffUL-(p[i]-'0'))/8)return -50;v=v*8+p[i]-'0';digits=1;}
 if(!digits)return -50;
 for(;i<n;i++)if(p[i]&&p[i]!=' ')return -50;
 *out=v;return 0;
}
static int release_tar_read(mtar_t *tar,void *p,unsigned n){ReleaseArchive *a=tar->stream;
 if(tar->pos>a->length||n>a->length-tar->pos)return -39;
 if(tar->pos==a->header_offset&&n==512){memcpy(p,a->header,512);return 0;}
 return a->read(a->context,tar->pos,p,n);
}
static int release_tar_seek(mtar_t *tar,unsigned n){return n>((ReleaseArchive*)tar->stream)->length?-39:0;}
static int release_tar_header(ReleaseArchive *a,unsigned long offset,mtar_header_t *h){
 if(offset>a->length||a->length-offset<512)return -39;
 int e=a->read(a->context,offset,a->header,512);if(e)return e;a->header_offset=offset;
 unsigned char *p=a->header;unsigned long n;
 if(!memchr(p,0,100)||!memchr(p+157,0,100)||p[156]!='0'||p[157]||memcmp(p+257,"ustar\00000",8)||p[345])return -50;
 if(release_octal(p+100,8,&n)||release_octal(p+108,8,&n)||release_octal(p+124,12,&n)||n>33554560UL||release_octal(p+136,12,&n)||release_octal(p+148,8,&n))return -50;
 mtar_t tar;memset(&tar,0,sizeof tar);tar.stream=a;tar.read=release_tar_read;tar.seek=release_tar_seek;tar.pos=offset;
 return mtar_read_header(&tar,h);
}
static int release_archive(ReleaseArchive *a){
 mtar_header_t h;unsigned long offset=0;int e;
 if((e=release_tar_header(a,offset,&h)))return e;
 if(strcmp(h.name,"Release")||h.size!=sizeof a->manifest)return -50;
 if((e=a->read(a->context,512,a->manifest,sizeof a->manifest)))return e;
 if((e=release_manifest_check(a->manifest,sizeof a->manifest)))return e;
 offset=512+((sizeof a->manifest+511)&~511UL);
 for(unsigned i=0;i<=RELEASE_COUNT;i++){
  if((e=release_tar_header(a,offset,&h)))return e;
  char name[100];unsigned long size;uint32_t crc;
  if(!i){strcpy(name,"Clarice.bin");size=update_be32(a->manifest+48);crc=update_be32(a->manifest+52);}
  else{strcpy(name,"Utilities/");strcat(name,release_names[i-1]);size=update_be32(a->manifest+56+(i-1)*32+24);crc=update_be32(a->manifest+56+(i-1)*32+28);}
  if(strcmp(h.name,name)||h.size!=size||offset+512>a->length||size>a->length-offset-512)return -50;
  if((e=a->member(a->context,i,offset+512,size,crc)))return e;
  offset+=512+((size+511)&~511UL);
 }
 /* Reject extra members and truncated end records; tar block padding is zero. */
 if(offset>a->length||a->length-offset<1024||(a->length-offset)%512)return -50;
 while(offset<a->length){if((e=a->read(a->context,offset,a->header,512)))return e;for(unsigned i=0;i<512;i++)if(a->header[i])return -50;offset+=512;}
 return 0;
}
#endif
