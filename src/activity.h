#ifndef CLARICE_ACTIVITY_H
#define CLARICE_ACTIVITY_H
#include <stdint.h>
typedef struct {uint32_t origin,next;unsigned step;int active;} ActivityClock;
/* -1 hides, -2 retains the current frame; nonnegative values are due frames.
   Commit only after presentation. Missed deadlines remain owed, not skipped. */
static int activity_due(ActivityClock *c,uint32_t now,uint32_t output,int running,int owned){
 if(!running||owned||now-output<300){c->active=0;return -1;}
 if(!c->active||c->origin!=output){c->active=1;c->origin=output;c->next=output+300;c->step=0;}
 return (int32_t)(now-c->next)<0?-2:(int)(c->step&3);
}
static void activity_presented(ActivityClock *c){c->step++;c->next+=30;}
#endif
