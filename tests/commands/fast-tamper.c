/* Disposable Snow acceptance fixture only: alter text, never instructions.
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
 if(!strcmp(argv[1],"/etc/clarice/clx-nano.ok")){char b[96];int n=snprintf(b,sizeof b,"receipt logical=%ld physical=%ld\n",meta.hFileInfo.ioFlLgLen,meta.hFileInfo.ioFlPyLen);return cli_write(1,b,n)==n?0:1;}
 if(strcmp(argv[1],"/bin/fastbad")||meta.hFileInfo.ioFlLgLen>512)return 3;
 char data[512];short ref;long n=meta.hFileInfo.ioFlLgLen,want=n;
 if(cli_native_open(path,0,0,&ref))return 4;int e=cli_native_read(ref,data,&n),c=cli_native_close(ref);if(e||c||n!=want)return 5;
 long at;for(at=0;at+22<=n;at++)if(!memcmp(data+at,"independent command v2",22))break;if(at+22>n)return 6;data[at]='X';
 if(cli_native_open(path,0,1,&ref))return 7;e=cli_native_write(ref,data,&n);c=cli_native_close(ref);if(e||c||n!=want)return 8;
 return cli_native_set(path,&meta)?9:0;
}
