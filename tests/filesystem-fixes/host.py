"""Exercise production decision logic with deterministic native boundary faults."""
from pathlib import Path
import subprocess,tempfile

def function(source,name):
 start=source.index('int '+name+'(')
 if source[max(0,start-7):start]=='static ':start-=7
 brace=source.index('{',start);depth=1;end=brace+1
 while depth:
  depth+=(source[end]=='{')-(source[end]=='}');end+=1
 return source[start:end]

files=Path('src/commands/files.c').read_text()
pre=r'''
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#define PATH_CAP 512
enum {bdNamErr=-37,fnfErr=-43,dirNFErr=-120,paramErr=-50,fLckdErr=-45};
typedef struct {struct {int ioFlAttrib;} hFileInfo;} CInfoPBRec;
static int parents=1,preserve,locked,source_dir,target_exists,copy_calls,delete_calls,rename_error,move_error,copy_error,target_error,source_exists,mkdir_calls;
static char target_data[16];
static int app_yield(void){return 0;}
static int checked(const char *p,char *n,char *c,int follow){(void)follow;strcpy(n,p);strcpy(c,p);return 0;}
static int resolve(const char *p,char *n){strcpy(n,p);return 0;}
static int info(const char *p,CInfoPBRec *b){b->hFileInfo.ioFlAttrib=0;if(!strcmp(p,"A:source")){b->hFileInfo.ioFlAttrib=locked|(source_dir?16:0);return source_exists?0:fnfErr;}if(!strcmp(p,".")||!strcmp(p,"existing")){b->hFileInfo.ioFlAttrib=16;return 0;}if(!strcmp(p,"A:target")||!strcmp(p,"B:target")||!strcmp(p,"B:source"))return target_error?target_error:target_exists?0:fnfErr;return fnfErr;}
static int isdir(const CInfoPBRec *b){return !!(b->hFileInfo.ioFlAttrib&16);}
static int same_name(const char *a,const char *b){return !strcasecmp(a,b);}
static int join_path(const char *a,const char *b,char *c){return snprintf(c,PATH_CAP,"%s/%s",a,b)>=PATH_CAP?bdNamErr:0;}
static int fail(const char *n,int e){(void)n;return e?1:0;}
static void errsay(const char *s){(void)s;}
static int cli_native_mkdir(const char *p){mkdir_calls++;return *p?0:bdNamErr;}
static int cli_native_rename(const char *a,const char *b){(void)a;(void)b;if(!rename_error){source_exists=0;target_exists=1;strcpy(target_data,"NEW");}return rename_error;}
static int cli_native_move(const char *a,const char *b){(void)a;(void)b;return move_error;}
static int copy_one(const char *a,const char *b,int d){(void)a;(void)b;(void)d;copy_calls++;if(copy_error)return copy_error;target_exists=1;strcpy(target_data,"NEW");return 0;}
static int cli_native_delete(const char *p){(void)p;delete_calls++;if(locked)return fLckdErr;source_exists=0;return 0;}
static void reset(void){locked=source_dir=copy_calls=delete_calls=rename_error=move_error=copy_error=target_error=0;source_exists=target_exists=1;strcpy(target_data,"OLD");}
'''
main=r'''
int main(void){char *v[]={"mv","A:source","A:target"};
 assert(mkdir_one("")==bdNamErr&&!mkdir_calls);assert(!mkdir_one(".")&&!mkdir_calls);assert(!mkdir_one("existing")&&!mkdir_calls);assert(!mkdir_one("new/child")&&mkdir_calls==2);
 reset();locked=1;assert(transfer(3,v,1,1)==1);assert(!copy_calls&&!delete_calls&&source_exists&&!strcmp(target_data,"OLD"));
 reset();locked=1;target_exists=0;assert(transfer(3,v,1,1)==1&&!target_exists&&!copy_calls&&source_exists);
 reset();assert(!transfer(3,v,1,1)&&!source_exists&&!strcmp(target_data,"NEW"));
 reset();target_exists=0;rename_error=-36;assert(transfer(3,v,1,1)==1&&!target_exists&&!copy_calls&&source_exists);
 reset();target_exists=0;rename_error=paramErr;move_error=fLckdErr;assert(transfer(3,v,1,1)==1&&!target_exists&&!copy_calls&&source_exists);
 reset();target_error=-36;assert(transfer(3,v,1,1)==1&&!copy_calls&&source_exists&&!strcmp(target_data,"OLD"));
 reset();copy_error=-34;assert(transfer(3,v,1,1)==1&&source_exists&&!strcmp(target_data,"OLD"));
 reset();target_exists=0;v[2]="B:source";assert(!transfer(3,v,1,1)&&copy_calls==1&&!source_exists&&target_exists);
 reset();source_dir=1;v[2]="B:target";assert(transfer(3,v,1,1)==1&&!copy_calls&&source_exists);
 puts("PASS empty mkdir, locked source absent/overwrite, native I/O/lock/catalog failures, copy failure, normal overwrite and cross-volume move");}
'''
ns=Path('src/namespace.inc').read_text();session=Path('src/session.inc').read_text()
cwd_pre=r'''
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>
#define PATH_CAP 512
#define bdNamErr -37
static char root_path[PATH_CAP]="A:Root:",shell_cwd[PATH_CAP],unix_cwd[PATH_CAP];
struct sh_saved {char cwd[PATH_CAP],unixdir[PATH_CAP];struct sh_saved *saved_next;};
typedef struct {struct sh_saved shell;} ShellSession;
static struct sh_saved *sh_saved_frames;
static ShellSession *local_session,*remote_session;
'''
cwd_main=r'''
int main(void){
 struct sh_saved frame={.cwd="A:Root:parent:child:",.unixdir="/parent/child"};
 ShellSession remote={.shell={.cwd="A:Root:parent:remote:",.unixdir="/parent/remote"}};
 ShellSession local={.shell={.cwd="A:Root:parent2:",.unixdir="/parent2"}};
 sh_saved_frames=&frame;remote_session=&remote;local_session=&local;
 strcpy(shell_cwd,"A:Root:parent:child:");strcpy(unix_cwd,"/parent/child");
 assert(!shell_relocate_cwd("a:root:parent","A:Root:renamed",0));assert(!strcmp(unix_cwd,"/parent/child"));
 assert(!shell_relocate_cwd("a:root:parent","A:Root:renamed",1));assert(!strcmp(unix_cwd,"/renamed/child"));assert(!strcmp(frame.unixdir,"/renamed/child"));assert(!strcmp(remote.shell.unixdir,"/renamed/remote"));assert(!strcmp(local.shell.unixdir,"/parent2"));
 assert(!shell_relocate_cwd("A:Root:renamed:child","A:Root:other:child",1));assert(!strcmp(unix_cwd,"/other/child"));
 assert(!shell_relocate_cwd("A:Root:other","A:Root:slash/name%",1));assert(!strcmp(unix_cwd,"/slash%2Fname%25/child"));
 char longpath[256];memset(longpath,'x',sizeof longpath-1);longpath[sizeof longpath-1]=0;
 assert(shell_relocate_cwd("A:Root:slash/name%",longpath,0)==bdNamErr);assert(!strcmp(unix_cwd,"/slash%2Fname%25/child"));
 puts("PASS cwd rename/ancestor/move, case/boundary, saved subshell and both sessions, encoded names and nonmutating overflow preflight");}
'''
with tempfile.TemporaryDirectory() as d:
 for name,code in [('files',pre+function(files,'mkdir_one')+function(files,'transfer')+main),('cwd',cwd_pre+function(ns,'encode_name')+function(ns,'unix_from_native')+function(session,'relocate_cwd_pair')+function(session,'shell_relocate_cwd')+cwd_main)]:
  p=Path(d)/name;p.with_suffix('.c').write_text(code)
  subprocess.run(['cc','-std=c99','-g','-fsanitize=address,undefined',str(p.with_suffix('.c')),'-o',str(p)],check=True);subprocess.run([str(p)],check=True)

old=subprocess.check_output(['git','show','7e3e036:src/commands/files.c'],text=True)
with tempfile.TemporaryDirectory() as d:
 p=Path(d)/'old';p.with_suffix('.c').write_text(pre+function(old,'transfer')+r'''
int main(void){char *v[]={"mv","A:source","A:target"};reset();locked=1;assert(transfer(3,v,1,1)==1);assert(!copy_calls&&!strcmp(target_data,"OLD"));}
''')
 subprocess.run(['cc','-std=c99',str(p.with_suffix('.c')),'-o',str(p)],check=True)
 assert subprocess.run([str(p)],capture_output=True).returncode!=0
 print('PASS negative control: locked-source test detects the original overwrite defect')
