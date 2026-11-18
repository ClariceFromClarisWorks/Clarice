/* Invocation-owned counters; native file mutations retain their original order. */
extern short input_ref,output_ref;
extern int cli_isatty(int);
extern int script_depth;
static int file_progress_moving,file_progress_known;
static unsigned long file_progress_bytes,file_progress_total,file_progress_count;
static char file_progress_name[40],file_progress_prefix[64];
static const char *file_progress_command;
static int file_progress_columns(void){return app_columns()-(int)strlen(file_progress_prefix);}
static void file_progress_row(const char *row,int finish){
 char line[132];unsigned n=strlen(file_progress_prefix),m=strlen(row);
 if(n>130)n=130;if(m>131-n)m=131-n;memcpy(line,file_progress_prefix,n);memcpy(line+n,row,m);line[n+m]=0;app_progress(line,finish);
}
#define progress_clock cli_ticks
#define progress_tty() (!script_depth&&input_ref==-1&&output_ref==-1&&cli_isatty(2))
#define progress_row file_progress_row
#define progress_columns file_progress_columns
#define PROGRESS_PENDING_COMMIT 1
/* Per-file bytes with an invocation-wide clock would give misleading rates. */
#define PROGRESS_NO_RATE 1
#include "progress.h"
static void file_progress_current(const char *path){
 if(!progress_enabled)return;
 const char *leaf=strrchr(path,':');if(!leaf)leaf=strrchr(path,'/');leaf=leaf?leaf+1:path;
 snprintf(file_progress_name,sizeof file_progress_name,"%.31s",leaf);
 for(unsigned i=0;file_progress_name[i];i++)if((unsigned char)file_progress_name[i]<32||(unsigned char)file_progress_name[i]>=127)file_progress_name[i]='?';
 file_progress_known=0;file_progress_bytes=file_progress_total=0;
}
static void file_progress_show(int finish){
 if(!progress_enabled)return;
 unsigned long now=cli_ticks();
 /* No flash for quick operations; do not format every chunk or item. */
 if(!progress_shown&&now-progress_started<30)return;
 if(!finish&&now-progress_last<60)return;
 int available=app_columns()-1;
 snprintf(file_progress_prefix,sizeof file_progress_prefix,"%s %lu: %.31s ",file_progress_command,file_progress_count,file_progress_name);
 if((int)strlen(file_progress_prefix)>available/2){snprintf(file_progress_prefix,sizeof file_progress_prefix,"%s %lu: ",file_progress_command,file_progress_count);}
 if((int)strlen(file_progress_prefix)>available/2)file_progress_prefix[0]=0;
 if(file_progress_known)progress_update(file_progress_bytes,file_progress_total,1,finish);
 else{char info[64];snprintf(info,sizeof info,"%lu items%s",file_progress_count,finish>0?" done":finish<0?" stopped":" completed");progress_message(info,finish);}
}
static void file_progress_complete(void){if(progress_enabled){file_progress_count++;file_progress_show(0);}}
