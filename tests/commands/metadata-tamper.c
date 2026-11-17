/* Disposable Snow acceptance fixture only: alter an unused relocation target, never instructions.
   Demonstrates the documented unchanged-metadata risk and strict rejection. */
#include <Files.h>
#include <string.h>
#include <stdio.h>
extern int resolve(const char *,char *),info(const char *,CInfoPBRec *);
extern int cli_native_open(const char *,int,int,short *),cli_native_close(short);
extern int cli_native_read(short,void *,long *),cli_native_write(short,const void *,long *);
extern int cli_native_set(const char *,CInfoPBRec *);
extern long cli_write(int,const void *,unsigned long);
int clarice_main(int argc,char **argv){
 char path[512];CInfoPBRec meta;if(argc!=2||resolve(argv[1],path)||info(path,&meta))return 2;
 if(strcmp(argv[1],"/bin/lfprobe")||meta.hFileInfo.ioFlLgLen!=56)return 3;
 char data[512];short ref;long n=meta.hFileInfo.ioFlLgLen,want=n;
 if(cli_native_open(path,0,0,&ref))return 4;int e=cli_native_read(ref,data,&n),c=cli_native_close(ref);if(e||c||n!=want)return 5;
 if(memcmp(data,"CLX1",4)||data[51]!=1)return 6;data[55]=2;
 if(cli_native_open(path,0,1,&ref))return 7;e=cli_native_write(ref,data,&n);c=cli_native_close(ref);if(e||c||n!=want)return 8;
 return cli_native_set(path,&meta)?9:0;
}
