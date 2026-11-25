/* Disposable Snow fixture driver. Not included in the distribution. */
#include "../../src/commands/mac-files.h"
#include <errno.h>
extern int cli_remove(const char *);
static int get(const char *path,char *native,CInfoPBRec *p){int e=resolve(path,native);return e?e:info(native,p);}
static int forks(const char*a,const char*b,int resource){short x=-1,y=-1;int e=cli_native_open(a,resource,0,&x);if(!e)e=cli_native_open(b,resource,0,&y);unsigned long offset=0;char p[1024],q[1024];
 while(!e){long n=sizeof p,m=sizeof q;int u=cli_native_read(x,p,&n),v=cli_native_read(y,q,&m);if((u&&u!=eofErr)||(v&&v!=eofErr)||n!=m){e=1;break;}
 for(long i=0;i<n;i++)if((!resource||offset+i<16||offset+i>=128)&&p[i]!=q[i]){e=1;break;}
 offset+=n;if(e||u==eofErr||v==eofErr||!n)break;if(app_yield()){e=130;break;}}
 if(x>=0)cli_native_close(x);if(y>=0)cli_native_close(y);return e;}
int clarice_main(int argc,char**argv){
 if(argc<3)return 2;if(!strcmp(argv[1],"unlink")){int e=cli_remove(argv[2]);if(e){char s[64];snprintf(s,sizeof s,"unlink errno=%d\n",errno);say(s);}return e?1:0;}
 char a[PATH_CAP],b[PATH_CAP],msg[200];CInfoPBRec x,y;int e=get(argv[2],a,&x);if(e)return 3;
 if(!strcmp(argv[1],"seed")){
  x.hFileInfo.ioFlCrDat=3700000000UL;x.hFileInfo.ioFlMdDat=3700000001UL;x.hFileInfo.ioFlBkDat=3700000002UL;
  x.hFileInfo.ioFlFndrInfo.fdFlags=0x4109;x.hFileInfo.ioFlFndrInfo.fdFldr=-2;x.hFileInfo.ioFlFndrInfo.fdLocation.h=123;x.hFileInfo.ioFlFndrInfo.fdLocation.v=77;
  unsigned char*v=(unsigned char*)&x.hFileInfo.ioFlXFndrInfo;for(int i=0;i<16;i++)v[i]=(unsigned char)(i+1);
  x.hFileInfo.ioFlXFndrInfo.fdIconID=321;x.hFileInfo.ioFlXFndrInfo.fdComment=234;x.hFileInfo.ioFlXFndrInfo.fdPutAway=345;
  return cli_native_set(a,&x)?4:0;
 }
 if(!strcmp(argv[1],"new"))return x.hFileInfo.ioFlFndrInfo.fdType=='TEXT'&&x.hFileInfo.ioFlFndrInfo.fdCreator=='CLRC'&&!x.hFileInfo.ioFlRLgLen?0:5;
 if(argc!=4||get(argv[3],b,&y))return 6;
 int edited=!strcmp(argv[1],"edited"),cross=!strcmp(argv[1],"cross");
 if(cross){x.hFileInfo.ioFlXFndrInfo.fdIconID=x.hFileInfo.ioFlXFndrInfo.fdComment=0;x.hFileInfo.ioFlXFndrInfo.fdPutAway=0;x.hFileInfo.ioFlFndrInfo.fdFlags&=~1;x.hFileInfo.ioFlFndrInfo.fdFldr=0;}
 if(!edited)x.hFileInfo.ioFlFndrInfo.fdFlags&=~0x100;
 if(memcmp(&x.hFileInfo.ioFlFndrInfo,&y.hFileInfo.ioFlFndrInfo,16)||memcmp(&x.hFileInfo.ioFlXFndrInfo,&y.hFileInfo.ioFlXFndrInfo,16)||x.hFileInfo.ioFlCrDat!=y.hFileInfo.ioFlCrDat||(!edited&&(x.hFileInfo.ioFlMdDat!=y.hFileInfo.ioFlMdDat||x.hFileInfo.ioFlBkDat!=y.hFileInfo.ioFlBkDat))){say("metadata mismatch\n");return 7;}
 if((!edited&&forks(a,b,0))||forks(a,b,1)){say("fork mismatch\n");return 8;}
 snprintf(msg,sizeof msg,"PASS %s forks/metadata, backup=%lu script=%u\n",argv[1],y.hFileInfo.ioFlBkDat,((unsigned char*)&y.hFileInfo.ioFlXFndrInfo)[8]);say(msg);return 0;
}
