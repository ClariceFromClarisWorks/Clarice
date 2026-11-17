/* Test-only, absent-only deployment of the embedded native benchmark worker.
 * Reuses the production MacBinary parser/extractor; never replaces a file. */
#include <Multiverse.h>
#include "../../src/update/extract.h"
#include "../../src/update/native-files.h"
#include "worker-image.h"
extern int resolve(const char *,char *),app_yield(void);
extern long cli_write(int,const void *,unsigned long);
typedef struct {unsigned offset;short out;char path[512];} State;
static int seek_image(void *v,uint32_t n){State *s=v;if(n>sizeof worker_image)return eofErr;s->offset=n;return 0;}
static int read_image(void *v,void *p,unsigned n){State *s=v;if(n>sizeof worker_image-s->offset)return eofErr;memcpy(p,worker_image+s->offset,n);s->offset+=n;return 0;}
static int begin(void *v,int resource){State *s=v;unsigned char p[256];update_pstr(p,s->path);return resource?HOpenRF(0,0,p,fsWrPerm,&s->out):HOpen(0,0,p,fsWrPerm,&s->out);}
static int write_image(void *v,const void *p,unsigned n){long count=n;int e=FSWrite(((State*)v)->out,&count,p);return e?e:count==n?0:ioErr;}
static int end(void *v){State *s=v;int e=FSClose(s->out);s->out=-1;return e;}
static int poll(void *v){(void)v;return app_yield()?userCanceledErr:0;}
int clarice_main(int argc,char **argv){(void)argv;if(argc!=1)return 2;State s;memset(&s,0,sizeof s);s.out=-1;UpdatePackage p;int e=update_package_kind(worker_image,sizeof worker_image,&p,"CLNB");if(e)return 2;if(resolve("/tmp/NBWorker",s.path))return 2;CInfoPBRec cat;e=update_info(s.path,&cat);if(e!=fnfErr){const char *msg="NBWorker already exists or cannot be inspected; unchanged.\n";cli_write(2,msg,strlen(msg));return 2;}
 unsigned char name[256];update_pstr(name,s.path);e=HCreate(0,0,name,'CLNB','APPL');if(e)return 1;
 UpdateExtractOps ops={&s,seek_image,read_image,begin,write_image,end,poll};e=update_extract(&ops,&p);if(!e){e=update_info(s.path,&cat);if(!e){cat.hFileInfo.ioFlFndrInfo.fdFlags=p.flags;cat.hFileInfo.ioFlCrDat=p.created;cat.hFileInfo.ioFlMdDat=p.modified;cat.hFileInfo.ioNamePtr=name;cat.hFileInfo.ioVRefNum=0;cat.hFileInfo.ioDirID=0;cat.hFileInfo.ioFDirIndex=0;e=PBSetCatInfoSync(&cat);}}
 if(e){if(s.out>=0)FSClose(s.out);HDelete(0,0,name);return e==userCanceledErr?130:1;}return 0;
}
