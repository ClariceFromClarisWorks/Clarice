#include "compat.h"
extern FILE *port_fdopen(int,const char *);
int upstream_main(int argc,char **argv){
 if(argc!=2)return 10;
 FILE *f=fopen(argv[1],"wb+");if(!f)return 11;
 if(fwrite("abcdef",1,6,f)!=6||fseek(f,2,SEEK_SET)||fputc('X',f)==EOF||fseek(f,0,SEEK_SET))return 12;
 char b[7]={0};if(fread(b,1,6,f)!=6||strcmp(b,"abXdef")||fclose(f))return 13;
 f=fopen(argv[1],"r+b");if(!f||fseek(f,-1,SEEK_END)||fgetc(f)!='f'||fclose(f))return 14;
 int fd=open(argv[1],O_RDWR);if(fd<0)return 15;
 f=port_fdopen(fd,"r+");if(!f||fseek(f,3,SEEK_SET)||fputc('Y',f)==EOF||fclose(f))return 16;
 errno=0;if(close(fd)!=-1||errno!=EBADF)return 17;
 f=fopen(argv[1],"rb");if(!f||fread(b,1,6,f)!=6||strcmp(b,"abXYef")||fclose(f))return 18;
 f=fopen(argv[1],"a+");if(!f||fseek(f,0,SEEK_SET)||fputc('G',f)==EOF||fclose(f))return 19;
 fd=open(argv[1],O_RDWR|O_CREAT|O_APPEND);if(fd<0||lseek(fd,0,SEEK_SET)<0||write(fd,"H",1)!=1||close(fd))return 20;
 char appended[9]={0};f=fopen(argv[1],"r");if(!f||fread(appended,1,8,f)!=8||strcmp(appended,"abXYefGH")||fclose(f))return 21;
 puts("PASS binary update streams, seek, descriptor ownership and reopen");return 0;
}
