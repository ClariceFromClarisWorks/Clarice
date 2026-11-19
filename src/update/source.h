/* Repository transport selection. Mounted AFP/HFS volumes use the ordinary
 * Macintosh file services; mounting/authentication remain OS responsibilities. */
#ifndef CLARICE_UPDATE_SOURCE_H
#define CLARICE_UPDATE_SOURCE_H
static int update_source_valid(const char *s){
 if(!s||!*s||strchr(s,'\n')||strchr(s,'\r'))return 0;
 const char *last=strrchr(s,'/');if(!last||!last[1])return 0;
 if(s[0]=='/')return s[1]&&s[1]!='/';
 if(strncmp(s,"http://",7))return 0;
 const char *slash=strchr(s+7,'/');char host[32];size_t n=slash?(size_t)(slash-s-7):0;
 if(!n||n>=sizeof host)return 0;memcpy(host,s+7,n);host[n]=0;
 char *port=strchr(host,':');if(port)*port=0;
 unsigned long ip=NetParseIP(host);
 return (ip>>24)==10||(ip>>16)==0xc0a8||(ip>>20)==0xac1||(ip>>24)==127;
}
/* Snapshot data-fork packages into owned /tmp storage. Never install directly
 * from a share that can disappear or change. Existing CRC/length checks follow.
 * No allocations or networking startup; one bounded transfer buffer. */
static int update_copy_source(const char *source,const char *dest,unsigned long maximum){
 short in=-1,out=-1;long size=0;int e=open_path(source,fsRdPerm,&in);
 if(!e)e=GetEOF(in,&size);
 if(!e&&(size<0||(maximum&&(unsigned long)size>maximum)))e=paramErr;
 if(!e)e=open_path(dest,fsWrPerm,&out);
 char bytes[2048];long left=size;
 while(!e&&left){
  if(app_yield()){e=userCanceledErr;break;}
  long n=left>(long)sizeof bytes?(long)sizeof bytes:left;
  e=io_read(in,bytes,n);if(!e)e=io_write(out,bytes,n);if(!e)left-=n;
 }
 if(!e){long now;e=GetEOF(in,&now);if(!e&&now!=size)e=ioErr;}
 if(out>=0){int x=FSClose(out);if(!e)e=x;}
 if(in>=0){int x=FSClose(in);if(!e)e=x;}
 return e;
}
#endif
