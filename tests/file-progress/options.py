from pathlib import Path
import subprocess,tempfile
s=Path('src/commands/files.c').read_text();s=s[s.index('static void usage_text'):]
pre=r'''
#include <stdio.h>
#include <string.h>
#include <assert.h>
#define PATH_CAP 512
#define fnfErr -43
typedef struct{struct{unsigned long ioFlMdDat;}hFileInfo;struct{unsigned long ioDrMdDat;}dirInfo;}CInfoPBRec;
static int recursive,preserve,force,parents,nocreate,output_error;
short input_ref=-1,output_ref=-1,error_ref=-3;int script_depth;
int cli_isatty(int fd){(void)fd;return error_ref<0&&error_ref!=-2;}
static int calls,progress_was_enabled,rows,last_fd;static char help[4096];
static unsigned long cli_ticks(void){return 0;}
static unsigned long cli_mac_time(void){return 0;}
static int app_columns(void){return 80;}
static void app_progress(const char*s,int done){(void)s;(void)done;rows++;}
'''
pre+='#include "'+str(Path('src/commands/file-progress.h').resolve())+'"\n'
pre+=r'''
static int transfer(int n,char**v,int i,int moving){(void)v;(void)moving;calls++;progress_was_enabled=progress_enabled;return n-i<2?2:0;}
static int attr(int n,char**v){(void)n;(void)v;return 0;}
static int symlink_command(int n,char**v){(void)n;(void)v;return 0;}
static int mkdir_one(const char*s){(void)s;return 0;}
static int remove_one(const char*s,int d,int onlydir){(void)s;(void)d;(void)onlydir;calls++;progress_was_enabled=progress_enabled;return 0;}
static int resolve_ex(const char*s,char*n,char*c,int f){(void)s;(void)n;(void)c;(void)f;return 0;}
static int checked(const char*s,char*n,char*c,int f){return resolve_ex(s,n,c,f);}
static int link_target(const char*s,char*t){(void)s;(void)t;return 0;}
static int info(const char*s,CInfoPBRec*p){(void)s;(void)p;return 0;}
static int isdir(CInfoPBRec*p){(void)p;return 0;}
static int cli_native_create(const char*s,int a,int b){(void)s;(void)a;(void)b;return 0;}
static int cli_native_set(const char*s,CInfoPBRec*p){(void)s;(void)p;return 0;}
static void say(const char*s){(void)s;}
static int fail(const char*s,int e){(void)s;return e?1:0;}
static int app_yield(void){return 0;}
static long cli_write(int fd,const char*s,unsigned long n){last_fd=fd;strncat(help,s,n);return n;}
'''
main=r'''
static void reset(void){calls=rows=recursive=preserve=force=parents=nocreate=0;help[0]=0;script_depth=0;input_ref=output_ref=-1;error_ref=-3;}
int main(void){
 for(int k=0;k<3;k++){char *name=k==0?"cp":k==1?"mv":"rm";
  char *normal[]={name,"source","target"};reset();assert(!clarice_main(3,normal)&&calls&&progress_was_enabled&&!rows);
  char *quiet[]={name,"--no-progress","source","target"};reset();assert(!clarice_main(4,quiet)&&calls&&!progress_was_enabled&&!rows);
  reset();script_depth=1;assert(!clarice_main(3,normal)&&!progress_was_enabled);
  reset();output_ref=4;assert(!clarice_main(3,normal)&&!progress_was_enabled);
  char *bad[]={name,"--bogus","source","target"};reset();assert(clarice_main(4,bad)==2&&!calls&&last_fd==2);
  char *dash[]={name,"--","--no-progress","target"};reset();assert(!clarice_main(4,dash)&&progress_was_enabled);
  char *h[]={name,"--help"};reset();assert(!clarice_main(2,h)&&!calls&&last_fd==1&&strstr(help,"--no-progress")&&strstr(help,"scripts/stdout redirection"));
 }
 char *bad[]={"touch","--no-progress","x"};reset();assert(clarice_main(3,bad)==2&&!calls);
 puts("PASS actual cp/mv/rm option parsing, default and quiet progress, -- operands, invalid option status/stderr, help/stdout, scripts and redirected output, no option leak to touch");
}
'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.c').write_text(pre+s+main);subprocess.run(['cc','-fsanitize=address,undefined',str(p/'test.c'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True)
