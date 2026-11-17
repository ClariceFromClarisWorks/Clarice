#ifndef CLARICE_UPDATE_PLAN_H
#define CLARICE_UPDATE_PLAN_H
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "package.h"
/* Fixed, endian-independent plan. It is not a signature or an authorization
 * boundary against malicious local software. Only written after confirmation.
 * resource_crc excludes OS-owned recovery scratch bytes 16..127 only.
 * Target/stage/backup are siblings; the original catalog ID and modification
 * date bind the plan to the file the user reviewed. */
#define UPDATE_PLAN_SIZE 944
#define UPDATE_PLAN_NAME "Clarice Update.plan"
typedef struct {char target[256],stage[256],backup[256];uint32_t build,original_id,original_modified,data_size,resource_size,created,modified;uint32_t data_crc,resource_crc;unsigned char metadata[16],extra[16];} UpdatePlan;
static void update_put32(unsigned char *p,uint32_t n){p[0]=n>>24;p[1]=n>>16;p[2]=n>>8;p[3]=n;}
static int update_plan_path(const char *p){size_t n=strlen(p);const char *leaf=strrchr(p,':');if(!n||n>255||!leaf||leaf==p||!leaf[1]||strlen(leaf+1)>31||p[0]==':')return 0;for(size_t i=0;i<n;i++)if((unsigned char)p[i]<32)return 0;return 1;}
static int update_plan_valid(const UpdatePlan *p){
 if(!update_plan_path(p->target)||!update_plan_path(p->stage)||!update_plan_path(p->backup)||!p->build||!p->original_id||p->data_size>16777216UL||!p->resource_size||p->resource_size>16777216UL)return 0;
 const char *a=strrchr(p->target,':'),*b=strrchr(p->stage,':'),*c=strrchr(p->backup,':');size_t n=a-p->target;
 if((size_t)(b-p->stage)!=n||(size_t)(c-p->backup)!=n||memcmp(p->target,p->stage,n)||memcmp(p->target,p->backup,n))return 0;
 /* Fixed derived stage and backup leaves, never caller-chosen destinations. */
 char stage[32],backup[32];snprintf(stage,sizeof stage,".cu-%08lx-%08lx.new",(unsigned long)p->original_id,(unsigned long)p->build);snprintf(backup,sizeof backup,".cu-%08lx-%08lx.old",(unsigned long)p->original_id,(unsigned long)p->build);
 return !strcmp(b+1,stage)&&!strcmp(c+1,backup)&&strcmp(a+1,stage)&&strcmp(a+1,backup);
}
static int update_plan_encode(const UpdatePlan *p,unsigned char out[UPDATE_PLAN_SIZE]){
 if(!update_plan_valid(p))return -50;memset(out,0,UPDATE_PLAN_SIZE);memcpy(out,"CLUPLAN2",8);
 memcpy(out+8,p->target,strlen(p->target));memcpy(out+264,p->stage,strlen(p->stage));memcpy(out+520,p->backup,strlen(p->backup));
 update_put32(out+776,p->build);update_put32(out+780,p->original_id);update_put32(out+784,p->original_modified);update_put32(out+788,p->data_size);update_put32(out+792,p->resource_size);update_put32(out+796,p->data_crc);update_put32(out+828,p->resource_crc);memcpy(out+860,p->metadata,16);update_put32(out+876,p->created);update_put32(out+880,p->modified);memcpy(out+884,p->extra,16);
 unsigned c=update_crc(out,UPDATE_PLAN_SIZE-2);out[UPDATE_PLAN_SIZE-2]=c>>8;out[UPDATE_PLAN_SIZE-1]=c;return 0;
}
static int update_plan_decode(const unsigned char in[UPDATE_PLAN_SIZE],UpdatePlan *p){
 if(memcmp(in,"CLUPLAN2",8)||update_crc(in,UPDATE_PLAN_SIZE-2)!=((unsigned)in[UPDATE_PLAN_SIZE-2]<<8|in[UPDATE_PLAN_SIZE-1]))return -50;
 if(!memchr(in+8,0,256)||!memchr(in+264,0,256)||!memchr(in+520,0,256))return -50;
 memset(p,0,sizeof *p);memcpy(p->target,in+8,256);memcpy(p->stage,in+264,256);memcpy(p->backup,in+520,256);p->build=update_be32(in+776);p->original_id=update_be32(in+780);p->original_modified=update_be32(in+784);p->data_size=update_be32(in+788);p->resource_size=update_be32(in+792);p->data_crc=update_be32(in+796);p->resource_crc=update_be32(in+828);memcpy(p->metadata,in+860,16);p->created=update_be32(in+876);p->modified=update_be32(in+880);memcpy(p->extra,in+884,16);
 for(unsigned i=800;i<828;i++)if(in[i])return -50;
 for(unsigned i=832;i<860;i++)if(in[i])return -50;
 for(unsigned i=900;i<UPDATE_PLAN_SIZE-2;i++)if(in[i])return -50;
 return update_plan_valid(p)?0:-50;
}
#endif
