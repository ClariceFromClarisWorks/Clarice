/* Bounded newline reader shared by online and offline catalog consumers. */
#ifndef CLARICE_CATALOG_READER_H
#define CLARICE_CATALOG_READER_H
typedef struct {short ref;char bytes[1024];unsigned pos,end;long complete;int eof;} RepoReader;
static int repo_next(RepoReader *r,char *line,unsigned cap){
 unsigned used=0;for(;;){
  if(r->pos==r->end){long n=sizeof r->bytes;int e=FSRead(r->ref,&n,r->bytes);if(e&&e!=eofErr)return e;r->pos=0;r->end=n;if(!n){r->eof=1;line[used]=0;return used?2:0;}}
  char c=r->bytes[r->pos++];if(!c||c=='\r')return paramErr;
  if(c=='\n'){line[used]=0;r->complete+=used+1;return 1;}
  if(used+1>=cap)return paramErr;line[used++]=c;
 }
}
#endif
