"""Production transfer/removal + resolver regressions, with baseline negative controls."""
from pathlib import Path
import subprocess
r=Path.cwd();w=r/'work/filesystem-p1';w.mkdir(parents=True,exist_ok=True)
a={};s=(r/'tests/filesystem-fixes/host.py').read_text();exec(s[:s.index("ns=Path(")],a)
b={};s=(r/'tests/filesystem-confidence/empty-path.py').read_text();exec(s[:s.index("extra=")],b)
extract=a['function'];current=(r/'src/commands/files.c').read_text()
old=subprocess.check_output(['git','show','8d136a1:src/commands/files.c'],text=True)
pre=a['pre'].replace('strcpy(n,p);','if(strlen(p)>=PATH_CAP)return bdNamErr;strcpy(n,p);')
transfer_test=r'''
int main(void){char dest[1025];char *v[]={"cp","A:source",dest};
 for(int moving=0;moving<2;moving++){
  v[0]=moving?"mv":"cp";
  for(int n=511;n<=1024;n++){
   memset(dest,'x',n);dest[n]=0;memcpy(dest,"B:",2);
   /* Encoded percent/slash bytes count in full, not as decoded characters. */
   memcpy(dest+10,"%25%2F",6);reset();int rc=transfer(3,v,1,moving);
   if(n==511){assert(!rc&&copy_calls==1);}
   else{assert(rc==1&&!copy_calls&&!delete_calls&&source_exists&&!strcmp(target_data,"OLD"));}
  }
  reset();strcpy(dest,"B:target");assert(!transfer(3,v,1,moving)&&copy_calls==1);
 }
 puts("PASS cp/mv destination 511/512/1024, encoded HFS bytes, no mutation on rejection and subsequent success");}
'''
# Native calls are mocked, but symlink/trailing separator decisions use the real resolver.
pre_rm=b['preamble'].replace('static int calls,fail;','static int calls,fail;static unsigned active=0x3ff;')
pre_rm=pre_rm.replace('if(!strcasecmp(s,nodes[i].path))','if((active&(1u<<i))&&!strcasecmp(s,nodes[i].path))')
pre_rm=pre_rm.replace('struct {int ioFlAttrib;} hFileInfo;','struct {int ioFlAttrib;short ioVRefNum;} hFileInfo;struct {long ioDrDirID;} dirInfo;')
pre_rm=pre_rm.replace('p->identity=i;', 'p->hFileInfo.ioVRefNum=-1;p->dirInfo.ioDrDirID=i;p->identity=i;')
pre_rm+=r'''
#define memFullErr -108
#define nsvErr -35
static int recursive=1,force,live,deletes,polls,cancel_at,allocation_fail;
static int app_yield(void){return cancel_at&&++polls==cancel_at;}
static void file_progress_current(const char*s){(void)s;}
static void file_progress_show(int n){(void)n;}
static void file_progress_complete(void){}
static void *cli_alloc(size_t n){if(allocation_fail)return NULL;void*p=malloc(n);if(p)live++;return p;}
static void cli_free(void*p){if(p){live--;free(p);}}
static int isdir(const CInfoPBRec*p){return !!(p->hFileInfo.ioFlAttrib&16);}
static int cli_native_delete(const char*p){CInfoPBRec pb;int e=info(p,&pb);if(e)return e;active&=~(1u<<pb.identity);deletes++;return 0;}
static int cli_native_nth(const char*p,int n,char*leaf,CInfoPBRec*pb){(void)n;if(!strcmp(p,"/tmp")&&(active&4)){strcpy(leaf,"file");return info("/tmp/file",pb);}return fnfErr;}
static int directory_nth(short volume,long id,int n,char*leaf,CInfoPBRec*pb){assert(volume==-1);return cli_native_nth(nodes[id].path,n,leaf,pb);}
static int encode_name(const char*p,char*q,size_t n){assert(strlen(p)<n);strcpy(q,p);return 0;}
static int join_path(const char*p,const char*q,char*out){return snprintf(out,PATH_CAP,"%s/%s",p,q)>=PATH_CAP?bdNamErr:0;}
'''
pre_rm='#include <stdlib.h>\n'+pre_rm+b['code']+r'''
static int checked(const char*p,char*n,char*c,int follow){return resolve_ex(p,n,c,follow);}
'''
rm_test=r'''
static void reset(void){active=0x3ff;deletes=polls=cancel_at=allocation_fail=fail=0;assert(!live);}
int main(void){char *bad[]={"/alias/","/alias///","/alias/.///","/tmp/.///"};
 for(unsigned i=0;i<sizeof bad/sizeof *bad;i++){reset();assert(remove_one(bad[i],0,0)!=0);assert(!deletes&&active==0x3ff&&!live);}
 reset();force=1;assert(remove_one("/alias/",0,0)==dirNFErr&&!deletes);force=0;
 reset();assert(!remove_one("/alias",0,0)&&deletes==1&&(active&6)==6&&!live);
 reset();assert(!remove_one("/tmp///",0,0)&&deletes==2&&(active&6)==0&&!live);
 reset();allocation_fail=1;assert(remove_one("/tmp/",0,0)==memFullErr&&!deletes&&!live);
 allocation_fail=0;assert(!remove_one("/tmp/",0,0)&&!live);
 reset();cancel_at=1;assert(remove_one("/tmp/",0,0)==130&&!deletes&&!live);
 cancel_at=0;assert(!remove_one("/tmp/",0,0)&&!live);
 reset();fail=-36;assert(remove_one("/alias/",0,0)==-36&&!deletes&&!live);
 fail=0;assert(!remove_one("/alias",0,0)&&!live);
 puts("PASS actual resolver + rm: final symlink slash rejection, target sentinel, ordinary slash directories, dot, low memory, I/O failure, cancellation/recovery");}
'''
for name,pre,tail,section in [('destination',pre,transfer_test,lambda s:extract(s,'transfer')),('remove',pre_rm,rm_test,lambda s:s[s.index('typedef struct RemoveFrame'):s.index('static int temporary_sibling')])]:
 for label,source in ([('baseline',old)] if __import__('os').environ.get('BASELINE_ONLY') else [('baseline',old),('fixed',current)]):
  path=w/f'{name}-{label}';path.with_suffix('.c').write_text(pre+section(source)+tail)
  subprocess.run(['cc','-g','-O1','-fsanitize=address,undefined',str(path.with_suffix('.c')),'-o',str(path)],check=True)
  run=subprocess.run([str(path)],capture_output=True,text=True);(w/f'{name}-{label}.txt').write_text(run.stdout+run.stderr)
  if label=='baseline':assert run.returncode!=0;print('REPRODUCED',name,'on 1965')
  else:print(run.stdout,end='');assert run.returncode==0,run.stderr
