#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../../src/update/transaction.h"
typedef struct {int slots[3], fail, second_fail, calls, busy, invalid;} State;
static int fault(State *s){++s->calls;return s->calls==s->fail||s->calls==s->second_fail?-36:0;}
static int probe(void *v,int i){State *s=v;int e=fault(s);return e?e:!!s->slots[i];}
static int check(void *v,int i){State *s=v;int e=fault(s);return e?e:s->invalid||s->slots[i]!=2?-50:0;}
static int closed(void *v,int i){State *s=v;int e=fault(s);return e?e:s->busy==i+1?-47:0;}
static int move(void *v,int a,int b){State *s=v;int e=fault(s);if(e)return e;assert(s->slots[a]);assert(!s->slots[b]);s->slots[b]=s->slots[a];s->slots[a]=0;return 0;}
static int flush(void *v){return fault(v);}
static UpdateResult run(State *s){UpdateOps o={s,probe,check,closed,move,flush};return update_commit(&o);}
static void preserved(State *s,int old){int count=0;for(int i=0;i<3;i++)count+=s->slots[i]==1;assert(count==old);count=0;for(int i=0;i<3;i++)count+=s->slots[i]==2;assert(count==1);}
int main(void){int cases=0;
 for(int old=0;old<=1;old++){
  State initial={{old,2,0},0};UpdateResult r=run(&initial);assert(!r.error&&r.installed);assert(initial.slots[0]==2&&initial.slots[2]==old);preserved(&initial,old);cases++;
  /* Every operation can fail, including the rollback attempt itself. */
  for(int a=1;a<=14;a++)for(int b=0;b<=16;b++){
   State s={{old,2,0},a,b};r=run(&s);preserved(&s,old);
   if(r.installed)assert(s.slots[0]==2);else assert(s.slots[1]==2);
   if(r.previous_retained)assert(s.slots[2]==1);
   cases++;
  }
 }
 for(int busy=1;busy<=2;busy++){State s={{1,2,0},0,0,0,busy};UpdateResult r=run(&s);assert(r.error==-47&&!r.installed&&s.slots[0]==1&&s.slots[1]==2);cases++;}
 State bad={{1,2,0},0,0,0,0,1};assert(run(&bad).error==-50&&bad.slots[0]==1);cases++;
 State backup={{1,2,3},0};assert(run(&backup).error==-48&&backup.slots[0]==1&&backup.slots[2]==3);cases++;
 printf("PASS %d transaction scenarios: creation/replacement, every operation failure, rollback failure, busy candidate/target, invalid candidate, retained backup refusal; neither version deleted\n",cases);
}
