"""Host ASan/UBSan regression for the staged pinned getopt stdin-operand fix."""
from pathlib import Path
import tempfile,subprocess
r=Path(__file__).resolve().parents[2]
with tempfile.TemporaryDirectory() as td:
 d=Path(td)
 (d/'reent.h').write_text('struct getopt_data {char *optarg; int optind, opterr, optopt, optwhere, permute_from, num_nonopts;};\n')
 (d/'test.c').write_text(r'''
#include <assert.h>
#include <string.h>
#include <getopt.h>
static void check(char **v,int argc,int want,int first) {
 optind=0;opterr=0;
 int got=getopt_long(argc,v,"ab:",NULL,NULL); assert(got==want);
 if(want=='a')assert(getopt_long(argc,v,"ab:",NULL,NULL)==-1);
 assert(optind==first);
}
int main(void){
 char *a[]={"tool","-","file",NULL};check(a,3,-1,1);
 char *b[]={"tool","file","-a","-",NULL};check(b,4,'a',2);assert(!strcmp(b[2],"file")&&!strcmp(b[3],"-"));
 char *c[]={"tool","--","-",NULL};check(c,3,-1,2);
 char *d[]={"tool","",NULL};check(d,2,-1,1);
 char *e[]={"tool","-b",NULL};optind=0;assert(getopt_long(2,e,"ab:",NULL,NULL)=='?');
 return 0;
}
''')
 subprocess.run(['clang','-g','-DNO_ARG=0','-DREQUIRED_ARG=1','-DOPTIONAL_ARG=2','-fsanitize=address,undefined','-include',str(d/'reent.h'),str(r/'work/busybox-additions/getopt.c'),str(d/'test.c'),'-o',str(d/'test')],check=True)
 subprocess.run([str(d/'test')],check=True)
print('PASS staged getopt lone dash, empty operand, permutation, --, missing argument (ASan/UBSan)')
