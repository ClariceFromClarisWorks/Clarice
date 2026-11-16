/* Same exclusive-create strategy as the existing nano adapter. A generated
   name is never a promise until HCreate (via port_open O_EXCL) succeeds. */
#include "compat.h"
extern unsigned long cli_ticks(void);
/* Only mktemp registers a result; other port users retain their own ownership.
   Allocate before creating anything; cleanup runs after a cancellation longjmp. */
extern int cli_remove(const char *);
extern long cli_write(int,const void *,unsigned long);
static char *unreported;static int result_owned;
void port_mktemp_track(const char *path){
 unreported=port_strdup(path);result_owned=0;if(!unreported){fputs("mktemp: insufficient memory tracking temporary item\n",port_standard(2));port_exit(2);}
}
static void result_created(const char *path){if(unreported){strcpy(unreported,path);result_owned=1;}}
void port_mktemp_commit(void){unreported=NULL;result_owned=0;}
int port_mktemp_cleanup(void){
 int error=0;
 if(unreported&&result_owned&&cli_remove(unreported)){
  const char *message="mktemp: cleanup failed; temporary item retained: ";
  cli_write(2,message,strlen(message));cli_write(2,unreported,strlen(unreported));cli_write(2,"\n",1);error=1;
 }
 unreported=NULL;result_owned=0;return error;
}
int port_mkstemp(char *path){
 static unsigned long sequence;size_t n=strlen(path);
 if(n<6||memcmp(path+n-6,"XXXXXX",6)){errno=EINVAL;return -1;}
 char *suffix=path+n-6;const char *hex="0123456789abcdef";
 for(int tries=0;tries<32;tries++){
  unsigned long id=cli_ticks()+ ++sequence;
  for(int i=0;i<6;i++)suffix[i]=hex[(id>>(4*i))&15];
  int fd=port_open(path,O_RDWR|O_CREAT|O_EXCL,0600);
  if(fd>=0){result_created(path);return fd;}if(errno!=EEXIST)return -1;
 }
 errno=EEXIST;return -1;
}

/* Native directory creation is exclusive: dupFNErr never means success.
   Resolve the parent through Clarice; never fall back to a default volume. */
extern int resolve_ex(const char *,char *,char *,int),protected_path(const char *);
extern int cli_native_mkdir(const char *),app_yield(void);
char *port_mkdtemp(char *path){
 static unsigned long sequence;size_t n=strlen(path);
 char native[512],canonical[512];
 if(n<6||memcmp(path+n-6,"XXXXXX",6)){errno=EINVAL;return NULL;}
 char *suffix=path+n-6;const char *hex="0123456789abcdef";
 for(int tries=0;tries<32;tries++){
  if(app_yield())port_exit(130);
  unsigned long id=cli_ticks()+ ++sequence;
  for(int i=0;i<6;i++)suffix[i]=hex[(id>>(4*i))&15];
  int e=resolve_ex(path,native,canonical,0);
  if(!e&&protected_path(canonical)){errno=EACCES;return NULL;}
  if(!e)e=cli_native_mkdir(native);
  if(!e){result_created(path);return path;}
  if(e==-48)continue; /* dupFNErr: includes existing files and directories. */
  switch(e){
  case -43:case -120:errno=ENOENT;break;
  case -44:case -45:case -46:case -54:errno=EACCES;break;
  case -34:errno=ENOSPC;break;case -108:errno=ENOMEM;break;
  case -37:errno=ENAMETOOLONG;break;default:errno=EIO;break;
  }
  return NULL;
 }
 errno=EEXIST;return NULL;
}
