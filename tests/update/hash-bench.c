#include <Quickdraw.h>
#include <Fonts.h>
#include <Windows.h>
#include <Menus.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <OSUtils.h>
#include <stdio.h>
#include "../../src/update/native-launch.h"
#include <stdint.h>
#include "../../src/update/vendor/sha256.h"
#include "../../src/update/crc32.h"
int main(void){InitGraf(&qd.thePort);InitFonts();InitWindows();InitMenus();TEInit();InitDialogs(0);InitCursor();FILE *f=fopen("Sys6Unix:HIL Results:hash-bench.txt","w");if(!f)return 1;unsigned char buffer[1024],digest[32];for(unsigned i=0;i<sizeof buffer;i++)buffer[i]=i;struct sha256 sha;UpdateCRC32 crc;
 unsigned long t=TickCount();sha256_init(&sha);for(unsigned i=0;i<64;i++)sha256_update(&sha,buffer,sizeof buffer);sha256_sum(&sha,digest);unsigned long sha_ticks=TickCount()-t;
 t=TickCount();update_crc32_init(&crc);for(unsigned i=0;i<64;i++)update_crc32_bytes(&crc,buffer,sizeof buffer);uint32_t sum=update_crc32_sum(&crc);unsigned long crc_ticks=TickCount()-t;
 fprintf(f,"bytes=65536 sha256_ticks=%lu crc32_ticks=%lu crc32=%08lx sha256=",sha_ticks,crc_ticks,(unsigned long)sum);for(unsigned i=0;i<32;i++)fprintf(f,"%02x",digest[i]);fprintf(f,"\n");fclose(f);FlushVol(0,0);update_native_launch("Sys6Unix:NativeTxn");ExitToShell();return 0;}
