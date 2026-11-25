"""Native wildcard output uses the same literal-name encoding as traversal."""
from pathlib import Path
import tempfile,subprocess
s=Path('tests/userland-p1/native.py').read_text();e={};exec(s[:s.index("s=Path('src/sh.inc')")],e);pre=e['pre'];fn=e['function']
pre=pre.replace('static int PBGetCatInfoSync','static const char *leaf;\nstatic int PBGetCatInfoSync').replace("p->hFileInfo.ioNamePtr[0]=31;memset(p->hFileInfo.ioNamePtr+1,'%',31);",'p->hFileInfo.ioNamePtr[0]=strlen(leaf);strcpy((char*)p->hFileInfo.ioNamePtr+1,leaf);')
sh=Path('src/sh.inc').read_text();code=pre+'\n#define bdNamErr -37\n'+fn(Path('src/namespace.inc').read_text(),'encode_name')+fn(sh,'sh_pattern')+fn(sh,'sh_glob')+r'''
int main(void){const char *names[]={".","..","/","%","%2E","x/y","tail/"};const char*expected[]={"%2E","%2E%2E","%2F","%25","%252E","x%2Fy","tail%2F"};
 for(int i=0;i<7;i++){leaf=names[i];char pattern[512];unsigned char mode[512]={0};strcpy(pattern,i<2?"dir/.*":"dir/*");struct sh_args a={0};assert(!sh_glob(&a,pattern,mode)&&a.n==1);char wanted[512];snprintf(wanted,sizeof wanted,"dir/%s",expected[i]);assert(!strcmp(a.v[0],wanted));free(a.v[0]);}
 leaf="..";char pattern[512];unsigned char mode[512]={0};memset(pattern,'a',504);pattern[504]='/';strcpy(pattern+505,".*");struct sh_args a={0};assert(!sh_glob(&a,pattern,mode)&&strlen(a.v[0])==511);free(a.v[0]);a.n=0;memset(pattern,'a',505);pattern[505]='/';strcpy(pattern+506,".*");assert(sh_glob(&a,pattern,mode)==2&&!a.n);
 puts("PASS dot/slash/percent wildcard identities and exact 511/512-byte encoded bounds");}
'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.c').write_text(code);subprocess.run(['cc','-fsanitize=address,undefined',str(p/'test.c'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True)
