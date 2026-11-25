#define main previous_recovery_main
#include "../recovery/native.c"
#undef main
#include "../../src/update/release-manifest.h"
#define PATH_CAP 512
#define fsRdWrPerm 3
static char root_path[256]="Disk:Data:Clarice Data:";
static int replaced,polled;
static int app_yield(void){polled++;return cancel_poll;}
static void errsay(const char *p){(void)p;}
static void app_install_progress(const char*p,unsigned a,unsigned b){(void)p;(void)a;(void)b;}
#define info(p,c) update_info(p,c)
#define pstr update_pstr
static int cli_native_open(const char*p,int fork,int write,short*r){return update_open(p,fork,write,r);}
static int HOpen(short v,long d,const unsigned char*p,int perm,short*r){(void)v;(void)d;char path[256];memcpy(path,p+1,p[0]);path[p[0]]=0;return update_open(path,0,perm==fsRdWrPerm,r);}
static int SetEOF(short r,long n){int e=boundary();if(e)return e;assert(n>=0&&n<=8192);fs[refs[r].file].info.hFileInfo.ioFlLgLen=n;return 0;}
static int SetFPos(short r,int mode,long n){assert(mode==fsFromStart);int e=boundary();if(!e)refs[r].pos=n;return e;}
static int command_paths(const char*n,char*d,char*s,char*b){snprintf(d,PATH_CAP,"%sbin:%s",root_path,n);strcpy(s,d);strcpy(b,d);return 0;}
static int update_hash_fork(const char*p,int fork,uint32_t*c,uint32_t n){return recovery_fork(p,0,fork,n,c,0);}
static int command_verify_native(const char*p){return find(p)<0?fnfErr:0;}
static int command_install_resume(const char *source,const char*name,int managed){(void)source;(void)name;(void)managed;replaced++;return 0;}
#include "../../src/offline-maintenance.inc"
static void catalog(const char *body){const char*p="Disk:Data:Clarice Data:etc:clarice:installed.catalog";int i=find(p);if(i<0){assert(!cli_native_create(p,'TEXT','CLRC'));i=find(p);}memcpy(fs[i].fork[0],body,strlen(body));fs[i].info.hFileInfo.ioFlLgLen=strlen(body);}
int main(void){
 unsigned char manifest[RELEASE_MANIFEST_SIZE]={0};recovery_put32(manifest+8,1977);char path[512];
 for(unsigned i=0;i<RELEASE_COUNT;i++){
  snprintf(path,sizeof path,"Disk:Kit:%s",release_names[i]);assert(!cli_native_create(path,'BINA','UNIX'));int f=find(path);memset(fs[f].fork[0],i+1,40);fs[f].info.hFileInfo.ioFlLgLen=40;
  uint32_t crc;assert(!update_hash_fork(path,0,&crc,40));recovery_put32(manifest+56+i*32+24,40);recovery_put32(manifest+56+i*32+28,crc);
  snprintf(path,sizeof path,"%sbin:%s",root_path,release_names[i]);assert(!cli_native_create(path,'CLX1','CLRC'));f=find(path);memset(fs[f].fork[0],i+1,40);fs[f].info.hFileInfo.ioFlLgLen=40;
 }
 memcpy(baseline,fs,sizeof fs);
 OfflineTool tools[RELEASE_COUNT]={0};uint32_t core;int changes;
 assert(!offline_catalog_read(tools,&core)&&!core);assert(!offline_tools_plan("Disk:Kit",manifest,tools,0,&changes)&&!changes);
 assert(!offline_tools_install("Disk:Kit",manifest,tools,0)&&!replaced);memset(tools,0,sizeof tools);assert(!offline_catalog_read(tools,&core));for(unsigned i=0;i<RELEASE_COUNT;i++)assert(tools[i].known&&tools[i].version==1977);
 snprintf(path,sizeof path,"%sbin:%s",root_path,release_names[0]);int f=find(path);
 OfflineTool previous=tools[0];fs[f].fork[0][0]^=7;tools[0].version=1978;
 assert(!update_hash_fork(path,0,&tools[0].crc,40));
 assert(!offline_tools_plan("Disk:Kit",manifest,tools,0,&changes)&&!changes&&!tools[0].needed);
 assert(!offline_tools_install("Disk:Kit",manifest,tools,0)&&!replaced);
 fs[f].fork[0][0]^=7;tools[0]=previous;
 puts("PASS verified newer compatible utility preserved by ordinary offline upgrade");
 fs[f].fork[0][0]^=1;
 assert(offline_tools_plan("Disk:Kit",manifest,tools,0,&changes)==paramErr&&!replaced);
 assert(!offline_tools_plan("Disk:Kit",manifest,tools,1,&changes)&&changes==1&&tools[0].needed);
 fs[f].fork[0][0]^=1;
 tools[0].known=0;fs[f].fork[0][1]^=1;assert(offline_tools_plan("Disk:Kit",manifest,tools,0,&changes)==paramErr);fs[f].fork[0][1]^=1;
 unsigned saved=fs[f].info.hFileInfo.ioFlLgLen;fs[f].path[0]=0;assert(!offline_tools_plan("Disk:Kit",manifest,tools,0,&changes)&&changes==1);strcpy(fs[f].path,path);fs[f].info.hFileInfo.ioFlLgLen=saved;
 snprintf(path,sizeof path,"Disk:Kit:%s",release_names[2]);fs[find(path)].fork[0][0]^=1;assert(offline_tools_plan("Disk:Kit",manifest,tools,0,&changes)==paramErr&&!replaced);fs[find(path)].fork[0][0]^=1;
 catalog("CLARICE-INSTALLED 1\ntool\tls\t1977\t40\t0000");assert(!offline_catalog_read(tools,&core)&&offline_catalog_end==20);
 catalog("CLARICE-INST");assert(!offline_catalog_read(tools,&core)&&!offline_catalog_end);short ref;assert(!offline_catalog_open(&ref));assert(!FSClose(ref));assert(!offline_catalog_read(tools,&core));
 /* A matching data image with damaged Finder codes is still repaired;
  * preserve unrelated Finder fields while restoring executable identity. */
 snprintf(path,sizeof path,"%sbin:%s",root_path,release_names[0]);f=find(path);
 fs[f].info.hFileInfo.ioFlFndrInfo.fdType='TEST';fs[f].info.hFileInfo.ioFlFndrInfo.fdCreator='USER';
 memset(tools,0,sizeof tools);assert(!offline_catalog_read(tools,&core));
 assert(!offline_tools_plan("Disk:Kit",manifest,tools,1,&changes)&&changes==1);
 assert(!offline_tools_install("Disk:Kit",manifest,tools,1));
 assert(fs[f].info.hFileInfo.ioFlFndrInfo.fdType=='CLX1'&&fs[f].info.hFileInfo.ioFlFndrInfo.fdCreator=='CLRC');
 puts("PASS explicit repair restores damaged executable Finder codes");
 const char *marker="Disk:Data:Clarice Data:etc:clarice:update.incomplete";
 assert(!offline_pending());assert(!cli_native_create(marker,'TEXT','CLRC'));int m=find(marker);char text[128];
 offline_manifest_crc=0x12345678;int len=repo_marker_header(text,sizeof text,0,RELEASE_MANIFEST_SIZE,offline_manifest_crc);assert(len>0);
 memcpy(fs[m].fork[0],text,len);fs[m].info.hFileInfo.ioFlLgLen=len;assert(!offline_pending());
 offline_manifest_crc++;assert(offline_pending()==paramErr);offline_manifest_crc--;
 memcpy(fs[m].fork[0],"CLARICE-UPDATE-INCOMPLETE 1\n",28);fs[m].info.hFileInfo.ioFlLgLen=28;assert(!offline_pending());
 fs[m].path[0]=0;puts("PASS production offline retry accepts unchanged v2/v1 marker grammar and rejects another kit");

 cancel_poll=1;assert(offline_catalog_read(tools,&core)==userCanceledErr);assert(offline_tools_install("Disk:Kit",manifest,tools,0)==userCanceledErr);cancel_poll=0;
 for(int i=0;i<32;i++)assert(!refs[i].used);assert(!allocs);
 cancel_poll=0;
 reset();memset(tools,0,sizeof tools);assert(!offline_catalog_read(tools,&core));boundaries=0;assert(!offline_tools_install("Disk:Kit",manifest,tools,0));int steps=boundaries;
 for(int kind=0;kind<2;kind++)for(int fault=1;fault<=steps;fault++){
  reset();memset(tools,0,sizeof tools);assert(!offline_catalog_read(tools,&core));boundaries=0;fault_code=kind?-34:ioErr;fail_at=fault;(void)offline_tools_install("Disk:Kit",manifest,tools,0);fail_at=0;
  for(int j=0;j<32;j++)assert(!refs[j].used);
  memset(tools,0,sizeof tools);assert(!offline_catalog_read(tools,&core));assert(!offline_tools_install("Disk:Kit",manifest,tools,0));
  memset(tools,0,sizeof tools);assert(!offline_catalog_read(tools,&core));for(unsigned j=0;j<RELEASE_COUNT;j++)assert(tools[j].known&&tools[j].version==1977);
 }
 printf("PASS catalog adoption retry: %d I/O and disk-full boundaries; all 49 records complete, no leaked handles\n",steps);
 puts("PASS offline policy: exact legacy adoption, full catalog, ordinary no-op, modified/unknown preservation, explicit repair plan, missing utility, damaged kit, torn catalog/header retry, cancellation and closed files");
}
