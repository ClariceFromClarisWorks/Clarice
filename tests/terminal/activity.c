#include <assert.h>
#include "../../src/activity.h"
int main(void){
 assert(activity_frame(299,0,1,0)==-1);
 assert(activity_frame(300,0,1,0)==0);
 assert(activity_frame(329,0,1,0)==0);
 assert(activity_frame(330,0,1,0)==1);
 assert(activity_frame(360,0,1,0)==2);
 assert(activity_frame(390,0,1,0)==3);
 assert(activity_frame(420,0,1,0)==0);
 assert(activity_frame(900,0,0,0)==-1);
 assert(activity_frame(900,0,1,1)==-1);
 assert(activity_frame(900,899,1,0)==-1);
 assert(activity_frame(200,UINT32_MAX-99,1,0)==0);
 return 0;
}
