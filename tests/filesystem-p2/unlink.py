"""Production unlink adapter and namespace resolver, with native-boundary faults."""
from pathlib import Path
import subprocess
r=Path.cwd();w=r/'work/filesystem-p2';w.mkdir(parents=True,exist_ok=True)
a={};s=(r/'tests/filesystem-fixes/host.py').read_text();exec(s[:s.index('ns=Path(')],a)
b={};s=(r/'tests/filesystem-confidence/empty-path.py').read_text();exec(s[:s.index('extra=')],b)
pre=b['preamble'].replace('static int calls,fail;','static int calls,fail;static unsigned active=0x3ff;static int deletes;')
pre=pre.replace('if(!strcasecmp(s,nodes[i].path))','if((active&(1u<<i))&&!strcasecmp(s,nodes[i].path))')
pre=pre.replace('{"/tmp/up","..",0}','{"/tmp/dangling","absent",0}')
pre+=b['code']+r'''
static int resolve(const char*p,char*q){return resolve_ex(p,q,0,1);}
static int port_errno(int e){assert(e);return -1;}
static void pstr(unsigned char *p,const char*s){assert(strlen(s)<256);p[0]=strlen(s);memcpy(p+1,s,p[0]);}
static int HDelete(int a,int b,unsigned char*p){(void)a;(void)b;char name[256];memcpy(name,p+1,p[0]);name[p[0]]=0;CInfoPBRec pb;int e=info(name,&pb);if(e)return e;deletes++;active&=~(1u<<pb.identity);return 0;}
'''
test=r'''
int main(void){char *links[]={"/tmp/ref","/alias","/tmp/dangling"};int ids[]={7,6,8};
 for(int i=0;i<3;i++){active=0x3ff;deletes=0;assert(!cli_remove(links[i]));assert(deletes==1&&!(active&(1u<<ids[i]))&&(active&6)==6);}
 for(int i=0;i<3;i++){char path[512];snprintf(path,sizeof path,"%s///",links[i]);active=0x3ff;deletes=0;assert(cli_remove(path)==-1&&!deletes&&active==0x3ff);}
 active=0x3ff;deletes=0;assert(!cli_remove("/tmp/file")&&deletes==1&&!(active&4));assert(cli_remove("/tmp/file")==-1&&deletes==1);
 active=0x3ff;assert(!cli_remove("/mnt/Test 2///"));
 active=0x3ff;deletes=0;fail=-36;assert(cli_remove("/tmp/ref")==-1&&!deletes);fail=0;assert(!cli_remove("/tmp/ref"));
 assert(cli_remove("")==-1&&cli_remove("///")==-1&&cli_remove("native:")==-1);
 active=0x3ff;deletes=0;assert(cli_remove("native:/tmp/ref:")==-1&&!deletes);assert(!cli_remove("native:/tmp/ref"));
 puts("PASS portable unlink: file/directory/dangling symlinks, slash guards, regular/temp cleanup, native errors and retry");}
'''
for label,source in [('baseline',subprocess.check_output(['git','show','15f2c3e:src/port-files.inc'],text=True)),('fixed',(r/'src/port-files.inc').read_text())]:
 p=w/f'unlink-{label}';p.with_suffix('.c').write_text(pre+a['function'](source,'cli_remove')+test)
 subprocess.run(['cc','-g','-O1','-fsanitize=address,undefined',str(p.with_suffix('.c')),'-o',str(p)],check=True)
 run=subprocess.run([str(p)],capture_output=True,text=True);(w/(label+'-unlink.txt')).write_text(run.stdout+run.stderr)
 if label=='baseline':assert run.returncode;print('REPRODUCED portable unlink on1966')
 else:print(run.stdout,end='');assert not run.returncode,run.stderr
