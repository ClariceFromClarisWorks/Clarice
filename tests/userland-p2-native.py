"""Focused production helpers with File Manager failure injection; no hardware."""
from pathlib import Path
import subprocess,tempfile

def fn(s,name):
 a=s.rfind('static ',0,s.index(name+'('));b=s.index('{',a);n=1;e=b+1
 while n:n+=(s[e]=='{')-(s[e]=='}');e+=1
 return s[a:e]
s=Path('src/port-files.inc').read_text()
code=r'''
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#define PATH_CAP 512
#define PORT_APPEND_REF 131072
#define VSTREAM_TAG 65536
#define dupFNErr -48
#define fnfErr -43
#define fsRdWrPerm 3
#define fsRdPerm 1
#define fsWrPerm 2
#define fsFromLEOF 2
static short input_ref=-1,output_ref=-1,error_ref=-3;
static int exists,open_error,delete_error,reported,deleted;
static int port_errno(int e){errno=e;return -1;}
static void error(const char*s,int e){(void)s;reported=e;}
static int resolve_ex(const char*s,char*p,char*c,int follow){(void)follow;strcpy(p,s);strcpy(c,s);return 0;}
static int vstream_kind(const char*s){return 0;}
static void pstr(unsigned char*p,const char*s){(void)p;(void)s;}
static int HCreate(int a,int b,unsigned char*n,int c,int d){if(exists)return dupFNErr;exists=1;return 0;}
static int HOpen(int a,int b,unsigned char*n,short mode,short*r){if(open_error)return open_error;*r=7;return 0;}
static int HDelete(int a,int b,unsigned char*n){deleted++;if(delete_error)return delete_error;exists=0;return 0;}
static int FSClose(short r){return 0;}
static int SetEOF(short r,int n){return 0;}
static int SetFPos(short r,int mode,int n){return 0;}
'''+fn(s,'cli_isatty')+fn(s,'cli_file_open')+fn(Path('src/shell.c').read_text(),'path_entry')+r'''
int main(void){
 assert(cli_isatty(0)&&cli_isatty(1)&&cli_isatty(2)&&!cli_isatty(9));
 for(int r=-11;r<=-5;r++){input_ref=output_ref=error_ref=r;assert(!cli_isatty(0)&&!cli_isatty(1)&&!cli_isatty(2));}
 open_error=-42;assert(cli_file_open("file","x")==-1&&errno==-42&&!exists&&deleted==1);
 exists=1;assert(cli_file_open("file","x")==-1&&errno==EEXIST&&exists&&deleted==1);
 exists=0;delete_error=-45;assert(cli_file_open("file","x")==-1&&errno==-42&&exists&&reported==-45);
 exists=0;delete_error=open_error=0;assert(cli_file_open("file","x+")==7&&exists);
 char path[2048],out[512];memset(path,'a',600);strcpy(path+600,":/bin::/usr/bin");const char *p=path;
 assert(path_entry(&p,out)&&!strcmp(out,"/bin"));assert(path_entry(&p,out)&&!strcmp(out,"."));assert(path_entry(&p,out)&&!strcmp(out,"/usr/bin"));assert(!path_entry(&p,out));
 puts("PASS device isatty, exclusive-create open/cleanup failures and retry, oversized PATH skip");
}
'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.c').write_text(code);subprocess.run(['cc','-Wno-multichar','-fsanitize=address,undefined',str(p/'test.c'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True)
