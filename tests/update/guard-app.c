#include <Quickdraw.h>
#include <Fonts.h>
#include <Windows.h>
#include <Menus.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <SegLoad.h>
#include <stdio.h>
#include "../../src/update/open-guard.h"
int main(void){InitGraf(&qd.thePort);InitFonts();InitWindows();InitMenus();TEInit();InitDialogs(0);InitCursor();
 int busy=1,e=update_open_guard("Sys6Unix:Clarice",&busy);char line[100];int len=snprintf(line,sizeof line,"error=%d busy=%d\n",e,busy);short ref;
 const unsigned char *name=(const unsigned char *)"\pSys6Unix:HIL Results:guard-after.txt";
 if(!HCreate(0,0,name,'CLRC','TEXT')&&!HOpen(0,0,name,fsWrPerm,&ref)){long n=len;FSWrite(ref,&n,line);FSClose(ref);FlushVol(0,0);}
 ExitToShell();return 0;}
