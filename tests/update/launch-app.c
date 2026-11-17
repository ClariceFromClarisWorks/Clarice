/* Disposable native transfer-launch acceptance, not an updater. */
#include <Quickdraw.h>
#include <Fonts.h>
#include <Windows.h>
#include <Menus.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <stdio.h>
#include "../../src/update/identity.h"
#include "../../src/update/native-launch.h"
#ifndef UPDATE_LAUNCH_TARGET
#define UPDATE_LAUNCH_TARGET "Sys6Unix:GuardAfter"
#endif
int main(void){InitGraf(&qd.thePort);InitFonts();InitWindows();InitMenus();TEInit();InitDialogs(0);InitCursor();
 char own[256];int e=update_application_path(own);FILE *f=fopen("Sys6Unix:HIL Results:launch-before.txt","w");if(f){fprintf(f,"identity=%d path=%s\n",e,e?"":own);fclose(f);}FlushVol(0,0);
 if(!e)e=update_native_launch(UPDATE_LAUNCH_TARGET);
 f=fopen("Sys6Unix:HIL Results:launch-error.txt","w");if(f){fprintf(f,"launch returned %d\n",e);fclose(f);}ExitToShell();return 0;}
