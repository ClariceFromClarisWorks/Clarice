"""Actual native environment and glob helpers; host File Manager fixture."""
from pathlib import Path
import subprocess,tempfile,sys
before='--before' in sys.argv
# Reuse the existing focused File Manager shim, not a second glob implementation.
s=Path('tests/userland-p1/native.py').read_text();pre=s[s.index("pre=r'''")+8:s.index("'''\ns=Path")]
pre=pre.replace("static int PBGetCatInfoSync",'static const char *leaf="b";\nstatic int PBGetCatInfoSync')
a="p->hFileInfo.ioNamePtr[0]=31;memset(p->hFileInfo.ioNamePtr+1,'%',31);"
pre=pre.replace(a,'p->hFileInfo.ioNamePtr[0]=strlen(leaf);strcpy((char*)p->hFileInfo.ioNamePtr+1,leaf);')
def fn(s,name):
 a=s.rfind('static ',0,s.index(name+'('));b=s.index('{',a);e=b+1;n=1
 while n:n+=(s[e]=='{')-(s[e]=='}');e+=1
 return s[a:e]
sh=Path('src/sh.inc').read_text();port=Path('src/port-files.inc').read_text()
code=pre+'\n#define bdNamErr -37\n'+fn(Path('src/namespace.inc').read_text(),'encode_name')+'\n#include <ctype.h>\n'+Path('src/shell-env.inc').read_text()+'\nstatic char unix_cwd[]="/tmp";\n'+fn(port,'cli_environment')+fn(sh,'sh_pattern')+fn(sh,'sh_glob')+r'''
int main(void){
 char value[128];assert(cli_environment(0,value,sizeof value)==1);printf("default=%s\n",value);
 assert(!strcmp(value,BEFORE?"HOME=/":"HOME=/home"));
 assert(!env_set("HOME=/custom"));assert(cli_environment(0,value,sizeof value)==1&&!strcmp(value,"HOME=/custom"));
 assert(cli_environment(0,value,4)==-1);env_free(sh_env);sh_env=0;
 sh_env_defaults=0;assert(!cli_environment(0,value,sizeof value));
 assert(!env_set("HOME=/explicit"));assert(cli_environment(0,value,sizeof value)==1&&!strcmp(value,"HOME=/explicit"));env_free(sh_env);sh_env=0;
 sh_env_defaults=3;
 char pattern[]="[a-c]";unsigned char mode[]={0,0,1,0,0,0};struct sh_args a={0};
 assert(!sh_glob(&a,pattern,mode)&&a.n==1);printf("escaped-hyphen=%s\n",a.v[0]);assert(!strcmp(a.v[0],BEFORE?"b":"[a-c]"));free(a.v[0]);
 leaf="-";a.n=0;assert(!sh_glob(&a,pattern,mode));assert(!strcmp(a.v[0],BEFORE?"[a-c]":"-"));free(a.v[0]);
 memset(mode,0,sizeof mode);leaf="b";a.n=0;assert(!sh_glob(&a,pattern,mode)&&!strcmp(a.v[0],"b"));free(a.v[0]);
 puts("PASS default/override/suppressed HOME; literal and unescaped native bracket ranges");
}
'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.c').write_text(code);subprocess.run(['cc','-DBEFORE='+str(int(before)),'-fsanitize=address,undefined',str(p/'test.c'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True)
