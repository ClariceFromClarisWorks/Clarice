/* Native Memory Manager snapshot, no retained allocation. */
#include "../hardware.h"
#include "../memory-info.h"
#include <stdio.h>
#include <string.h>
extern long cli_write(int,const void *,unsigned long);
static int output(int fd,const char *s){return cli_write(fd,s,strlen(s))<0;}
int clarice_main(int argc,char **argv){unsigned long divisor=1024;const char *unit="KiB";
 if(argc==2&&!strcmp(argv[1],"--help"))return output(1,
 "free [-b|-k|-m|-h] [--]\nReport installed RAM and Clarice's current application heap.\nDefault -k: KiB; -b: bytes; -m: MiB; -h: human readable. Last wins.\nFree and largest blocks are not machine-wide free physical RAM.\nAfter-purge values include reclaimable purgeable handles; no purge is\nrequested. Heap extent excludes the application stack/code outside it;\nSystem 6 partition-wide capacity is not claimed. Values include this\ncommand's temporary image. No Linux swap/buffers/process statistics.\nExample: free -b\nStatus: 0 success, 1 query/output error, 2 invalid option. See status.\n");
 for(int i=1;i<argc;i++){
 if(!strcmp(argv[i],"--")){if(i+1!=argc)goto usage;break;}
 if(!strcmp(argv[i],"-b")){divisor=1;unit="bytes";}
 else if(!strcmp(argv[i],"-k")){divisor=1024;unit="KiB";}
 else if(!strcmp(argv[i],"-m")){divisor=1048576;unit="MiB";}
 else if(!strcmp(argv[i],"-h")){divisor=0;}
 else goto usage;
 }
 ClariceHardware hw;ClariceMemory m;char line[100];
 if(cli_hardware(&hw,sizeof hw)||cli_memory(&m,sizeof m)){output(2,"free: memory query unavailable\n");return 1;}
 const char *labels[]={"Installed RAM","Application heap extent","Free heap","Largest free block","Free after purging","Largest after purging"};
 unsigned long values[]={hw.ram,m.heap_extent,m.free_bytes,m.largest,m.after_purge,m.largest_after_purge};
 for(int i=0;i<6;i++){if(divisor)snprintf(line,sizeof line,"%-25s %lu %s\n",labels[i],values[i]/divisor,unit);else{unsigned long d=values[i]>=1048576?1048576:values[i]>=1024?1024:1;snprintf(line,sizeof line,"%-25s %lu.%02lu %s\n",labels[i],values[i]/d,(values[i]%d)*100/d,d==1048576?"MiB":d==1024?"KiB":"B");}if(output(1,line))return 1;}return 0;
usage:output(2,"usage: free [-b|-k|-m|-h] [--]; use free --help\n");return 2;
}
