"""Disposable diagnostic core: startup timing and real heap-pressure acceptance."""
from pathlib import Path
import shutil
r=Path(__file__).resolve().parents[2];w=r/'work/distribution';s=w/'src'
if s.exists():shutil.rmtree(s)
shutil.copytree(r/'src',s)
p=s/'command-install.inc';t=p.read_text().replace('static int appinstall_command(int argc,char **av){','''static Ptr distribution_reserve;
static int appinstall_command(int argc,char **av){
 if(argc==3&&!strcmp(av[1],"test-reserve")){
  if(distribution_reserve){DisposePtr(distribution_reserve);distribution_reserve=0;}
  long n=atol(av[2]);if(n>0)distribution_reserve=NewPtr(n);
  return n>0&&!distribution_reserve?1:0;
 }
''');p.write_text(t)
p=s/'app.c';t=p.read_text();pos=t.index('int main(void)');t=t[:pos]+'''
static void distribution_report(unsigned long ready,unsigned long drawn,long heap){
 char b[240];int n=snprintf(b,sizeof b,"ready=%lu drawn=%lu heap=%ld largest=%ld clx=%d\\n",ready,drawn,heap,MaxBlock(),Count1Resources('CLX1'));
 unsigned char path[]="\\pAudit:distribution-startup";short ref;HCreate(0,0,path,'CLRC','TEXT');
 if(!HOpen(0,0,path,fsWrPerm,&ref)){SetFPos(ref,fsFromLEOF,0);long bytes=n;FSWrite(ref,&bytes,b);FSClose(ref);}FlushVol(0,0);
}
'''+t[pos:];needle='while(!done){\n#ifdef CLARICE_ACTIVITY_DIAGNOSTICS';assert needle in t;t=t.replace(needle,'draw();distribution_report(start_tick,TickCount()-launch,initial_free);\n '+needle);p.write_text(t)
print(s)
