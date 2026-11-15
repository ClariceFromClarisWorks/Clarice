/* Test-only: common work either in CLX or a native MultiFinder sublaunch.
   A single fresh request beside the worker; never overwrite unrelated files. */
#include <Multiverse.h>
#include "common.h"
#include "../../src/update/native-files.h"
extern int resolve(const char *,char *),app_yield(void);
extern long cli_write(int,const void *,unsigned long);
extern unsigned long cli_ticks(void);
static void print(const char *s){cli_write(1,s,strlen(s));}
int clarice_main(int argc,char **argv){NBJob j;memset(&j,0,sizeof j);j.magic=NB_MAGIC;j.ticket=j.started=cli_ticks();strcpy(j.argument,"alpha beta");unsigned long start=j.started;int e=0;
 if(argc==2&&!strcmp(argv[1],"clx")){j.entered=cli_ticks();nb_body(&j);j.finished=cli_ticks();}
 else if(argc==3&&!strcmp(argv[1],"native")){
 if(NGetTrapAddress(0xA88F,1)==NGetTrapAddress(0xA89F,1)){print("MultiFinder required; single Finder remains CLX.\n");return 2;}
 char path[512],request[256];CInfoPBRec cat;if(resolve(argv[2],path)||strlen(path)>240||!strrchr(path,':'))return 2;
 if((e=update_info(path,&cat))||cat.hFileInfo.ioFlFndrInfo.fdType!='APPL')return 2;
 strcpy(request,path);strcpy(strrchr(request,':')+1,"NB Request");CInfoPBRec existing;e=update_info(request,&existing);if(e!=fnfErr){print("Remove prior test request explicitly before another run.\n");return 2;}
 unsigned char n[256];update_pstr(n,request);e=HCreate(0,0,n,'CLNB','DATA');short r=-1;long count=sizeof j;if(!e)e=HOpen(0,0,n,fsWrPerm,&r);if(!e)e=FSWrite(r,&count,&j);if(r>=0){int c=FSClose(r);if(!e)e=c;}if(e||count!=sizeof j)return 1;
 unsigned char appname[256];update_pstr(appname,path);short rf;FCBPBRec f;memset(&f,0,sizeof f);e=HOpenRF(0,0,appname,fsRdPerm,&rf);if(e)return 1;f.ioRefNum=rf;e=PBGetFCBInfoSync(&f);FSClose(rf);if(e)return 1;
 WDPBRec wd;memset(&wd,0,sizeof wd);wd.ioVRefNum=f.ioFCBVRefNum;wd.ioWDDirID=f.ioFCBParID;wd.ioWDProcID='CLNB';short previous;GetVol(0,&previous);e=PBOpenWDSync(&wd);if(e)return 1;short opened=wd.ioVRefNum;e=SetVol(0,opened);
 update_pstr(appname,strrchr(path,':')+1);
 struct __attribute__((packed,aligned(2))) {unsigned char *name;short param;char lc[2];long len;short flags;unsigned long options;} block={appname,0,{'L','C'},6,cat.hFileInfo.ioFlFndrInfo.fdFlags,0xc0000000UL};
 if(!e){register void *a0 __asm__("a0")=&block;register long d0 __asm__("d0");__asm__ volatile(".word 0xa9f2":"=d"(d0),"+a"(a0)::"d1","a1","cc","memory");e=(short)d0;if(e>=0)e=0;}
 SetVol(0,previous);wd.ioVRefNum=opened;PBCloseWDSync(&wd);
 while(!e&&cli_ticks()-start<1800){if(app_yield()){e=-128;break;}r=-1;e=HOpen(0,0,n,fsRdPerm,&r);count=sizeof j;if(!e)e=FSRead(r,&count,&j);if(r>=0)FSClose(r);if(e||count!=sizeof j){e=-36;break;}if(j.magic==NB_MAGIC+1&&j.ticket==start)break;}
 if(e||j.magic!=NB_MAGIC+1){char problem[100];snprintf(problem,sizeof problem,"native failed error=%d magic=%lx elapsed=%lu; request retained\n",e,(unsigned long)j.magic,cli_ticks()-start);print(problem);return e==-128?130:1;}
 HDelete(0,0,n);
 }else{print("nb clx | nb native /path/to/NBWorker\n");return 2;}
 cli_write(1,j.out,strlen(j.out));cli_write(2,j.err,strlen(j.err));char text[180];snprintf(text,sizeof text,"probe entry=%lu body=%lu return=%lu ticks; worker-heap=%lu status=%d\n",j.entered-start,j.finished-j.entered,cli_ticks()-start,(unsigned long)j.heap,j.status);print(text);return j.status;}
