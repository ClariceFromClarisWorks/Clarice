/* Runs through the production portable runtime; intentionally leaves resources
   open in failure/cancellation modes so invocation cleanup is exercised. */
#include "compat.h"
#define assert(x) do {if(!(x)){fprintf(stderr,"stream check failed at %d\n",__LINE__);return 3;}} while(0)
extern FILE *port_fdopen(int,const char *);
extern int port_fileno(FILE *);
int upstream_main(int argc,char **argv){
 int n=argc>2?atoi(argv[2]):40;if(n<1||n>40)return 2;int sequential=argc>3;FILE *fp[40];int fd[40];char c;
 for(int i=0;i<n;i++){fp[i]=fopen(argv[1],"r");if(!fp[i]){fprintf(stderr,"fopen %d errno %d\n",i,errno);return 2;}}
 for(int i=0;i<n;i++){assert(fgetc(fp[i])=='x');}
 if(sequential)for(int i=0;i<n;i++)assert(!fclose(fp[i]));
 for(int i=0;i<n;i++){fd[i]=open(argv[1],O_RDONLY);if(fd[i]<0){fprintf(stderr,"open %d errno %d\n",i,errno);return 2;}}
 for(int i=0;i<n;i++){assert(read(fd[i],&c,1)==1&&c=='x');}
 for(int i=0;i<n;i+=2){if(!sequential)assert(!fclose(fp[i]));assert(!close(fd[i]));}
 for(int i=0;i<n;i+=2){if(!sequential){fp[i]=fopen(argv[1],"r");if(!fp[i])return 2;}fd[i]=open(argv[1],O_RDONLY);if(fd[i]<0){fprintf(stderr,"open %d errno %d\n",i,errno);return 2;}}
 for(int i=0;i<n;i++){FILE *f=port_fdopen(fd[i],"r");if(!f)return 2;assert(port_fileno(f)==fd[i]);int rc=fclose(f);if(rc){fprintf(stderr,"fd %d close %d errno %d\n",fd[i],rc,errno);return 3;}assert(read(fd[i],&c,1)==-1);}
 if(!sequential)for(int i=0;i<n;i++)assert(!fclose(fp[i]));
 return 0;
}
