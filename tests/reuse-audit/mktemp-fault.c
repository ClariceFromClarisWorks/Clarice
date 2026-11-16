/* Guest-only runtime boundary faults. Native HFS creation/removal stays real. */
#include <errno.h>
extern long cli_write(int,const void *,unsigned long);
long fixture_write(int fd,const void *data,unsigned long size){
 if(fd==1){errno=ENOSPC;return -1;}return cli_write(fd,data,size);
}
/* Directory creation uses the real app_yield. In this fixture the runtime's
   first yield is stdout flush, after ownership has been established. */
int fixture_yield(void){return 1;}
