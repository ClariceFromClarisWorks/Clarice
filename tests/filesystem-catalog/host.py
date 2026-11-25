"""Actual namespace and recursive remove; native catalog names, faults and retries."""
from pathlib import Path
import subprocess,tempfile
base=Path('tests/filesystem-native-remove/host.py').read_text();env={};exec(base[:base.index('helpers=')],env)
fn=env['function'];pre=env['pre'];a=pre.index('static const struct Node seed[]=');b=pre.index('static int fault',a)
entries=[('Disk:parent:',1),('Disk:parent:outer',1),('Disk:parent:outer:sentinel',0),('Disk:parent:outer:selected',1),('Disk:parent:outer:selected:..',1),('Disk:parent:outer:selected:..:.',0),('Disk:parent:outer:selected:.',1),('Disk:parent:outer:selected:.:..',0)]
for leaf in ['/','%','mid/name','tail/','%2E','%2E%2E','%2F','%25','%3A','..%2F..','...']:
 entries.append(('Disk:parent:outer:selected:'+leaf,0))
entries += [('Disk:parent:outer:selected:link',0),('Disk:parent:home',1),('Disk:parent:home:sentinel',0)]
N=len(entries)
seed='static const struct Node seed[]={\n'+',\n'.join('{"%s",%s,%d,1,%d,"DATA","FORK"}'%(p,'"/outer"' if p.endswith(':link') else '0',d,i+1) for i,(p,d) in enumerate(entries))+'};\n'
pre=pre[:a]+seed+f'static struct Node nodes[{N}];static int calls,fail_at,live,allocs,alloc_fail,polls,cancel_at,recursive=1,force;\n'+pre[b:]
pre=pre.replace('i<10','i<COUNT').replace('#define PATH_CAP 512',f'#define PATH_CAP 512\n#define COUNT {N}\n#define LAST {N-3}')
pre=pre.replace('return ++polls==cancel_at;', 'return !nodes[2].alive||++polls==cancel_at;')
test=r'''
static void reset(void){assert(!live);memcpy(nodes,seed,sizeof nodes);calls=fail_at=allocs=alloc_fail=polls=cancel_at=force=0;}
static void outside(void){int ids[]={0,1,2,COUNT-2,COUNT-1};for(unsigned i=0;i<sizeof ids/sizeof*ids;i++)assert(!memcmp(nodes+ids[i],seed+ids[i],sizeof*nodes));assert(!live);}
int main(int argc,char**argv){(void)argv;reset();int rc=remove_one("/outer/selected",0,0);
 if(argc>1){assert(rc==130&&!nodes[2].alive);puts("REPRODUCED1969: enumerated literal .. removes outside-root sentinel");return 0;}
 assert(!rc);for(int i=3;i<=LAST;i++)assert(!nodes[i].alive);outside();int total=calls,steps=polls,frames=allocs;
 for(int i=1;i<=total;i++){reset();fail_at=i;assert(remove_one("/outer/selected",0,0));outside();fail_at=0;force=1;assert(!remove_one("/outer/selected",0,0));outside();}
 for(int i=1;i<=steps;i++){reset();cancel_at=i;assert(remove_one("/outer/selected",0,0)==130);outside();cancel_at=0;force=1;assert(!remove_one("/outer/selected",0,0));outside();}
 for(int i=1;i<=frames;i++){reset();alloc_fail=i;assert(remove_one("/outer/selected",0,0)==memFullErr);outside();alloc_fail=0;force=1;assert(!remove_one("/outer/selected",0,0));outside();}
 reset();char encoded[100],decoded[32],native[512],canonical[512];
 const char *names[]={".","..","/","%","x/y","y/","%2E","%2e%2E","%2F","%3A","%00","...",".hidden","..hidden"};
 for(unsigned i=0;i<sizeof names/sizeof*names;i++){assert(!encode_name(names[i],encoded,sizeof encoded));assert(!decode_name(encoded,strlen(encoded),decoded)&&!strcmp(decoded,names[i]));char operand[512];snprintf(operand,sizeof operand,"/outer/selected/%s",encoded);assert(!resolve_ex(operand,native,canonical,0));char wanted[512];snprintf(wanted,sizeof wanted,"Disk:parent:outer:selected:%s",names[i]);assert(!strcmp(native,wanted));}
 assert(!resolve_ex("/outer/selected/..",native,canonical,0)&&!strcmp(canonical,"/outer"));
 assert(!resolve_ex("/outer/selected/.",native,canonical,0)&&!strcmp(canonical,"/outer/selected"));
 assert(!resolve_ex("native:Disk:parent:outer:selected:..",native,canonical,0)&&!strcmp(canonical,"/outer/selected/%2E%2E"));
 assert(decode_name("%3A",3,decoded)==bdNamErr&&decode_name(":",1,decoded)==bdNamErr&&decode_name("%00",3,decoded)==bdNamErr);
 char guard[8];memset(guard,'X',sizeof guard);assert(encode_name("..",guard,6)==bdNamErr&&guard[6]=='X');assert(!encode_name("..",guard,7)&&!strcmp(guard,"%2E%2E")&&guard[7]=='X');
 reset();assert(remove_one("/outer/selected/link/",0,0));outside();assert(!remove_one("/outer/selected/link",0,0));outside();
 reset();assert(!remove_one("native:Disk:parent:outer:selected:.",0,0));outside();assert(nodes[4].alive&&!nodes[6].alive&&!nodes[7].alive);
 printf("PASS catalog literals, navigation, single decoding, sibling forks/metadata; %d native faults, %d cancellation points, %d allocation failures, retry and zero survivors\n",total,steps,frames);
}
'''
for label in ('baseline','fixed'):
 ns=subprocess.check_output(['git','show','70ca55c:src/namespace.inc'],text=True) if label=='baseline' else Path('src/namespace.inc').read_text()
 helpers=''.join(fn(ns,n) for n in ('same_name','encode_name','decode_name','native_path','unix_from_native','parent_path','resolve_full','resolve_ex','protected_path'))
 h=Path('src/commands/mac-files.h').read_text();helpers+=''.join(fn(h,n) for n in ('checked','join_path'))
 s=Path('src/commands/files.c').read_text();body=s[s.index('typedef struct RemoveFrame'):s.index('static int temporary_sibling')]
 with tempfile.TemporaryDirectory() as d:
  p=Path(d);(p/'test.c').write_text(pre+helpers+body+test)
  subprocess.run(['cc','-g','-O1','-fsanitize=address,undefined',str(p/'test.c'),'-o',str(p/'test')],check=True)
  subprocess.run([str(p/'test')]+(['baseline'] if label=='baseline' else []),check=True)
