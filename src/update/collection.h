/* Coordinated release activation using the existing native file transaction
 * primitives. No allocation, unlink, overwrite or OS patch. The adapter owns
 * durable journal encoding, identity checks, same-volume renames and flushing.
 * An unfinished journal blocks normal launch. Recovery rolls back all members;
 * do not delete the journal unless the complete rollback has been flushed.
 */
#ifndef CLARICE_UPDATE_COLLECTION_H
#define CLARICE_UPDATE_COLLECTION_H
/* Slots: CURRENT=0, STAGED=1, PREVIOUS=2. Identity classifications are
 * ABSENT=0, OLD=1, NEW=2; negative means I/O, corruption or unknown ownership.
 * `old` is fixed in the prepared journal before the first rename. */
typedef struct {
 void *ctx;
 unsigned count;
 int (*old)(void *,unsigned);
 int (*identify)(void *,unsigned,int);
 int (*move)(void *,unsigned,int,int);
 int (*flush)(void *);
 int (*journal)(void *,int); /* 1 prepared, 2 committed; 0 clear after rollback */
 int (*cancel)(void *);
} CollectionOps;
typedef struct {int error,recovery_error,committed;} CollectionResult;
static int collection_recover(const CollectionOps *o){
 unsigned i=o->count;
 while(i--){
  int a=o->identify(o->ctx,i,0),s=o->identify(o->ctx,i,1),b=o->identify(o->ctx,i,2),e;
  if(a<0||s<0||b<0)return a<0?a:s<0?s:b;
  if(a==2){
   if(s)return -50;
   if((e=o->move(o->ctx,i,0,1))||(e=o->flush(o->ctx)))return e;
   a=0;s=2;
  }
  if(o->old(o->ctx,i)){
   if(a==0&&b==1){
    if((e=o->move(o->ctx,i,2,0))||(e=o->flush(o->ctx)))return e;
    a=1;b=0;
   }
   if(a!=1||b||s!=2)return -50;
  }else if(a||b||s!=2)return -50;
 }
 {int e=o->flush(o->ctx);return e?e:o->journal(o->ctx,0);}
}
static CollectionResult collection_commit(const CollectionOps *o){
 CollectionResult r={0,0,0};unsigned i;int e;
 if(!o->count){r.error=-50;return r;}
 /* Full validation/ownership decisions precede any installed-file mutation. */
 for(i=0;i<o->count;i++){
  if((e=o->cancel(o->ctx))){r.error=e;return r;}
  int a=o->identify(o->ctx,i,0),s=o->identify(o->ctx,i,1),b=o->identify(o->ctx,i,2);
  if(a!=(o->old(o->ctx,i)?1:0)||s!=2||b){r.error=a<0?a:s<0?s:b<0?b:-50;return r;}
 }
 if((e=o->journal(o->ctx,1))||(e=o->flush(o->ctx))){r.error=e;return r;}
 for(i=0;i<o->count;i++){
  if((e=o->cancel(o->ctx)))goto failed;
  if(o->old(o->ctx,i)){
   if((e=o->move(o->ctx,i,0,2))||(e=o->flush(o->ctx)))goto failed;
  }
  if((e=o->move(o->ctx,i,1,0))||(e=o->flush(o->ctx)))goto failed;
 }
 for(i=0;i<o->count;i++)if(o->identify(o->ctx,i,0)!=2){e=-50;goto failed;}
 if((e=o->journal(o->ctx,2))||(e=o->flush(o->ctx))){r.error=e;return r;}
 r.committed=1;return r;
failed:
 r.error=e;r.recovery_error=collection_recover(o);return r;
}
#endif
