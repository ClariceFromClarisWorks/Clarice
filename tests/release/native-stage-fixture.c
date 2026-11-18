/* One-off deployment fixture: extract only into disposable staging directories.
 * Uses production
 * release reader + native HFS primitives. Not a shipped installer. */
#include "../../src/commands/mac-files.h"
#include "../../src/update/release-tar.h"
extern int app_yield(void);
static short input=-1;
static char directory[PATH_CAP];
static unsigned char buffer[4096];
static int verify_only;
static int read_at(void *unused,unsigned long pos,void *p,unsigned n){
 (void)unused;int e=SetFPos(input,fsFromStart,pos);long count=n;
 if(!e)e=cli_native_read(input,p,&count);return e?e:count==n?0:ioErr;
}
static int member(void *unused,unsigned i,unsigned long pos,unsigned long size,uint32_t expected){
 (void)unused;char path[PATH_CAP];short out=-1;int e=0,owned=0;
 const char *name=i?release_names[i-1]:"Clarice.bin";
 if(verify_only&&!i)return 0;
 if(strlen(directory)+strlen(name)+2>=sizeof path)return bdNamErr;
 strcpy(path,directory);strcat(path,":");strcat(path,name);
 if(verify_only){e=cli_native_open(path,0,0,&out);long length=0;if(!e)e=GetEOF(out,&length);if(!e&&(unsigned long)length!=size)e=paramErr;}
 else {e=cli_native_create(path,'CLRC',i?'CLX1':'BINA');if(!e){owned=1;e=cli_native_open(path,0,1,&out);}}
 UpdateCRC32 crc;update_crc32_init(&crc);
 while(!e&&size){unsigned n=size>sizeof buffer?sizeof buffer:size;long count=n;
  if(app_yield()){e=userCanceledErr;break;}
  e=verify_only?cli_native_read(out,buffer,&count):read_at(0,pos,buffer,n);
  if(!e&&count!=n)e=ioErr;
  if(!e){update_crc32_bytes(&crc,buffer,n);if(!verify_only){e=cli_native_write(out,buffer,&count);if(!e&&count!=n)e=ioErr;}}
  pos+=n;size-=n;
 }
 if(out>=0){int x=cli_native_close(out);if(!e)e=x;}
 if(!e&&update_crc32_sum(&crc)!=expected)e=paramErr;
 if(e&&owned){int x=cli_native_delete(path);if(x)error("retained partial staging file",x);}
 say(e?"FAIL ":"OK ");say(name);say("\n");return e;
}
int clarice_main(int argc,char **argv){
 if(argc!=4|| (strcmp(argv[1],"extract")&&strcmp(argv[1],"verify"))){errsay("usage: release-stage extract|verify BUNDLE DIRECTORY\n");return 2;}
 verify_only=!strcmp(argv[1],"verify");char native[PATH_CAP];CInfoPBRec p;
 int e=resolve(argv[3],directory);if(!e)e=info(directory,&p);if(!e&&!(p.hFileInfo.ioFlAttrib&16))e=paramErr;
 if(!e)e=resolve(argv[2],native);if(!e)e=cli_native_open(native,0,0,&input);
 long length=0;if(!e)e=GetEOF(input,&length);
 if(!e){ReleaseArchive a;memset(&a,0,sizeof a);a.length=length;a.read=read_at;a.member=member;e=release_archive(&a);}
 if(input>=0){int x=cli_native_close(input);if(!e)e=x;input=-1;}
 if(e){error("release-stage",e);return e==userCanceledErr?130:1;}
 say(verify_only?"PASS: all 47 installed utility lengths/CRCs match release\n":"PASS: complete bundle extracted and CRC verified\n");return 0;
}
