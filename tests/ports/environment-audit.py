from pathlib import Path
import os,subprocess,json,tempfile
r=Path(__file__).resolve().parents[2];out=r/'outputs/shell-practical';rows=[]
for count,length,budget in [(30,4096,None),(30,4096,64),(30,4096,512),(30,4096,4096),(30,4096,16384),(3,5,None)]:
 env={**os.environ,'CLARICE_TEST_ENV_COUNT':str(count),'CLARICE_TEST_ENV_VALUE':'x'*length,'CLARICE_TEST_STATS':'1'}
 if budget:env['CLARICE_TEST_ALLOC_BUDGET']=str(budget)
 p=subprocess.run([str(r/'work/awk-host/awk'),'BEGIN { n=0; for(k in ENVIRON)n++; print n, length(ENVIRON["ENV'+str(count-1)+'"]) }'],env=env,capture_output=True)
 assert b'remaining=0' in p.stderr and (p.returncode!=0 if budget else p.returncode==0 and p.stdout==f'{count} {length}\n'.encode()),(count,length,budget,p.returncode,p.stdout,p.stderr)
 rows.append(dict(entries=count,value_bytes=length,budget=budget,status=p.returncode,stdout=p.stdout.decode(),accounting=p.stderr.decode()))
# Compile only the unchanged editor environment adapter, not a mock reimplementation.
s=(r/'src/ports/terminal/platform.c').read_text();s=s[s.index('extern int cli_environment'):s.index('/* Synchronous signal')]
source='''#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
static int large;
int cli_environment(int i,char *p,unsigned long cap){if(i==30)return 0;char name[32];snprintf(name,sizeof name,"ENV%d=",i);size_t n=strlen(name),v=large?1500:1;if(n+v+1>cap)return -1;memcpy(p,name,n);memset(p+n,'x',v);p[n+v]=0;return 1;}
'''+s+'''
int main(int argc,char **argv){large=argc>1;env_init();int imported=0;for(int i=0;i<30;i++){char name[32];snprintf(name,sizeof name,"ENV%d",i);if(port_curses_getenv(name))imported++;}char longvalue[1501];memset(longvalue,'x',1500);longvalue[1500]=0;int result=setenv("LONG",longvalue,1);printf("imported=%d long_set=%d errno=%d\\n",imported,result,errno);for(int i=0;i<24;i++)free(environment[i]);}
'''
w=r/'work/shell-practical';(w/'editor-env.c').write_text(source)
subprocess.run(['cc','-fsanitize=address,undefined',str(w/'editor-env.c'),'-o',str(w/'editor-env')],check=True)
editor=[]
for args in [[],['large']]:
 p=subprocess.run([str(w/'editor-env'),*args],capture_output=True);assert p.returncode==0,(p.stderr)
 editor.append(dict(large_values=bool(args),result=p.stdout.decode()))
assert editor[0]['result'].startswith('imported=18 long_set=-1') and editor[1]['result'].startswith('imported=0 long_set=-1')
(out/'environment.json').write_text(json.dumps(dict(awk=rows,unchanged_editor=editor),indent=2)+'\n');print('PASS elastic awk / exhaustion; confirmed editor 18-import, 24-slot, 1024-byte limits')
