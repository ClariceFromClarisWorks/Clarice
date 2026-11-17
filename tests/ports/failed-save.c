/* Reproduce the current nano-tiny save sequence; this is a risk probe,
   not a passing recovery test. Run only on a disposable host fixture. */
#include "compat.h"
extern FILE *port_fdopen(int,const char *);
int upstream_main(int argc,char **argv){
 if(argc!=2)return 10;
 int fd=open(argv[1],O_WRONLY|O_CREAT|O_TRUNC,0600);if(fd<0)return 11;
 FILE *f=port_fdopen(fd,"wb");if(!f){close(fd);return 12;}
 fputs("replacement",f);
 if(fclose(f)==EOF){fprintf(stderr,"save failed: %s\n",strerror(errno));return 2;}
 return 0;
}
