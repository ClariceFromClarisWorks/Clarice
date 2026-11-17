"""Exercise actual long-list formatting with invocation-local timestamp reuse."""
from pathlib import Path
import subprocess,tempfile
root=Path.cwd()
s=(root/'src/listing.inc').read_text()
s=s[s.index('typedef struct'):s.index('static unsigned long list_ticks')]
head=r'''
#include <stdio.h>
#include <string.h>
#include <assert.h>
#define PATH_CAP 256
#define LS_START ((void)0)
#define LS_PHASE(n) ((void)0)
#define ls_emit emit
#define ls_yield(p) app_yield()
typedef struct {unsigned long type,creator;unsigned short fdFlags;} FInfo;
typedef struct {struct {unsigned char ioFlAttrib;FInfo ioFlFndrInfo;unsigned long ioFlLgLen,ioFlRLgLen,ioFlCrDat;} hFileInfo;struct {unsigned long ioDrCrDat;} dirInfo;} CInfoPBRec;
static int output_error,used,calls;
static char output[1024];
static void emit(const char *s,long n){assert(used+n<1024);memcpy(output+used,s,n);used+=n;output[used]=0;}
static void say(const char *s){emit(s,strlen(s));}
static int app_yield(void){return 0;}
static int app_page(void){return 0;}
static int app_columns(void){return 80;}
static void term_present(void){}
static int is_link(const CInfoPBRec *p){return p->hFileInfo.ioFlFndrInfo.type==7;}
static void format_size(char *s,unsigned long n,int h){(void)h;snprintf(s,32,"%lu",n);}
#define format_created reference_created
'''
head+=(root/'src/commands/created.c').read_text()+'\n#undef format_created\n'
head+=(root/'src/commands/decimal.c').read_text()
head+='\nstatic void format_created(char *s,unsigned long t){calls++;reference_created(s,t);}\n'
tail=r'''
/* Independent printf oracle for the existing byte-for-byte column contract. */
static void oracle(char *out,Listing *l,const char *name,const CInfoPBRec *pb){
 int dir=!pb||(pb->hFileInfo.ioFlAttrib&16);unsigned flags=pb?pb->hFileInfo.ioFlFndrInfo.fdFlags:0;
 char kind[6],data[32]="",resource[32]="",created[17]="                ",type[10];
 kind[0]=dir?'d':is_link(pb)?'l':'-';kind[1]=pb&&(pb->hFileInfo.ioFlAttrib&1)?'L':'-';kind[2]=flags&0x4000?'I':'-';kind[3]=flags&0x1000?'N':'-';kind[4]=flags&0x2000?'B':'-';kind[5]=0;
 if(pb&&!dir){if(l->human){format_size(data,pb->hFileInfo.ioFlLgLen,1);format_size(resource,pb->hFileInfo.ioFlRLgLen,1);}else{snprintf(data,sizeof data,"%lu",pb->hFileInfo.ioFlLgLen);snprintf(resource,sizeof resource,"%lu",pb->hFileInfo.ioFlRLgLen);}}
 if(pb)reference_created(created,dir?pb->dirInfo.ioDrCrDat:pb->hFileInfo.ioFlCrDat);
 const char *slash=dir&&strcmp(name,".")&&strcmp(name,"..")?"/":"";
 if(l->detail==1)snprintf(out,1024,"%.3s %10s %10s %s %s%s\n",kind,data,resource,created,name,slash);
 else{for(int j=0;j<8;j++){unsigned char c=dir?'-':((unsigned char*)&pb->hFileInfo.ioFlFndrInfo)[j];type[j+(j>=4)]=(c>=32&&c<127)?c:'.';}type[4]=' ';type[9]=0;
  char flag[5];if(pb)snprintf(flag,sizeof flag,"%04X",flags);else strcpy(flag,"----");
  snprintf(out,1024,"%s %s %10s %10s %s %s %s%s\n",kind,flag,data,resource,type,created,name,slash);
 }
}
int main(void){Listing l={0};CInfoPBRec p={0};char expected[1024];unsigned long times[]={0,0,59,60,60,86400,0xffffffffUL,0xffffffffUL,0,1};
unsigned long sizes[]={0,1,9,10,99,100,65535,65536,0x80000000UL,0xffffffffUL};
const char *names[]={"space name",".","..","encoded%2Fname"};
for(int detail=1;detail<=2;detail++)for(int human=0;human<2;human++){
 memset(&l,0,sizeof l);l.detail=detail;l.human=human;
 for(int i=0;i<1000;i++){
  p.hFileInfo.ioFlAttrib=(i%3)?0:16;p.hFileInfo.ioFlFndrInfo.fdFlags=i;
  p.hFileInfo.ioFlLgLen=sizes[i%10];p.hFileInfo.ioFlRLgLen=sizes[(i/10)%10];
  p.hFileInfo.ioFlCrDat=p.dirInfo.ioDrCrDat=times[i%10];
  const char *name=names[i%4];const CInfoPBRec *entry=i%17?&p:0;
  used=0;assert(!list_item(&l,name,entry));oracle(expected,&l,name,entry);assert(!strcmp(expected,output));
  Listing cold={0};cold.detail=detail;cold.human=human;used=0;
  assert(!list_item(&cold,name,entry));assert(!strcmp(expected,output));assert(used&&output[used-1]=='\n');
 }
}
memset(&l,0,sizeof l);l.detail=1;p.hFileInfo.ioFlAttrib=0;p.hFileInfo.ioFlCrDat=0;calls=0;
for(int i=0;i<100;i++){used=0;assert(!list_item(&l,"same",&p));}assert(calls==1);
p.hFileInfo.ioFlCrDat=1;used=0;assert(!list_item(&l,"changed",&p));assert(calls==2);
puts("PASS: 4000 cached/cold long listings match independent printf oracle; mixed dates, directories, virtual entries, flags, full 32-bit sizes, human/full modes, dot names and newline boundaries; exact-key invalidation and zero timestamp");}
'''
with tempfile.TemporaryDirectory() as t:
 p=Path(t);(p/'test.c').write_text(head+s+tail)
 subprocess.run(['cc','-fsanitize=address,undefined','-Wno-multichar',str(p/'test.c'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
