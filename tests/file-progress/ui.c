#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
short input_ref=-1,output_ref=-1,error_ref=-3;int script_depth;
int cli_isatty(int fd){(void)fd;return error_ref<0&&error_ref!=-2;}
static unsigned long now;static int width=80,calls,finishes;static char row[132];
static unsigned long cli_ticks(void){return now;}
static int app_columns(void){return width;}
static void app_progress(const char *s,int done){assert(strlen(s)<132);assert(strlen(s)<(unsigned)width||width<2);strcpy(row,s);calls++;finishes+=done;}
#include "../../src/commands/file-progress.h"
static void begin(void){calls=finishes=0;file_progress_command="cp";file_progress_count=0;progress_begin();file_progress_current("A:file");}
int main(void){
 begin();file_progress_known=1;file_progress_total=1000;
 for(now=0;now<30;now++){file_progress_bytes=now;file_progress_show(0);}assert(!calls);
 file_progress_show(0);assert(calls==1);now=60;file_progress_show(0);assert(calls==1);
 now=90;file_progress_bytes=1000;file_progress_show(0);assert(calls==2&&strstr(row,"99%")&&!strstr(row,"100%"));
 file_progress_complete();file_progress_known=0;file_progress_show(1);assert(finishes==1&&strstr(row,"1 items done"));
 for(width=1;width<=132;width++){
  now=1000;begin();file_progress_current("A:bad\033name");assert(!strchr(file_progress_name,27));now+=30;
  file_progress_known=1;file_progress_total=0xfffffffeUL;file_progress_bytes=0x80000000UL;file_progress_show(0);assert(calls==1);
  now+=60;file_progress_known=0;file_progress_count=1234;file_progress_show(0);assert(calls==2);file_progress_show(-1);assert(finishes==1);
 }
 width=80;
 for(int mode=0;mode<5;mode++){input_ref=-1;output_ref=-1;error_ref=-3;script_depth=0;if(mode==0)input_ref=3;if(mode==1)output_ref=4;if(mode==2)error_ref=5;if(mode==3)script_depth=1;if(mode==4)output_ref=-6;begin();now+=120;file_progress_show(1);assert(!calls);}
 input_ref=output_ref=-1;error_ref=-3;script_depth=0;begin();progress_enabled=0;now+=120;file_progress_show(1);assert(!calls);
 puts("PASS delayed display, once/second updates, byte completion capped before commit, final success/error, 1..132 columns, both-fork 32-bit totals, sanitized names, item counts, script/redirection/device/explicit suppression");
}
