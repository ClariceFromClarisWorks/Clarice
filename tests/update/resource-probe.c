#include <Quickdraw.h>
#include <Fonts.h>
#include <Windows.h>
#include <Menus.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <OSUtils.h>
#include <stdio.h>
#include "../../src/update/native-launch.h"
#include "../../src/update/stamp.h"
int main(void){InitGraf(&qd.thePort);InitFonts();InitWindows();InitMenus();TEInit();InitDialogs(0);InitCursor();FILE *f=fopen("Sys6Unix:HIL Results:resource-probe.txt","w");if(!f)return 1;
 UniversalProcPtr missing=GetToolboxTrapAddress(0xa89f),h=GetToolboxTrapAddress(0xa81a),older=GetToolboxTrapAddress(0xa9c4);
 fprintf(f,"HOpenResFile=%08lx OpenRFPerm=%08lx Unimplemented=%08lx h_available=%d older_available=%d\n",(unsigned long)h,(unsigned long)older,(unsigned long)missing,h!=missing,older!=missing);fflush(f);FlushVol(0,0);
 uint32_t build=0;unsigned system=0;long before=FreeMem();int e=update_stamp("Sys6Unix:Clarice",&build,&system);fprintf(f,"stamp_error=%d build=%lu system=%04x free_before=%ld free_after=%ld\n",e,(unsigned long)build,system,before,FreeMem());fclose(f);FlushVol(0,0);update_native_launch("Sys6Unix:HashBench");ExitToShell();return 0;}
