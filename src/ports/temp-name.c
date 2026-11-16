/* Same exclusive-create strategy as the existing nano adapter. A generated
   name is never a promise until HCreate (via port_open O_EXCL) succeeds. */
#include "compat.h"
extern unsigned long cli_ticks(void);
int port_mkstemp(char *path){
 static unsigned long sequence;size_t n=strlen(path);
 if(n<6||memcmp(path+n-6,"XXXXXX",6)){errno=EINVAL;return -1;}
 char *suffix=path+n-6;const char *hex="0123456789abcdef";
 for(int tries=0;tries<32;tries++){
  unsigned long id=cli_ticks()+ ++sequence;
  for(int i=0;i<6;i++)suffix[i]=hex[(id>>(4*i))&15];
  int fd=port_open(path,O_RDWR|O_CREAT|O_EXCL,0600);
  if(fd>=0)return fd;if(errno!=EEXIST)return -1;
 }
 errno=EEXIST;return -1;
}
