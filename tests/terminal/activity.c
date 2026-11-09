#include <assert.h>
#include <stdio.h>
#include "../../src/activity.h"
int main(void){ActivityClock c={0};
 assert(activity_due(&c,299,0,1,0)==-1);
 assert(activity_due(&c,300,0,1,0)==0);activity_presented(&c);
 assert(activity_due(&c,329,0,1,0)==-2);
 /* A two-second stall must repay all four frames, in order. */
 for(int i=1;i<=4;i++){assert(activity_due(&c,420,0,1,0)==(i&3));activity_presented(&c);}
 assert(activity_due(&c,420,0,1,0)==-2);
 assert(activity_due(&c,900,0,0,0)==-1);
 assert(activity_due(&c,900,0,1,1)==-1);
 assert(activity_due(&c,900,899,1,0)==-1);
 assert(activity_due(&c,200,UINT32_MAX-99,1,0)==0);activity_presented(&c);
 assert(activity_due(&c,229,UINT32_MAX-99,1,0)==-2);
 assert(activity_due(&c,230,UINT32_MAX-99,1,0)==1);
 puts("PASS: activity delay, sequential catch-up, suppression and tick wraparound");
}
