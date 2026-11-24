"""Exercise actual ls/df argument parsing, without touching native disks."""
from pathlib import Path
import subprocess,tempfile
s=Path('src/listing.inc').read_text();src=s[s.index('/* Options are validated first.'):s.index('static int df_row(')]
src+=s[s.index('static int df_command('):]
stub=r'''
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "help-output.h"
#define PATH_CAP 256
#define MAX_ARGS 32
#define nsvErr -35
typedef struct {int unused;} CInfoPBRec;
typedef struct {struct {unsigned char *ioNamePtr;int ioVolIndex;} volumeParam;} HParamBlockRec;
static int output_error,output_ref=-1,script_depth,calls,cancel_at,fail_at;
static void say(const char *s){(void)s;}
static void errsay(const char *s){(void)s;}
static void error(const char*s,int e){(void)s;(void)e;}
static int list(const char*p,int d,int a,int pg,int h,int one){(void)d;(void)a;(void)pg;(void)h;(void)one;assert(p);calls++;return cancel_at==calls?130:fail_at==calls?1:0;}
static int resolve(const char *p,char *n){assert(p);strcpy(n,"Boot:file");return 0;}
static int info(const char *p,CInfoPBRec*b){(void)p;(void)b;return 0;}
static int same_name(const char*a,const char*b){return !strcmp(a,b);}
static int PBHGetVInfoSync(HParamBlockRec *p){if(p->volumeParam.ioVolIndex>1)return nsvErr;memcpy(p->volumeParam.ioNamePtr,"\4Boot",5);return 0;}
static int df_row(HParamBlockRec*p,int h){(void)p;(void)h;calls++;return cancel_at==calls?130:fail_at==calls?1:0;}
'''
main=r'''
int main(void){char *av[132];av[0]="ls";av[1]="-h";for(int i=2;i<132;i++)av[i]="file";
 for(int df=0;df<2;df++){
  int (*command)(int,char**)=df?df_command:ls_command;
  calls=cancel_at=fail_at=0;assert(!command(132,av)&&calls==130);
  calls=0;cancel_at=45;assert(command(132,av)==130&&calls==45);
  calls=cancel_at=0;fail_at=45;assert(command(132,av)==1&&calls==130);
  calls=fail_at=0;assert(!command(1,av)&&calls==1);
  char *bad[]={"ls","file","-Z"};calls=0;assert(command(3,bad)==2&&!calls);
  char *dash[]={"ls","--","-Z","-"};calls=0;assert(!command(4,dash)&&calls==2);
  for(int i=2;i<132;i++)assert(!strcmp(av[i],"file"));
 }
 puts("PASS: ls/df 130 operands, no argv mutation, errors, cancellation, defaults, -- and invalid options");
}
'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.c').write_text(stub+src+main)
 subprocess.run(['cc','-Isrc','-std=c99','-fsanitize=address,undefined',str(p/'test.c'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
