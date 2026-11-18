"""Actual listing, buffered errors/cleanup and legacy-column equivalence."""
from pathlib import Path
import ast, subprocess, tempfile
root=Path.cwd()
def literal(path,name):
 t=ast.parse(Path(path).read_text())
 return ast.literal_eval(next(n.value for n in t.body if isinstance(n,ast.Assign) and any(isinstance(x,ast.Name) and x.id==name for x in n.targets)))
pre=literal('tests/commands/listing-host.py','preamble')
a=pre.index('typedef struct {unsigned short fdFlags;');b=pre.index('static short expected_volume;')
pre=pre[:a]+r'''
#include <stdlib.h>
typedef struct {unsigned long type,creator;unsigned short fdFlags;} FInfo;
typedef union {struct {unsigned char *ioNamePtr;short ioVRefNum,ioFDirIndex;long ioDirID;unsigned char ioFlAttrib;FInfo ioFlFndrInfo;unsigned long ioFlLgLen,ioFlRLgLen,ioFlCrDat;} hFileInfo;struct {unsigned char *name;short ioVRefNum,index;long ioDrDirID;unsigned char attr;FInfo info;unsigned long a,b,ioDrCrDat;} dirInfo;} CInfoPBRec;
static int output_ref=-1,script_depth,output_error,seen,calls,cancel,fault,oversize;
static int writes,allocated,peak,alloc_fail,write_fail,yields,cancel_at,width=80;
static char output[300000];static unsigned used;
static void emit(const char *s,long n){writes++;if(write_fail==writes){output_error=1;return;}assert(used+n<sizeof output);memcpy(output+used,s,n);used+=n;output[used]=0;}
static void say(const char *s){emit(s,strlen(s));}
static void *cli_alloc(unsigned long n){if(alloc_fail)return 0;assert(!allocated);allocated=n;peak=n;return malloc(n);}
static void cli_free(void *p){assert(allocated);allocated=0;free(p);}
static int app_columns(void){return width;}
static int app_page(void){return ++yields==cancel_at;}
static void app_output_batch(int n){(void)n;}
static int is_link(const CInfoPBRec *p){return p&&p->hFileInfo.ioFlFndrInfo.type==7;}
static void format_size(char *s,unsigned long n,int h){(void)h;sprintf(s,"%lu",n);}
#define ls_emit emit
#define ls_yield(p) ((p)?app_page():app_yield())
'''+pre[b:]
pre=pre.replace('return cancel;', 'return ++yields==cancel_at;')
pre=pre[:pre.index('static int list_item(')]
a=pre.index('static int PBGetCatInfoSync(')
pre=pre[:a]+r'''
static int entries=1000;
static int PBGetCatInfoSync(CInfoPBRec *p){calls++;int i=p->hFileInfo.ioFDirIndex;if(fault&&i==17)return -36;if(i>entries)return fnfErr;char n[32];sprintf(n,"file%04d",i);p->hFileInfo.ioNamePtr[0]=8;memcpy(p->hFileInfo.ioNamePtr+1,n,8);p->hFileInfo.ioFlAttrib=i%7==0?16:0;p->hFileInfo.ioFlFndrInfo.fdFlags=0;p->hFileInfo.ioFlLgLen=i;p->hFileInfo.ioFlRLgLen=0;p->hFileInfo.ioFlCrDat=0;return 0;}
'''
pre+=Path('src/commands/created.c').read_text()+Path('src/commands/decimal.c').read_text()
main=r'''
int main(int argc,char **argv){assert(argc==9);int detail=atoi(argv[1]),ref=atoi(argv[2]);output_ref=ref;alloc_fail=atoi(argv[3]);cancel_at=atoi(argv[4]);write_fail=atoi(argv[5]);fault=atoi(argv[6]);width=atoi(argv[7]);int one=atoi(argv[8]);
 int e=list("/fixture",detail,1,ref<0,0,one);assert(!allocated);fwrite(output,1,used,stdout);fprintf(stderr,"%d %d %d %d %d\n",e,writes,peak,calls,yields);return 0;
}
'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d);bins=[]
 for name,path in [('before','tests/ls-output/before-listing.inc'),('after','src/listing.inc')]:
  s=Path(path).read_text();s=s[s.index('typedef struct'):s.index('/* Options are validated first.')]
  c=p/(name+'.c');c.write_text(pre+s+main);exe=p/name
  subprocess.run(['cc','-std=c99','-fsanitize=address,undefined',str(c),'-o',str(exe)],check=True);bins.append(exe)
 def run(exe,args):
  r=subprocess.run([str(exe),*map(str,args)],capture_output=True,check=True)
  return r.stdout,list(map(int,r.stderr.split()))
 count=0
 for detail in [0,1,2]:
  for ref in [-1,4]:
   for fail in [0,1]:
    for cancel in [0,1,18,400]:
     for fault in [0,1]:
      for width in [20,80,132]:
       for one in [0,1]:
        args=[detail,ref,fail,cancel,0,fault,width,one]
        old,om=run(bins[0],args);new,nm=run(bins[1],args)
        projected=b''.join(line[:26]+line[36:] for line in new.splitlines(keepends=True)) if detail==1 else new
        assert old==projected,(args,len(old),len(new));assert om[0]==nm[0] and om[3]==nm[3],(args,om,nm)
        assert nm[2]==(4096 if ref>=0 and not fail else 0)
        count+=1
 for detail in [0,1,2]:
  for nth in [1,2,3]:
   _,m=run(bins[1],[detail,4,0,0,nth,0,80,1])
   # Small output may finish before the requested failure.
   assert m[0] in [0,1] and (m[0]!=1 or m[1]==nth),m
 for detail in [0,1]:
  _,old=run(bins[0],[detail,4,0,0,0,0,80,1]);_,new=run(bins[1],[detail,4,0,0,0,0,80,1])
  print('1000 files plus dots, detail',detail,': writes',old[1],'->',new[1],'; peak',new[2])
 print('PASS:',count,'before/after byte/status cases; allocation fallback, cancellation, catalog faults, widths, paging, detailed and plain output; failed writes are not retried; allocations freed')
