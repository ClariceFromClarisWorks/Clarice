#define INSTALLER_C_REUSE 1
/* The B fixture imports the existing fork-preserving native File Manager model. */
#include "../installer-b/native.c"
#undef main
#include "../../src/update/repository.h"
#include "../../src/update/catalog-reader.h"
static int yield_count,cancel_after;
static int app_yield(void){return cancel_poll||(cancel_after&&++yield_count==cancel_after);}
static void app_install_progress(const char *p,unsigned n,unsigned all){(void)p;(void)n;(void)all;}
static int update_application_path(char *p){strcpy(p,"Disk:Kit:Installer");return 0;}
static int EqualString(const unsigned char *a,const unsigned char*b,int c,int d){(void)c;(void)d;return a[0]==b[0]&&!strncasecmp((const char*)a+1,(const char*)b+1,a[0]);}
static int idfile(long id){for(int i=0;i<100;i++)if(fs[i].path[0]&&fs[i].info.hFileInfo.ioDirID==id)return i;return -1;}
static int parentof(int i){char p[256];strcpy(p,fs[i].path);*strrchr(p,':')=0;return find(p);}
static int PBGetCatInfoSync(CInfoPBRec *c){
 int e=boundary();if(e)return e;unsigned char *name=c->hFileInfo.ioNamePtr;int i=-1;
 if(c->dirInfo.ioFDirIndex==-1){name=c->dirInfo.ioNamePtr;i=idfile(c->dirInfo.ioDrDirID);}
 else {int nth=c->hFileInfo.ioFDirIndex;for(int j=0;j<100;j++)if(fs[j].path[0]){int p=parentof(j);if(p>=0&&fs[p].info.hFileInfo.ioDirID==c->hFileInfo.ioDirID&&!--nth){i=j;break;}}}
 if(i<0)return fnfErr;*c=fs[i].info;int p=parentof(i);c->dirInfo.ioDrDirID=c->hFileInfo.ioDirID;c->dirInfo.ioDrParID=p<0?2:fs[p].info.hFileInfo.ioDirID;
 const char *leaf=strrchr(fs[i].path,':')+1;update_pstr(name,leaf);return 0;
}
static int HDelete(short v,long id,const unsigned char *name){(void)v;int p=idfile(id);if(p<0)return paramErr;char path[256];strcpy(path,fs[p].path);strcat(path,":");size_t n=strlen(path);memcpy(path+n,name+1,name[0]);path[n+name[0]]=0;return cli_native_delete(path);}
#include "../../src/offline-uninstall.inc"
static InstallLocations selected={"Disk:Apps:Clarice","Disk:Files:Clarice Data:"};
static void seed(void){
 initial_locations();assert(!cli_native_mkdir("Disk:Files:Clarice Data"));assert(!cli_native_mkdir("Disk:Files:Clarice Data:etc"));assert(!cli_native_mkdir("Disk:Files:Clarice Data:etc:clarice"));assert(!cli_native_mkdir("Disk:Files:Clarice Data:home"));
 file(selected.core,"Core");fs[find(selected.core)].info.hFileInfo.ioFlFndrInfo.fdType='APPL';
 file("Disk:Files:Clarice Data:etc:clarice:installed.catalog","CLARICE-INSTALLED 1\ncore\tClarice\t1947\t0\t00000000\n");fs[find("Disk:Files:Clarice Data:etc:clarice:installed.catalog")].info.hFileInfo.ioFlRLgLen=0;
 const char *names[]={".","..","/","keep/","%2E","link"};for(unsigned i=0;i<6;i++){char p[256];snprintf(p,sizeof p,"Disk:Files:Clarice Data:home:%s",names[i]);file(p,"user");}
 file("Disk:Files:outside","sentinel");file("Disk:Apps:Clarice Previous","historical");assert(!locations_write(&selected));boundaries=0;
}
static void untouched(void){assert(find("Disk:Files:outside")>=0);assert(!memcmp(&fs[find("Disk:Files:outside")],&baseline[find("Disk:Files:outside")],sizeof fs[0]));assert(find("Disk:Apps:Clarice Previous")>=0);finish_handles();}
int main(void){
 seed();memcpy(baseline,fs,sizeof fs);char core[256],data[256];assert(!shell_uninstall_locations(core,data));assert(!strcmp(core,selected.core)&&!strcmp(data,selected.data));assert(!memcmp(baseline,fs,sizeof fs));
 assert(!shell_uninstall(core,data,0));assert(find(core)<0);for(int i=0;i<100;i++)if(strstr(baseline[i].path,"Clarice Data"))assert(!memcmp(&baseline[i],&fs[i],sizeof fs[i]));untouched();
 seed();memcpy(baseline,fs,sizeof fs);assert(!shell_uninstall(selected.core,selected.data,1));int total=boundaries;assert(find("Disk:Files:Clarice Data")<0&&find("Disk:System:Clarice Locations")<0);untouched();
 for(int kind=0;kind<3;kind++)for(int n=1;n<=total;n++){
  reset();if(kind==2){crash_at=n;if(!setjmp(reset_point))shell_uninstall(selected.core,selected.data,1);process_reset();}else {fault_code=kind?-34:ioErr;fail_at=n;(void)shell_uninstall(selected.core,selected.data,1);fail_at=0;}finish_handles();
  if(find("Disk:System:Clarice Locations")>=0){int e=shell_uninstall(selected.core,selected.data,1);if(e)fprintf(stderr,"retry kind=%d boundary=%d error=%d\n",kind,n,e);assert(!e);}
  assert(find("Disk:Files:Clarice Data")<0);untouched();
 }
 for(int n=1;n<=20;n++){reset();cancel_after=n;yield_count=0;(void)shell_uninstall(selected.core,selected.data,1);cancel_after=0;if(find("Disk:System:Clarice Locations")>=0)assert(!shell_uninstall(selected.core,selected.data,1));untouched();}
 seed();int lock=find("Disk:Files:Clarice Data:home:..");fs[lock].info.hFileInfo.ioFlAttrib|=1;assert(shell_uninstall(selected.core,selected.data,1)==permErr);assert(find("Disk:Files:Clarice Data:etc:clarice:installed.catalog")>=0);fs[lock].info.hFileInfo.ioFlAttrib&=~1;assert(!shell_uninstall(selected.core,selected.data,1));
 seed();memcpy(baseline,fs,sizeof fs);cancel_poll=1;assert(shell_uninstall(selected.core,selected.data,1)==userCanceledErr);assert(!memcmp(baseline,fs,sizeof fs));cancel_poll=0;assert(!shell_uninstall(selected.core,selected.data,1));
 seed();fs[find("Disk:System:Clarice Locations")].fork[0][20]^=1;memcpy(baseline,fs,sizeof fs);assert(shell_uninstall(selected.core,selected.data,1)==paramErr);assert(!memcmp(baseline,fs,sizeof fs));
 seed();assert(!cli_native_delete("Disk:Files:Clarice Data:etc:clarice:installed.catalog"));assert(shell_uninstall(selected.core,selected.data,1));assert(find(selected.core)>=0);
 seed();fs[find(selected.core)].info.hFileInfo.ioFlFndrInfo.fdCreator='OTHR';assert(shell_uninstall(selected.core,selected.data,0)==paramErr);
 seed();assert(!cli_native_delete(selected.core));assert(!cli_native_delete("Disk:Files:Clarice Data:etc:clarice:installed.catalog"));assert(shell_uninstall(selected.core,selected.data,1)==paramErr);assert(find("Disk:Files:Clarice Data:home:..")>=0);
 /* Legacy no-record installation writes the same authoritative record only
    after confirmation; interrupted final cleanup can then be retried. */
 seed();assert(!cli_native_delete("Disk:System:Clarice Locations"));
 for(int i=0;i<100;i++)if(fs[i].path[0]){char q[256];strcpy(q,fs[i].path);if(!strncmp(q,"Disk:Files:Clarice Data",23))snprintf(fs[i].path,256,"Disk:System:Clarice Data%s",q+23);else if(!strcmp(q,selected.core))strcpy(fs[i].path,"Disk:Clarice");}
 assert(!shell_uninstall_locations(core,data));assert(!strcmp(core,"Disk:Clarice"));assert(!shell_uninstall(core,data,0));assert(find("Disk:System:Clarice Locations")>=0);assert(!shell_uninstall(core,data,1));
 seed();file("Disk:Files:Clarice Data:etc:clarice:update.incomplete","in progress");memcpy(baseline,fs,sizeof fs);assert(shell_uninstall(selected.core,selected.data,1)==fBsyErr);assert(!memcmp(baseline,fs,sizeof fs));
 seed();test_system_path="Disk:Files:Clarice Data:home:";strcpy(fs[find("Disk:System:Clarice Locations")].path,"Disk:Files:Clarice Data:home:Clarice Locations");memcpy(baseline,fs,sizeof fs);assert(shell_uninstall(selected.core,selected.data,1)==fBsyErr);assert(!memcmp(baseline,fs,sizeof fs));test_system_path="Disk:System:";
 finish_handles();printf("PASS native uninstall: app-only preservation; full literal native tree; siblings/forks/metadata; %d I/O/disk-full/interruption retry boundaries; locks, cancellation, invalid/missing authority, foreign Core; zero allocations\n",total);
}
