/* Managed recovery inventory. Native files, not a second package format.
 * Only the final, CRC-checked inventory authorises rotation or recovery.
 * No inventory is inferred from filenames or historical .old/.previous files. */
#ifndef CLARICE_RECOVERY_RECORD_H
#define CLARICE_RECOVERY_RECORD_H
#include "repository.h"
#include "package.h"
#include "crc32.h"
#define RECOVERY_HEADER 544
#define RECOVERY_ENTRY 96
#define RECOVERY_CURRENT "Recovery Previous"
#define RECOVERY_PENDING "Recovery Preparing"
#define RECOVERY_RETIRED "Recovery Retiring"
#define RECOVERY_MANIFEST "Inventory"
#define RECOVERY_HELPER "Recover Clarice"
typedef struct {
 char name[24]; uint32_t data,resource,data_crc,resource_crc,created,modified,id,source_id;
 unsigned char finder[16],extra[16];
} RecoveryFile;
typedef struct {
 char root[256],origin[256];uint32_t build,count,origin_id,origin_modified,retired_id;
} RecoveryHeader;
static void recovery_put32(unsigned char *p,uint32_t n){p[0]=n>>24;p[1]=n>>16;p[2]=n>>8;p[3]=n;}
static int recovery_path(const char *s,int dir){
 size_t n=strlen(s);if(!n||n>255||s[0]==':'||(dir?(s[n-1]!=':'):(s[n-1]==':')))return 0;
 const char *start=s;int separators=0;
 for(const char *p=s;*p;p++){if((unsigned char)*p<32)return 0;if(*p==':'){if(p==start||p-start>31)return 0;start=p+1;separators++;}}
 return separators&&strlen(start)<=31;
}
static int recovery_name(const char *s){return !strcmp(s,"@core")||!strcmp(s,"@catalog")||!strcmp(s,"@helper")||repo_word(s,0);}
static int recovery_header_valid(const RecoveryHeader *h){
 size_t n=strlen(h->root);if(!recovery_path(h->root,1)||!recovery_path(h->origin,0)||n<13||strcmp(h->root+n-13,"Clarice Data:")||!h->build||!h->origin_id||h->count<2||h->count>(0x7fffffffUL-RECOVERY_HEADER-4)/RECOVERY_ENTRY)return 0;
 size_t v=strchr(h->root,':')-h->root;return (size_t)(strchr(h->origin,':')-h->origin)==v&&!memcmp(h->root,h->origin,v);
}
static int recovery_header_encode(const RecoveryHeader *h,unsigned char *b){
 if(!recovery_header_valid(h))return -50;memset(b,0,RECOVERY_HEADER);memcpy(b,"CLRSNAP1",8);memcpy(b+8,h->root,strlen(h->root));memcpy(b+264,h->origin,strlen(h->origin));
 recovery_put32(b+520,h->build);recovery_put32(b+524,h->count);recovery_put32(b+528,h->origin_id);recovery_put32(b+532,h->origin_modified);recovery_put32(b+536,h->retired_id);return 0;
}
static int recovery_header_decode(const unsigned char *b,RecoveryHeader *h){
 if(memcmp(b,"CLRSNAP1",8)||!memchr(b+8,0,256)||!memchr(b+264,0,256))return -50;
 for(unsigned i=540;i<RECOVERY_HEADER;i++)if(b[i])return -50;
 RecoveryHeader t;memset(&t,0,sizeof t);memcpy(t.root,b+8,256);memcpy(t.origin,b+264,256);t.build=update_be32(b+520);t.count=update_be32(b+524);t.origin_id=update_be32(b+528);t.origin_modified=update_be32(b+532);t.retired_id=update_be32(b+536);
 if(!recovery_header_valid(&t))return -50;*h=t;return 0;
}
static int recovery_file_encode(const RecoveryFile *f,unsigned char *b){
 if(!memchr(f->name,0,sizeof f->name)||!recovery_name(f->name)||(!f->id&&(!strcmp(f->name,"@core")||!strcmp(f->name,"@helper")))||f->data>0x7fffffffUL||f->resource>0x7fffffffUL)return -50;
 memset(b,0,RECOVERY_ENTRY);memcpy(b,f->name,strlen(f->name));
 recovery_put32(b+24,f->data);recovery_put32(b+28,f->resource);recovery_put32(b+32,f->data_crc);recovery_put32(b+36,f->resource_crc);recovery_put32(b+40,f->created);recovery_put32(b+44,f->modified);recovery_put32(b+48,f->id);memcpy(b+52,f->finder,16);memcpy(b+68,f->extra,16);recovery_put32(b+84,f->source_id);return 0;
}
static int recovery_file_decode(const unsigned char *b,RecoveryFile *f){
 if(!memchr(b,0,24))return -50;for(unsigned i=88;i<RECOVERY_ENTRY;i++)if(b[i])return -50;
 RecoveryFile t;memset(&t,0,sizeof t);memcpy(t.name,b,24);t.data=update_be32(b+24);t.resource=update_be32(b+28);t.data_crc=update_be32(b+32);t.resource_crc=update_be32(b+36);t.created=update_be32(b+40);t.modified=update_be32(b+44);t.id=update_be32(b+48);memcpy(t.finder,b+52,16);memcpy(t.extra,b+68,16);t.source_id=update_be32(b+84);
 unsigned char canonical[RECOVERY_ENTRY];if(recovery_file_encode(&t,canonical)||memcmp(b,canonical,sizeof canonical))return -50;*f=t;return 0;
}
/* namespace_install resolved the running System's actual volume and folder.
   Take its volume component; do not guess the name "System Folder". */
static int recovery_install_path(const char *root,char out[256]){
 if(!recovery_path(root,1))return -50;size_t n=strchr(root,':')-root+1;
 memcpy(out,root,n);strcpy(out+n,"Clarice");return 0;
}
static int recovery_member(const char *directory,const char *name,char out[256]){
 if(!recovery_path(directory,0)||!recovery_name(name))return -50;
 const char *leaf=!strcmp(name,"@core")?"Core":!strcmp(name,"@catalog")?"Catalog":!strcmp(name,"@helper")?RECOVERY_HELPER:name;
 if(strlen(directory)+strlen(leaf)+2>256)return -37;strcpy(out,directory);strcat(out,":");strcat(out,leaf);return 0;
}
#endif
