#ifndef CLARICE_ACTIVITY_H
#define CLARICE_ACTIVITY_H
#include <stdint.h>
/* TickCount is 60 Hz. Unsigned subtraction also handles its wraparound. */
static int activity_frame(uint32_t now,uint32_t last_output,int running,int screen_owned){
 uint32_t quiet=now-last_output;
 return !running||screen_owned||quiet<300?-1:(int)(((quiet-300)/30)%4);
}
#endif
