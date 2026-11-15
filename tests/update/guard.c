#include "../../src/commands/mac-files.h"
#define UPDATE_OPEN_RESOURCE(path,ref) cli_native_open(path,1,0,ref)
#include "../../src/update/open-guard.h"
int clarice_main(int argc,char **argv){char path[PATH_CAP],line[100];int busy=1,e;if(argc!=2)return 2;e=resolve(argv[1],path);if(!e)e=update_open_guard(path,&busy);snprintf(line,sizeof line,"update open guard: error=%d busy=%d\n",e,busy);say(line);return e?1:busy?2:0;}
