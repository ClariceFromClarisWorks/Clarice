/* Two stateless streams, not a device filesystem. Native FCB references are
   signed shorts; portable file tokens use a disjoint positive range. */
#ifndef CLARICE_VIRTUAL_STREAM_H
#define CLARICE_VIRTUAL_STREAM_H
#include <errno.h>
#include <string.h>
#define VSTREAM_TAG 65536
static int vstream_code(int ref){
 int code=ref>=-11&&ref<=-5?-ref:ref>=VSTREAM_TAG+5&&ref<=VSTREAM_TAG+11?ref-VSTREAM_TAG:0;
 return code==5||code==6||code==7||code==9||code==10||code==11?code:0;
}
static int vstream_kind(const char *path){return !strcmp(path,"/dev/null")?1:!strcmp(path,"/dev/zero")?2:0;}
static long vstream_read(int ref,void *buf,unsigned long count){
 int code=vstream_code(ref);if(!(code&1)){errno=EBADF;return -1;}
 if(count>0x7fffffffUL){errno=EINVAL;return -1;}
 if(!count||(code>>2)==1)return 0;memset(buf,0,count);return (long)count;
}
static long vstream_write(int ref,unsigned long count){
 if(!(vstream_code(ref)&2)){errno=EBADF;return -1;}
 if(count>0x7fffffffUL){errno=EINVAL;return -1;}return (long)count;
}
#endif
