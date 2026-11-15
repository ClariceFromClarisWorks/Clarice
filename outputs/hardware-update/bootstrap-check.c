/* One-off physical bootstrap diagnosis. Not a shipped command. */
#include <stdio.h>
#include <Files.h>
#include <string.h>
/* Fixed ASCII fixture volume; avoid an import absent in the old bootstrap. */
static int fixture_equal(const unsigned char*a,const unsigned char*b,int c,int d){(void)c;(void)d;return a[0]==b[0]&&!memcmp(a+1,b+1,a[0]);}
#define EqualString fixture_equal
#include "../../src/update/stamp.h"
extern int cli_native_open(const char*,int,int,short*);
#define UPDATE_OPEN_RESOURCE(path,ref) cli_native_open(path,1,0,ref)
#include "../../src/update/open-guard.h"
extern int cli_native_set(const char*,CInfoPBRec*);
extern long cli_write(int,const void*,long);
int clarice_main(int argc,char **argv){
 const char *path="Macintosh HD:Clarice Update";CInfoPBRec cat;uint32_t build=0;unsigned sys=0;int busy=-1,e=update_info(path,&cat);if(e)return 1;
 int stamp=update_stamp(path,&build,&sys),guard=update_open_guard(path,&busy);char s[200];int n=snprintf(s,sizeof s,"bootstrap: stamp=%d build=%lu system=%04x guard=%d busy=%d data=%ld resource=%ld flags=%04x\n",stamp,(unsigned long)build,sys,guard,busy,cat.hFileInfo.ioFlLgLen,cat.hFileInfo.ioFlRLgLen,cat.hFileInfo.ioFlFndrInfo.fdFlags);cli_write(1,s,n);
 if(argc==2&&!strcmp(argv[1],"bundle")){
  if(stamp||guard||busy||build!=2026111402UL||cat.hFileInfo.ioFlLgLen||cat.hFileInfo.ioFlRLgLen!=941423||cat.hFileInfo.ioFlFndrInfo.fdType!='APPL'||cat.hFileInfo.ioFlFndrInfo.fdCreator!='CLRC')return 1;
  cat.hFileInfo.ioFlFndrInfo.fdFlags=(cat.hFileInfo.ioFlFndrInfo.fdFlags|0x2000)&~0x0100;e=cli_native_set(path,&cat);if(!e){ParamBlockRec flush;memset(&flush,0,sizeof flush);flush.volumeParam.ioVRefNum=cat.hFileInfo.ioVRefNum;e=PBFlushVolSync(&flush);}n=snprintf(s,sizeof s,"restore package bundle flag: %d\n",e);cli_write(1,s,n);return e?1:0;
 }return stamp||guard?1:0;
}
