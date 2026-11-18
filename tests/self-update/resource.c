/* Native-resource lifecycle shim; actual entry point, not guest performance. */
#include <assert.h>
#include <string.h>
typedef char **Handle;
static short self_update_resources=7,current=3;
static int session_io,missing,locked,released,called;
static char bytes[64],*data=bytes;
static short CurResFile(void){return current;}
static void UseResFile(short n){current=n;}
static Handle Get1Resource(long type,short id){assert(type=='CLXU'&&id==128&&current==7);return missing?0:&data;}
static void HLock(Handle h){assert(h==&data);locked++;}
static void HUnlock(Handle h){assert(h==&data&&locked==1);locked--;}
static long GetHandleSize(Handle h){assert(h==&data);return sizeof bytes;}
static void ReleaseResource(Handle h){assert(h==&data&&!locked);released++;}
static void errsay(const char *s){(void)s;}
static int module_load_source(const char *p,int argc,char **argv,int run,unsigned char *b,long size){
 assert(!strcmp(p,"built-in clarice")&&run&&b==(unsigned char*)bytes&&size==64&&locked&&current==3);
 assert(argc==2&&argv);called++;return 130;
}
#include "../../src/self-update.inc"
int main(void){
 char *av[]={"clarice","--help",0};
 assert(self_update_command(2,av)==130&&called==1&&released==1&&!locked&&current==3);
 missing=1;assert(self_update_command(2,av)==126&&called==1&&released==1&&current==3);missing=0;
 data=0;assert(self_update_command(2,av)==126&&released==2&&!locked);data=bytes;
 session_io=1;av[1]="update";assert(self_update_command(2,av)==2&&called==1);
 av[1]="--help";assert(self_update_command(2,av)==130&&called==2&&released==3&&!locked);
 return 0;
}
