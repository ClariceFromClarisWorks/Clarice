"""Actual parser: optional explanations, heading selection, status and streams."""
import ast,subprocess,tempfile
from pathlib import Path
t=ast.parse(Path('tests/commands/listing-operands.py').read_text())
stub=ast.literal_eval(next(n.value for n in t.body if isinstance(n,ast.Assign) and any(isinstance(x,ast.Name) and x.id=='stub' for x in n.targets)))
stub=stub.replace('static void say(const char *s){(void)s;}', 'static char out[8000],err[8000],lastpath[256];static int detail,all,human;\nstatic void say(const char *s){assert(strlen(out)+strlen(s)<sizeof out);strcat(out,s);}')
stub=stub.replace('static void errsay(const char *s){(void)s;}', 'static void errsay(const char *s){strcat(err,s);}')
stub=stub.replace('(void)d;(void)a;(void)pg;(void)h;(void)one;', 'detail=d;all=a;human=h;(void)pg;(void)one;strcpy(lastpath,p);')
stub+='\nstatic long capture(int fd,const void *data,unsigned long n){(void)fd;strncat(out,data,n);return n;}\nstatic int capture_help(int fd,const char *text){return help_format(80,fd,text,capture,0);}\n#define help_output capture_help\n'
s=Path('src/listing.inc').read_text();s=s[s.index('/* Options are validated first.'):s.index('static int df_row(')]
main=r'''
static int run(int n,char **v){out[0]=err[0]=lastpath[0]=0;calls=detail=all=human=0;return ls_command(n,v);}
int main(void){
 char *ordinary[]={"ls","-l"},*allv[]={"ls","-la"},*readable[]={"ls","-lah"};
 assert(!run(2,ordinary)&&calls==1&&detail==1&&!all&&!human&&!out[0]);
 assert(!run(2,allv)&&detail==1&&all&&!human&&!out[0]);
 assert(!run(2,readable)&&detail==1&&all&&human&&!out[0]);
 char *legend[]={"ls","--legend"};assert(!run(2,legend)&&calls==1&&detail==1);assert(strstr(out,"FLG: kind")&&strstr(out,"Sizes are bytes")&&strstr(out,"FLG       DATA   RESOURCE TYPE CREA CREATED          NAME\n")&&!strstr(out,"HEX"));
 char *full[]={"ls","--legend","--full","-lah"};assert(!run(4,full)&&detail==2&&human&&all);assert(strstr(out,"FLAGS HEX        DATA   RESOURCE TYPE CREA CREATED          NAME\n")&&strstr(out,"Sizes use KiB/MiB"));
 char *multi[]={"ls","--legend","one","two"};assert(!run(4,multi)&&calls==2);char *first=strstr(out,"FLG: kind");assert(first&&!strstr(first+1,"FLG: kind"));assert(strstr(out,"one:\nFLG ")&&strstr(out,"two:\nFLG "));
 char *dash[]={"ls","--","--legend"};assert(!run(3,dash)&&calls==1&&!detail&&!strcmp(lastpath,"--legend")&&!out[0]);
 char *bad[]={"ls","--legend","--wrong"};assert(run(3,bad)==2&&!calls&&!out[0]&&strstr(err,"usage:"));
 char *help[]={"ls","--help"};assert(!run(2,help)&&!calls&&strstr(out,"--legend")&&strstr(out,"type and creator")&&!err[0]);
 output_error=1;assert(run(2,legend)==1&&!calls);output_error=0;
 output_ref=4;script_depth=1;assert(!run(2,legend)&&calls==1&&strstr(out,"TYPE/CREA"));
 puts("PASS -l/-la/-lah, --legend implies -l, human/full headings, one explanation/multiple headers, -- operands, help/stdout, invalid options/stderr, output failure before catalog work, explicit redirected/script legend");
}
'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'t.c').write_text(stub+s+main)
 subprocess.run(['cc','-Isrc','-std=c99','-fsanitize=address,undefined',str(p/'t.c'),'-o',str(p/'t')],check=True)
 subprocess.run([str(p/'t')],check=True)
