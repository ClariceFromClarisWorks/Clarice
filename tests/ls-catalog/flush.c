/* Test-media boundary only: explicitly flush the volume holding the evidence.
 * Not a production command, cache change, or part of any timed operation. */
#include "../../src/sys6.h"
#include <Files.h>
extern int cli_resolve_info(const char *,char *,char *,CInfoPBRec *);
extern void error(const char *,int);
int clarice_main(int argc,char **argv){
 char native[PATH_CAP];CInfoPBRec pb;
 if(argc!=2)return 2;
 int e=cli_resolve_info(argv[1],native,0,&pb);
 if(!e)e=FlushVol(0,pb.hFileInfo.ioVRefNum);
 if(e)error("test flush",e);
 return e?1:0;
}
