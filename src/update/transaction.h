/* Recoverable same-directory HFS replacement, following appinstall's policy.
 * No deletion, overwrite, yield or allocation during commit. Not atomic and not
 * a power-loss guarantee. The adapter must verify identity, both forks and
 * compatibility before calling this, then recheck immediately before commit.
 * Paths are fixed siblings on one resolved volume/directory, not user strings.
 */
#ifndef CLARICE_UPDATE_TRANSACTION_H
#define CLARICE_UPDATE_TRANSACTION_H
/* Adapter: probe(slot) returns 0 if absent, 1 if present, negative on error;
 * check(1) fully validates the candidate; check(0) confirms the same verified
 * object after rename; closed(slot) fails if open;
 * move(from,to) refuses an existing destination; flush() persists the volume.
 * Slots: 0 installed, 1 verified candidate, 2 retained previous version.
 * All adapter errors are negative. Nothing here deletes a recoverable file.
 */
typedef struct {
 void *context;
 int (*probe)(void *,int);
 int (*check)(void *,int);
 int (*closed)(void *,int);
 int (*move)(void *,int,int);
 int (*flush)(void *);
} UpdateOps;
typedef struct {
 int error, rollback_error;
 int installed, previous_retained, candidate_retained;
} UpdateResult;
static UpdateResult update_commit(const UpdateOps *o){
 UpdateResult r={0,0,0,0,0};int present,e;void *c=o->context;
 present=o->probe(c,0);if(present<0){r.error=present;return r;}
 e=o->probe(c,2);if(e){r.error=e<0?e:-48;return r;}
 e=o->probe(c,1);if(e!=1){r.error=e<0?e:-43;return r;}r.candidate_retained=1;
 if((e=o->check(c,1))||(e=o->closed(c,1))||(present&&(e=o->closed(c,0)))||(e=o->flush(c))){r.error=e;return r;}
 if(present){
  e=o->move(c,0,2);if(e){r.error=e;return r;}r.previous_retained=1;
  e=o->flush(c);if(e)goto rollback;
 }
 e=o->move(c,1,0);if(e)goto rollback;
 r.installed=1;r.candidate_retained=0;
 /* A failed final flush/check does not discard either version. Return error
    and exact locations; recovery must be explicit, not a destructive retry. */
 e=o->flush(c);if(!e)e=o->check(c,0);r.error=e;return r;
rollback:
 r.error=e;
 if(r.previous_retained){
  r.rollback_error=o->move(c,2,0);
  if(!r.rollback_error){r.previous_retained=0;r.rollback_error=o->flush(c);}
 }
 return r;
}
#endif
