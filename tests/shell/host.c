#define SH_HOST_TEST 1
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>
#define PATH_CAP 512
#define MAX_ARGS 32
#define fsRdPerm 1
#define fsRdWrPerm 3
#define fsFromStart 1
#define eofErr -39
#define dupFNErr -48
static short input_ref=-1,output_ref=-1,error_ref=-3;
static int output_error,shell_status,script_depth;
static char env_names[16][32],env_values[16][128],shell_cwd[PATH_CAP]="./",home_path[PATH_CAP]="./";
static FILE *refs[128];

static int cancel_after,polls;
static int app_yield(void){return cancel_after&&++polls>=cancel_after;}
static void app_quit(void){}
static void history_add(const char*s){(void)s;}
static void term_write(const char*s,long n){fwrite(s,1,n,stdout);}
static int FSWrite(short r,long*n,const void*b){size_t k=fwrite(b,1,*n,refs[r]);int e=k!=(size_t)*n;*n=k;return e;}
static int FSRead(short,long*,char*);
#include "../../src/streams.inc"
static void say(const char*s){emit(s,strlen(s));}
static const char *env_get(const char *name){for(int i=0;i<16;i++)if(!strcmp(env_names[i],name))return env_values[i];return "";}
static int env_set(const char *s){const char *e=strchr(s,'=');if(!e)return 1;for(int i=0;i<16;i++)if(!env_names[i][0]||(strlen(env_names[i])==(size_t)(e-s)&&!strncmp(s,env_names[i],e-s))){memcpy(env_names[i],s,e-s);env_names[i][e-s]=0;strcpy(env_values[i],e+1);return 0;}return 1;}
static void pstr(unsigned char*p,const char*s){*p=strlen(s);memcpy(p+1,s,*p);}
static void pathstr(char *s,const unsigned char*p){memcpy(s,p+1,*p);s[*p]=0;}
static int HCreate(int a,int b,const unsigned char*p,int c,int d){char s[256];(void)a;(void)b;(void)c;(void)d;pathstr(s,p);FILE*f=fopen(s,"wx");if(!f)return dupFNErr;fclose(f);return 0;}
static int open_path(const char*s,short mode,short*ref){for(int i=0;i<128;i++)if(!refs[i]){refs[i]=fopen(s,mode==fsRdPerm?"rb":"r+b");if(!refs[i])return -1;*ref=i;return 0;}return -1;}
static int HOpen(int a,int b,const unsigned char*p,short mode,short*ref){char s[256];(void)a;(void)b;pathstr(s,p);return open_path(s,mode,ref);}
static int HDelete(int a,int b,const unsigned char*p){char s[256];(void)a;(void)b;pathstr(s,p);return unlink(s);}
static int FSClose(short r){int rc=fclose(refs[r]);refs[r]=0;return rc;}
static int SetFPos(short r,int mode,long pos){(void)mode;return fseek(refs[r],pos,SEEK_SET);}
static int FSRead(short r,long*n,char*b){long count=fread(b,1,*n,refs[r]);int rc=count==*n?0:eofErr;*n=count;return rc;}
static int GetEOF(short r,long*n){long p=ftell(refs[r]);fseek(refs[r],0,SEEK_END);*n=ftell(refs[r]);fseek(refs[r],p,SEEK_SET);return 0;}
static int redirect(const char*s,int append){for(int i=0;i<128;i++)if(!refs[i]){refs[i]=fopen(s,append?"ab":"wb");if(!refs[i])return 1;output_ref=i;return 0;}return 1;}
typedef struct {struct {int ioFlAttrib;long ioFlLgLen;}hFileInfo;}CInfoPBRec;
static int resolve(const char*s,char*p){strcpy(p,s);return 0;}
static int info(const char*p,CInfoPBRec *b){struct stat s;if(stat(p,&s))return 1;b->hFileInfo.ioFlAttrib=S_ISDIR(s.st_mode)?16:0;b->hFileInfo.ioFlLgLen=s.st_size;return 0;}
static int run_args(int argc,char**av){if(!strcmp(av[0],"echo")){for(int i=1;i<argc;i++){if(i>1)say(" ");say(av[i]);}say("\n");return 0;}if(!strcmp(av[0],"cat")){FILE*f=argc==1?(input_ref>=0?refs[input_ref]:stdin):fopen(av[1],"rb");if(!f)return 1;char b[512];size_t n;while((n=fread(b,1,sizeof b,f)))emit(b,n);if(argc>1)fclose(f);return 0;}if(!strcmp(av[0],"export")){for(int i=1;i<argc;i++)if(env_set(av[i]))return 2;return 0;}return 127;}
#include "../../src/sh.inc"
int main(int argc,char **argv){if(argc<2)return 2;if(argc>2)cancel_after=atoi(argv[2]);return shell_run(argv[1]);}
