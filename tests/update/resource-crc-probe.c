/* Disposable comparison around Resource Manager inspection. */
#include <Quickdraw.h>
#include <Fonts.h>
#include <Windows.h>
#include <Menus.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <OSUtils.h>
#include <stdio.h>
#include "../../src/update/verify.h"
int main(void){InitGraf(&qd.thePort);InitFonts();InitWindows();InitMenus();TEInit();InitDialogs(0);InitCursor();FILE*f=fopen("Sys6Unix:HIL Results:resource-crc-probe.txt","w");if(!f)return 1;const char*path="Sys6Unix:CRC Probe";CInfoPBRec p;int e=update_info(path,&p);uint32_t a=0,b=0,build=0;unsigned sys=0;if(!e)e=update_hash_fork(path,1,&a,p.hFileInfo.ioFlRLgLen);fprintf(f,"before error=%d crc=%08lx\n",e,(unsigned long)a);fflush(f);int stamp=update_stamp(path,&build,&sys);e=update_hash_fork(path,1,&b,p.hFileInfo.ioFlRLgLen);fprintf(f,"stamp=%d build=%lu after error=%d crc=%08lx\n",stamp,(unsigned long)build,e,(unsigned long)b);fclose(f);FlushVol(0,0);ExitToShell();return 0;}
