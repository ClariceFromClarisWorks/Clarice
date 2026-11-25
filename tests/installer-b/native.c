/* Reuse the native HFS fault model and the actual recovery implementation. */
#define main previous_recovery_main
#include "../recovery/native.c"
#undef main
static const char *test_system_path="Disk:System:";
static int system_path(char *p){strcpy(p,test_system_path);return 0;}
#define LOCATIONS_SYSTEM_PATH system_path
#define CLARICE_OFFLINE_INSTALLER 1
static int HCreate(short v,long d,const unsigned char *p,unsigned long creator,unsigned long type){(void)v;(void)d;char s[256];memcpy(s,p+1,p[0]);s[p[0]]=0;return cli_native_create(s,type,creator);}
#include "../../src/update/locations-native.h"
static void finish_handles(void){for(int i=0;i<32;i++)assert(!refs[i].used);assert(!allocs);}
static void initial_locations(void){memset(fs,0,sizeof fs);memset(refs,0,sizeof refs);boundaries=fail_at=crash_at=fail_alloc=cancel_poll=0;fault_code=ioErr;}
static int custom_snapshot(RecoveryRotation *rotation){
 RecoveryFile f[4]={0};const char *names[]={"@core","@helper","@catalog","ls"};for(int i=0;i<4;i++)strcpy(f[i].name,names[i]);
 RecoverySnapshot s={0};strcpy(s.header.root,"Disk:Files:Clarice Data:");strcpy(s.header.origin,"Disk:Apps:Clarice");s.header.build=1947;s.header.format=3;s.header.count=4;s.header.origin_id=fs[find(s.header.origin)].info.hFileInfo.ioDirID;s.header.origin_modified=77;s.files=f;
 return recovery_prepare(rotation,&s,"Disk:Helper",0);
}
#ifndef INSTALLER_C_REUSE
int main(void){
 InstallLocations p={"Disk:Apps:Clarice","Disk:Files:Clarice Data:"},got;
 initial_locations();assert(!locations_load(&got));assert(!strcmp(got.core,"Disk:Clarice")&&!strcmp(got.data,"Disk:System:Clarice Data:"));assert(!boundaries||find("Disk:System:Clarice Locations")<0);
 boundaries=0;assert(!locations_write(&p));int total=boundaries;finish_handles();assert(!locations_read(&got));assert(!memcmp(&p,&got,sizeof p));char target[256];assert(!locations_core(p.data,target)&&!strcmp(target,p.core));assert(locations_core("Disk:Other:Clarice Data:",target)==paramErr);
 for(int disk=0;disk<2;disk++)for(int n=1;n<=total;n++){
  initial_locations();fault_code=disk?-34:ioErr;fail_at=n;(void)locations_write(&p);fail_at=0;finish_handles();assert(!locations_write(&p));assert(!locations_read(&got)&&!memcmp(&p,&got,sizeof p));finish_handles();
 }
 for(int n=1;n<=total;n++){
  initial_locations();crash_at=n;if(!setjmp(reset_point))locations_write(&p);process_reset();assert(!locations_write(&p));assert(!locations_read(&got)&&!memcmp(&p,&got,sizeof p));finish_handles();
 }
 /* Interrupted prefix and foreign/changed records never authorize relocation. */
 initial_locations();assert(!cli_native_create("Disk:System:Clarice Locations.new",'DATA','CLRC'));unsigned char bytes[LOCATION_BYTES];assert(!locations_encode(&p,bytes));int ix=find("Disk:System:Clarice Locations.new");memcpy(fs[ix].fork[0],bytes,90);fs[ix].info.hFileInfo.ioFlLgLen=90;assert(!locations_write(&p));
 strcpy(got.core,"Disk:Elsewhere:Clarice");assert(locations_write(&got)==paramErr);
 ix=find("Disk:System:Clarice Locations");fs[ix].fork[0][40]^=1;assert(locations_load(&got)==paramErr);finish_handles();
 printf("PASS authoritative locations: %d I/O + disk-full + interruption boundaries; exact retry; partial stage; invalid record fails closed; immutable installed destinations\n",total);
 initial_locations();
 file("Disk:Clarice","root sentinel");file("Disk:Apps:Clarice","custom old core");file("Disk:Helper","helper");file("Disk:Files:Clarice Data:etc:clarice:installed.catalog","CLARICE-INSTALLED 1\n");file("Disk:Files:Clarice Data:bin:ls","old ls");file("Disk:Files:Clarice Data:home:notes","user notes");
 RecoveryRotation rotation={{"Disk:Files:Clarice Data:etc:clarice:Recovery Previous","Disk:Files:Clarice Data:etc:clarice:Recovery Preparing","Disk:Files:Clarice Data:etc:clarice:Recovery Retiring"}};
 assert(!custom_snapshot(&rotation));RecoverySnapshot s;assert(!recovery_inventory(rotation.path[0],&s,0)&&s.header.format==3);assert(!recovery_verify(rotation.path[0],&s));recovery_dispose(&s);
 fs[find("Disk:Apps:Clarice")].fork[0][0]^=1;fs[find("Disk:Files:Clarice Data:bin:ls")].fork[1][0]^=1;memcpy(baseline,fs,sizeof fs);
 char launch[256];boundaries=0;assert(!recovery_restore(rotation.path[0],launch));total=boundaries;assert(!strcmp(launch,"Disk:Apps:Clarice"));finish_handles();
 for(int kind=0;kind<3;kind++)for(int n=1;n<=total;n++){
  reset();if(kind==2){crash_at=n;if(!setjmp(reset_point))recovery_restore(rotation.path[0],launch);process_reset();}else{fault_code=kind?-34:ioErr;fail_at=n;(void)recovery_restore(rotation.path[0],launch);fail_at=0;finish_handles();}
  assert(!recovery_restore(rotation.path[0],launch));finish_handles();assert(!strcmp(launch,"Disk:Apps:Clarice"));
  assert(!strcmp((char*)fs[find("Disk:Clarice")].fork[0],"root sentinel"));
  assert(!strcmp((char*)fs[find("Disk:Files:Clarice Data:home:notes")].fork[0],"user notes"));
  assert(!memcmp(fs[find("Disk:Files:Clarice Data:home:notes")].fork[1],"resource!",9));
 }
 printf("PASS custom V3 Core/data restore: %d I/O + disk-full + interruption boundaries; both forks, catalog, root sibling, user files and exact launch target preserved\n",total);
}

#endif
