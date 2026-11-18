"""Real resolver + real listing loop: equal results, fewer catalog requests."""
from pathlib import Path
import ast, subprocess, tempfile

def literal(path, name):
    tree=ast.parse(Path(path).read_text())
    return ast.literal_eval(next(n.value for n in tree.body if isinstance(n,ast.Assign) and any(isinstance(t,ast.Name) and t.id==name for t in n.targets)))
pre=literal('tests/namespace/resolve-info.py','preamble')
pre=pre.replace('typedef struct {struct {int ioFlAttrib;} hFileInfo;int identity;} CInfoPBRec;', '''
typedef struct {unsigned short fdFlags;} FInfo;
typedef union {struct {unsigned char *ioNamePtr;short ioVRefNum,ioFDirIndex;long ioDirID;int ioFlAttrib;FInfo ioFlFndrInfo;} hFileInfo;
struct {unsigned char *name;short ioVRefNum,index;long ioDrDirID;} dirInfo;} Catalog;
typedef struct {union {Catalog cat;struct {unsigned char *ioNamePtr;short ioVRefNum,ioFDirIndex;long ioDirID;int ioFlAttrib;FInfo ioFlFndrInfo;} hFileInfo;struct {unsigned char *name;short ioVRefNum,index;long ioDrDirID;} dirInfo;};int identity;} CInfoPBRec;''')
pre=pre.replace('static int calls,fail;', 'static int calls,fail,missing_virtual;')
pre=pre.replace('calls++;if(fail)return -36;', 'calls++;if(fail)return -36;if(missing_virtual&&!strcmp(s,"/mnt"))return fnfErr;')
pre=pre.replace('p->identity=i;', 'memset(p,0,sizeof *p);p->hFileInfo.ioVRefNum=-1;p->hFileInfo.ioDirID=42;p->identity=i;')
resolver=Path('src/namespace.inc').read_text()
resolver=resolver[resolver.index('static int resolve_full('):resolver.index('static int protected_path(')]
stubs=r'''
#define LS_START ((void)0)
#define LS_PHASE(n) ((void)0)
#define nsvErr -35
typedef struct {int detail,human,paged,multi,used,presented;char row[132];char *buffer;} Listing;
static int output_ref=-1,script_depth,output_error;
static unsigned long list_ticks,list_first,list_entries;
static char rendered[1024];
static unsigned long TickCount(void){return 1;}
static void app_page_begin(void){}
static void term_present(void){}
static int app_yield(void){return 0;}
static void error(const char *s,int e){(void)s;(void)e;}
static int volume_name(int i,char *raw){if(i>1)return nsvErr;strcpy(raw,"Boot");return 0;}
static int encode_name(const char *s,char *out,size_t n){(void)n;strcpy(out,s);return 0;}
static int PBGetCatInfoSync(CInfoPBRec *p){(void)p;return fnfErr;}
static int list_item(Listing *l,const char *name,const CInfoPBRec *p){strcat(rendered,name);if(l->detail&&p){char b[32];snprintf(b,sizeof b,":%d",p->identity);strcat(rendered,b);}strcat(rendered,"|");return 0;}
static int list_flush(Listing *l){(void)l;return 0;}
#define LIST_BUFFER_SIZE 4096
static void *cli_alloc(unsigned long n){(void)n;return 0;}
static void cli_free(void *p){(void)p;}
static void list_drain(Listing *l){(void)l;}
'''
main=r'''
int main(void){
 const char *cases[]={"/",".","..","file","/tmp/file","/tmp/.","/tmp/..","/alias/file","/tmp/ref","/tmp/up","/alias/../tmp/file","/mnt/Test 2/file","native:/tmp/file","~","~/tmp/file","/missing","/tmp/missing","/tmp/file/..","/cycle","/mnt","/mnt/.","/mnt/../mnt"};
 for(missing_virtual=0;missing_virtual<2;missing_virtual++)for(fail=0;fail<2;fail++)for(int detail=0;detail<2;detail++)for(unsigned i=0;i<sizeof cases/sizeof *cases;i++){
  calls=0;rendered[0]=0;int e=list_impl(cases[i],detail,1,0,0,1);
  printf("%d %d %d %s\t%d\t%s\t%d\n",missing_virtual,fail,detail,cases[i],e,rendered,calls);
 }
}
'''
results=[]
with tempfile.TemporaryDirectory() as tmp:
    for variant in ('base','next'):
        source=(subprocess.check_output(['git','show','6a0546a:src/listing.inc'],text=True) if variant=='base' else Path('src/listing.inc').read_text())
        source=source[source.index('static int list_walk(') if 'static int list_walk(' in source else source.index('static int list_impl('):source.index('/* Batch bulk paints')]
        p=Path(tmp)/variant;p.with_suffix('.c').write_text(pre+resolver+stubs+source+main)
        subprocess.run(['cc','-std=c11','-fsanitize=address,undefined',str(p.with_suffix('.c')),'-o',str(p)],check=True)
        results.append(subprocess.check_output([str(p)],text=True).splitlines())
assert len(results[0])==len(results[1])==176
savings=[]
for old,new in zip(*results):
    a=old.split('\t');b=new.split('\t');assert a[:3]==b[:3],(old,new)
    if a[0].startswith('0 0') and a[1]=='0':
        assert int(b[3])<=int(a[3]),(old,new)
        if int(b[3])<int(a[3]):savings.append((a[0],int(a[3]),int(b[3])))
assert savings
print('PASS: 176 matched real-resolver/listing cases; paths, links, errors, metadata, virtual /mnt without backing directory; no stale result')
for case,before,after in savings:print(case,': catalog metadata calls',before,'->',after)
