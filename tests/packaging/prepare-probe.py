"""Create an isolated, disposable startup measurement build. Production untouched."""
from pathlib import Path
import shutil
r=Path(__file__).resolve().parents[2];w=r/'work/packaging';s=w/'src'
assert 'static int install_commands(void)' in (r/'src/module.inc').read_text(), 'Historical probe requires the e67c569 source; use tests/distribution/prepare-probe.py for the separated core'
if s.exists():shutil.rmtree(s)
shutil.copytree(r/'src',s)
p=s/'CMakeLists.txt';p.write_text(p.read_text().replace('../work/build/Commands.r','/src/work/build/Commands.r'))
p=s/'module.inc';t=p.read_text();t=t.replace('static int install_commands(void){','static int package_install_actual(void){',1);t+='''
unsigned long package_ticks;long package_before,package_after;int package_index,package_skip;
static int install_commands(void){
 unsigned char path[]="\\pAudit:package-index";short ref;char value=0;long n=1;
 if(!HOpen(0,0,path,fsRdPerm,&ref)){FSRead(ref,&n,&value);FSClose(ref);}
 package_index=value;package_skip=value>=4;
 package_before=FreeMem();unsigned long t=TickCount();
 int e=package_skip?0:package_install_actual();
 package_ticks=TickCount()-t;package_after=FreeMem();return e;
}
''';p.write_text(t)
p=s/'app.c';t=p.read_text();pos=t.index('int main(void)');t=t[:pos]+'''
extern unsigned long package_ticks;extern long package_before,package_after;extern int package_index,package_skip;
static void package_report(unsigned long ready,unsigned long drawn,long heap){
 long largest=MaxBlock(),before=FreeMem();int loaded=0,count=Count1Resources('CLX1');
 SetResLoad(0);for(int i=1;i<=count;i++){Handle h=Get1IndResource('CLX1',i);if(h&&*h)loaded++;}SetResLoad(1);
 char b[400];int len=snprintf(b,sizeof b,"run=%d skip=%d ready=%lu drawn=%lu install=%lu before=%ld after=%ld heap=%ld largest=%ld clx_count=%d loaded=%d census_cost=%ld\\n",package_index,package_skip,ready,drawn,package_ticks,package_before,package_after,heap,largest,count,loaded,before-FreeMem());
 unsigned char path[]="\\pAudit:package-results";short ref;HCreate(0,0,path,'CLRC','TEXT');
 if(!HOpen(0,0,path,fsWrPerm,&ref)){SetFPos(ref,fsFromLEOF,0);long n=len;FSWrite(ref,&n,b);FSClose(ref);}
 unsigned char index[]="\\pAudit:package-index";HCreate(0,0,index,'CLRC','DATA');
 if(!HOpen(0,0,index,fsWrPerm,&ref)){char v=package_index+1;long n=1;FSWrite(ref,&n,&v);FSClose(ref);}FlushVol(0,0);
}
'''+t[pos:];t=t.replace('while(!done){\n#ifdef CLARICE_ACTIVITY_DIAGNOSTICS','draw();package_report(start_tick,TickCount()-launch,initial_free);\n while(!done){\n#ifdef CLARICE_ACTIVITY_DIAGNOSTICS');p.write_text(t)
print(s)
