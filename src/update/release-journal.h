/* Foreground-only recovery record. Experimental version 2 binds the ordered
 * command inventory as well as file identities. Version 1 was never deployed.
 * CRC detects accidental damage, not malicious editing. No heap allocation. */
#ifndef CLARICE_RELEASE_JOURNAL_H
#define CLARICE_RELEASE_JOURNAL_H
#include "release-manifest.h"
#define RELEASE_JOURNAL_SIZE (556+64*(RELEASE_COUNT+1))
typedef struct {uint32_t id,modified,data,resource,created;} ReleaseIdentity;
typedef struct {
 char root[256],target[256];
 unsigned char identity[32];
 ReleaseIdentity old[RELEASE_COUNT+1],next[RELEASE_COUNT+1];
} ReleaseTransaction;
static void release_put32(unsigned char *p,uint32_t n){p[0]=n>>24;p[1]=n>>16;p[2]=n>>8;p[3]=n;}
static void release_encode_identity(unsigned char *p,const ReleaseIdentity *id){release_put32(p,id->id);release_put32(p+4,id->modified);release_put32(p+8,id->data);release_put32(p+12,id->resource);release_put32(p+16,id->created);}
static void release_decode_identity(const unsigned char *p,ReleaseIdentity *id){id->id=update_be32(p);id->modified=update_be32(p+4);id->data=update_be32(p+8);id->resource=update_be32(p+12);id->created=update_be32(p+16);}
static int release_journal_path(const char p[256],int directory){
 const char *end=memchr(p,0,256),*colon;
 if(!end||end==p)return -50;
 colon=memchr(p,':',end-p);
 if(!colon||colon==p||colon-p>31)return -50;
 if(directory?(end[-1]!=':'):(end[-1]==':'))return -50;
 /* Native absolute HFS paths, never relative-parent syntax. */
 const char *component=p;
 for(const char *q=p;q<end;q++)if(*q==':'){
  if(q==component||q-component>31)return -50;component=q+1;
 }
 if(end-component>31)return -50;
 return 0;
}
static int release_journal_check(const unsigned char *p,unsigned long size,
 const char *root,const char *target){
 if(size!=RELEASE_JOURNAL_SIZE||memcmp(p,"CLRTX002",8))return -50;
 UpdateCRC32 crc;update_crc32_init(&crc);update_crc32_bytes(&crc,p,size-4);
 if(update_crc32_sum(&crc)!=update_be32(p+size-4))return -50;
 if(release_journal_path((const char*)p+8,1)||release_journal_path((const char*)p+264,0))return -50;
 /* Caller supplies trusted installation paths, not paths from the record. */
 if(strcmp((const char*)p+8,root)||strcmp((const char*)p+264,target))return -50;
 for(unsigned i=0;i<=RELEASE_COUNT;i++){
  const unsigned char *r=p+552+64*i;
  const char *name=i==RELEASE_COUNT?"@core":release_names[i];
  unsigned n=strlen(name);
  if(n>=24||memcmp(r,name,n))return -50;
  for(unsigned j=n;j<24;j++)if(r[j])return -50;
  if(!update_be32(r+44))return -50; /* Every replacement must exist. */
  if(!update_be32(r+24))for(unsigned j=24;j<44;j++)if(r[j])return -50;
  if(!memcmp(r+24,r+44,20))return -50; /* Cannot classify old as new. */
 }
 return 0;
}
static int release_journal_encode(const ReleaseTransaction *t,unsigned char *p){
 if(release_journal_path(t->root,1)||release_journal_path(t->target,0))return -50;
 memset(p,0,RELEASE_JOURNAL_SIZE);memcpy(p,"CLRTX002",8);
 memcpy(p+8,t->root,strlen(t->root));memcpy(p+264,t->target,strlen(t->target));
 memcpy(p+520,t->identity,32);
 for(unsigned i=0;i<=RELEASE_COUNT;i++){
  const char *name=i==RELEASE_COUNT?"@core":release_names[i];
  if(strlen(name)>=24)return -50;
  unsigned char *r=p+552+i*64;memcpy(r,name,strlen(name));
  release_encode_identity(r+24,&t->old[i]);release_encode_identity(r+44,&t->next[i]);
 }
 UpdateCRC32 crc;update_crc32_init(&crc);update_crc32_bytes(&crc,p,RELEASE_JOURNAL_SIZE-4);
 release_put32(p+RELEASE_JOURNAL_SIZE-4,update_crc32_sum(&crc));
 return release_journal_check(p,RELEASE_JOURNAL_SIZE,t->root,t->target);
}
static int release_journal_decode(const unsigned char *p,unsigned long size,
 const char *root,const char *target,ReleaseTransaction *out){
 int e=release_journal_check(p,size,root,target);if(e)return e;
 /* No writes until the entire record is valid. Output may supply root/target. */
 memcpy(out->root,p+8,256);memcpy(out->target,p+264,256);memcpy(out->identity,p+520,32);
 for(unsigned i=0;i<=RELEASE_COUNT;i++){
  release_decode_identity(p+576+i*64,&out->old[i]);
  release_decode_identity(p+596+i*64,&out->next[i]);
 }
 return 0;
}
#endif
