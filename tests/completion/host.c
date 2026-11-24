#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define PATH_CAP 512
#define LINE_CAP 1024
#define fnfErr -43
typedef struct {struct {unsigned char *ioNamePtr;short ioVRefNum;long ioDirID;short ioFDirIndex;int ioFlAttrib;} hFileInfo;} CInfoPBRec;
static const char *commands[]={"echo","exit",0};
static const char *names[1100];static int entries,reads,polls,abort_at,questions,yes_pages,read_error,error_at;
static char output[32768];
static int width=80;static int app_columns(void){return width;}
static int app_completion_poll(void){return abort_at&&++polls>=abort_at;}
static int app_completion_more(void){questions++;return questions<=yes_pages;}
static void term_puts(const char *s){assert(strlen(output)+strlen(s)<sizeof output);strcat(output,s);}
static int resolve_ex(const char *s,char *n,char *c,int x){(void)x;strcpy(n,s);strcpy(c,s);size_t z=strlen(c);if(z>1&&c[z-1]=='/')c[z-1]=0;return 0;}
static int directory(const char *s,short *v,long *id){(void)s;*v=1;*id=2;return 0;}
static int volume_name(int i,char *s){if(i>entries)return fnfErr;strcpy(s,names[i-1]);return 0;}
static int encode_name(const char *s,char *o,size_t n){if(strlen(s)>=n)return 1;strcpy(o,s);return 0;}
static int PBGetCatInfoSync(CInfoPBRec *p){reads++;if(read_error&&(!error_at||reads==error_at))return read_error;int i=p->hFileInfo.ioFDirIndex-1;if(i>=entries)return fnfErr;size_t n=strlen(names[i]);p->hFileInfo.ioNamePtr[0]=n;memcpy(p->hFileInfo.ioNamePtr+1,names[i],n);return 0;}
static int program_kind(const char *s){(void)s;return 1;}
static int command_path(const char *s,char *out,int x){(void)x;snprintf(out,PATH_CAP,"/bin/%s",s);return 1;}
static int is_link(CInfoPBRec *p){(void)p;return 0;}
static const char *env_get(const char *n){(void)n;return "/bin";}
static int path_entry(const char **p,char *out){if(!**p)return 0;strcpy(out,*p);*p="";return 1;}
#include "../../src/completion.inc"
static void complete_functions(Completion *c){(void)c;}
static void setup(void){entries=reads=polls=abort_at=questions=yes_pages=read_error=error_at=0;output[0]=0;}
static int run(char *s){int cursor=strlen(s);return shell_complete(s,&cursor);}
int main(void){char line[LINE_CAP];
 setup();strcpy(line,"ec");assert(run(line)==1&&!strcmp(line,"echo "));assert(!*output);
 setup();names[0]="My File";entries=1;strcpy(line,"cat my");assert(run(line)==1&&!strcmp(line,"cat My\\ File "));
 strcpy(line,"cat \"my");assert(run(line)==1&&!strcmp(line,"cat \"My File\" "));
 strcpy(line,"cat 'my");assert(run(line)==1&&!strcmp(line,"cat 'My File' "));
 strcpy(line,"cat \"my\"");int cur=7;assert(shell_complete(line,&cur)==1&&!strcmp(line,"cat \"My File\" "));
 names[0]="O'Brien";strcpy(line,"cat 'o");assert(run(line)==1&&!strcmp(line,"cat 'O'\\''Brien' "));
 setup();names[0]="alpha";names[1]="alpine";entries=2;strcpy(line,"cat al");assert(run(line)==2&&!strcmp(line,"cat alp"));assert(!*output);assert(run(line)==2&&strstr(output,"alpha")&&strstr(output,"alpine"));
 setup();static char many[1000][16];for(int i=0;i<1000;i++){snprintf(many[i],16,"e%04d",i);names[i]=many[i];}entries=1000;
 strcpy(line,"cat e");assert(run(line)==1000&&!strcmp(line,"cat e0"));assert(!*output&&reads==1001);
 reads=0;assert(run(line)>1&&questions==1&&reads==COMPLETION_PAGE+1);assert(strstr(output,"e0015")&&!strstr(output,"e0016"));
 output[0]=0;questions=0;yes_pages=1;reads=0;assert(run(line)>1&&questions==2&&reads==COMPLETION_PAGE*2+1);assert(strstr(output,"e0031")&&!strstr(output,"e0032"));
 shell_completion_reset();output[0]=0;questions=0;assert(run(line)>1&&!output[0]&&!questions);
 setup();names[0]="ea";names[1]="ez";entries=1000;strcpy(line,"cat e");{int n=run(line);assert(n==2&&reads==2&&!strcmp(line,"cat e"));}
 setup();entries=1000;for(int i=0;i<1000;i++)names[i]=many[i];abort_at=4;strcpy(line,"cat unmatched");assert(run(line)==-1&&!strcmp(line,"cat unmatched")&&reads==32);
 setup();entries=1000;for(int i=0;i<1000;i++)names[i]=many[i];abort_at=4;strcpy(line,"cat e");assert(run(line)==-1&&!strcmp(line,"cat e")&&reads==32);
 setup();entries=1000;for(int i=0;i<1000;i++)names[i]=many[i];read_error=-108;error_at=9;strcpy(line,"cat e");assert(run(line)==-1&&!strcmp(line,"cat e")&&reads==9);
 setup();read_error=-108;strcpy(line,"cat memory");assert(run(line)==-1&&!strcmp(line,"cat memory"));
 setup();names[0]="file";entries=1;strcpy(line,"cat fi");assert(run(line)==1&&!strcmp(line,"cat file "));
 setup();names[0]="Disk Space";entries=1;strcpy(line,"cd /mnt/disk");assert(run(line)==1&&!strcmp(line,"cd /mnt/Disk\\ Space/"));
 setup();entries=100;for(int i=0;i<100;i++)names[i]="abcdefghijklmnopqrstuvwxy1234567";width=26;
 strcpy(line,"cat a");assert(run(line)>1);assert(run(line)>1&&questions==1&&reads<=105);width=80;
 for(int i=0;i<1000;i++){setup();names[0]="only";entries=1;strcpy(line,"cat on");assert(run(line)==1&&!strcmp(line,"cat only "));}
 /* Expansion overflow must leave the complete input untouched. */
 setup();names[0]="'''''''''''''''''''''''''''''''";entries=1;strcpy(line,"cat ");
 for(int i=4;i<504;i+=2){line[i]='\\';line[i+1]='\'';}line[504]='/';line[505]='\'';line[506]=0;
 char original[LINE_CAP];strcpy(original,line);assert(run(line)==0&&!strcmp(line,original));
 puts("PASS completion: LCP, single/second Tab, bounded pages, case/quotes/spaces/volumes, 1000 entries, nonmatch cancellation, native error recovery; no allocator used");
}
