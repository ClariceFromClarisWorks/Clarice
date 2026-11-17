/* Streaming MacBinary fork extraction. Policy and HFS ownership stay with the
 * caller. Readers/writers return native errors and must reject short I/O. */
#ifndef CLARICE_UPDATE_EXTRACT_H
#define CLARICE_UPDATE_EXTRACT_H
#include "package.h"
typedef struct {void *context;int (*seek)(void *,uint32_t);int (*read)(void *,void *,unsigned);int (*begin_fork)(void *,int);int (*write)(void *,const void *,unsigned);int (*end_fork)(void *);int (*poll)(void *);} UpdateExtractOps;
static int update_extract(const UpdateExtractOps *o,const UpdatePackage *p){
 unsigned char buffer[2048];void *c=o->context;int e=0;
 for(int fork=0;fork<2;fork++){
  e=o->seek(c,fork?p->resource_offset:128);if(e)return e;
  e=o->begin_fork(c,fork);if(e)return e;
  uint32_t left=fork?p->resource_size:p->data_size;
  while(left){unsigned n=left>sizeof buffer?sizeof buffer:(unsigned)left;
   if((e=o->read(c,buffer,n))||(e=o->write(c,buffer,n))||(e=o->poll(c)))break;left-=n;
  }
  int close_error=o->end_fork(c);if(e||close_error)return e?e:close_error;
 }
 return 0;
}
#endif
