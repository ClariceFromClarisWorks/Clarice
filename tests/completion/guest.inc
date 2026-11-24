/* Test-only matched guest timing; no normal-build command or state. */
static int completion_probe(void){
 static const int sizes[]={1,10,100,1000};char line[LINE_CAP],report[256];
 for(int run=0;run<3;run++)for(int k=0;k<4;k++){
  snprintf(line,sizeof line,"cat /tmp/completion/n%d/e",sizes[k]);int cursor=strlen(line);long heap=FreeMem(),largest=MaxBlock();unsigned long t=TickCount();int n=shell_complete(line,&cursor);unsigned long dt=TickCount()-t;
  snprintf(report,sizeof report,"complete run=%d entries=%d matches=%d ticks=%lu heap=%ld largest=%ld retained=%ld line=%.100s\n",run,sizes[k],n,dt,heap,largest,heap-FreeMem(),line);say(report);
  t=TickCount();n=shell_complete(line,&cursor);dt=TickCount()-t;
  snprintf(report,sizeof report,"second run=%d entries=%d matches=%d ticks=%lu\n",run,sizes[k],n,dt);say(report);
 }
 for(int n=8;n<=64;n*=2){unsigned long t=TickCount();for(int i=0;i<n;i++)term_puts("entry-000000  ");term_puts("\n");term_present();snprintf(report,sizeof report,"display entries=%d ticks=%lu\n",n,TickCount()-t);say(report);}
 for(int mode=0;mode<2;mode++){
  {char reset[LINE_CAP]="cat /tmp/completion/no-such";int pos=strlen(reset);shell_complete(reset,&pos);}
  snprintf(line,sizeof line,"cat /tmp/completion/n1000/%s",mode?"z":"e0");int cursor=strlen(line);unsigned long t=TickCount();int n=shell_complete(line,&cursor);snprintf(report,sizeof report,"scan prefix=%s matches=%d ticks=%lu\n",mode?"z":"e0",n,TickCount()-t);say(report);
 }
 {short v;long id;int e=directory("/tmp/completion/n1000",&v,&id);unsigned long t=TickCount();int n=0;
  for(int i=1;!e;i++){CInfoPBRec pb;unsigned char name[256];memset(&pb,0,sizeof pb);pb.hFileInfo.ioNamePtr=name;pb.hFileInfo.ioVRefNum=v;pb.hFileInfo.ioDirID=id;pb.hFileInfo.ioFDirIndex=i;e=PBGetCatInfoSync(&pb);if(!e)n++;}
  snprintf(report,sizeof report,"catalog-only entries=%d ticks=%lu\n",n,TickCount()-t);say(report);
 }
 for(int mode=0;mode<3;mode++){extern int app_completion_probe(unsigned long *,int);unsigned long out[5];int good=app_completion_probe(out,mode);snprintf(report,sizeof report,"input mode=%d correct=%d event_ticks=%lu response_ticks=%lu heap_before=%lu heap_after=%lu largest=%lu\n",mode,good,out[0],out[1],out[2],out[3],out[4]);say(report);}
 return 0;
}
