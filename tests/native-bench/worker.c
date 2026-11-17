#include <Multiverse.h>
#include "common.h"
#include "../../src/update/identity.h"
static void explain(void){ParamText("\065This is a Clarice launch benchmark. Run it from Clarice.","\0","\0","\0");Alert(151,0);}
int main(void){unsigned long entered=TickCount();InitGraf(&qd.thePort);InitFonts();InitWindows();InitMenus();TEInit();InitDialogs(0);InitCursor();
 char path[256];NBJob j;short r=-1;int e=update_application_path(path);if(e){explain();return 1;}strcpy(strrchr(path,':')+1,"NB Request");unsigned char n[256];n[0]=strlen(path);memcpy(n+1,path,n[0]);e=HOpen(0,0,n,fsRdWrPerm,&r);long count=sizeof j;if(!e)e=FSRead(r,&count,&j);
 if(e||count!=sizeof j||j.magic!=NB_MAGIC||!memchr(j.argument,0,sizeof j.argument)||TickCount()-j.started>1800){if(r>=0)FSClose(r);explain();return 1;}
 j.entered=entered;j.heap=FreeMem();nb_body(&j);j.finished=TickCount();j.magic=NB_MAGIC+1;e=SetFPos(r,fsFromStart,0);count=sizeof j;if(!e)e=FSWrite(r,&count,&j);int closed=FSClose(r);return e||closed||count!=sizeof j?1:0;
}
