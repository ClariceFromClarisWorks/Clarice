/* GPL-3.0-or-later; MiniVNC technique, see mouse.h and vendor notes.
 * Interrupt code: no heap, stdio, networking, QuickDraw, app globals or A5 use.
 * A fixed, completely prepared sequence must end in release; max duration 3 sec.
 */
#include "mouse.h"
#define TICKS (*(volatile unsigned long*)0x016a)
#define MBTICKS (*(volatile unsigned long*)0x016e)
#define BUTTON (*(volatile unsigned char*)0x0172)

OSErr hil_post(short what,long msg,short mods,short x,short y) {
 register long result __asm__("d0")=msg;
 register long event __asm__("a0")=what;
 __asm__ volatile(".word 0xa12f":"+d"(result),"+a"(event)::"d1","a1","cc","memory");
 EvQElPtr q=(EvQElPtr)event;
 if(!(short)result&&q){q->evtQModifiers=mods;q->evtQWhere.h=x;q->evtQWhere.v=y;}
 return (short)result;
}
static void position(short x,short y) {
 /* MiniVNC's MouseTemp/RawMouseLocation and cursor-dirty sequence. */
 volatile short *temp=(volatile short*)0x0828,*raw=(volatile short*)0x082c;
 temp[0]=raw[0]=y;temp[1]=raw[1]=x;
 *(volatile unsigned char*)0x08ce=*(volatile unsigned char*)0x08cf;
}
void hil_release(MousePlan *s) {
 if(s->down){
  MBTICKS=TICKS;BUTTON=0x80;s->down=0;
  volatile short *p=(volatile short*)0x082c;
  if(hil_post(mouseUp,0,btnState,p[1],p[0]))s->error=1;
 }
 s->active=0;s->completed=TICKS;
}
void hil_tick(MousePlan *s) {
 if(!s->active)return;
 unsigned long elapsed=TICKS-s->started;
 if(s->cancel||elapsed>240){hil_release(s);return;}
 /* Refresh only while our bounded plan owns the button. Never latch across
    commands: a lost host can delay release by at most this finite sequence. */
 if(s->down){MBTICKS=TICKS+3;BUTTON=0;}
 if(s->index<s->count&&elapsed>=s->steps[s->index].tick){
  MouseStep *p=&s->steps[s->index++];position(p->x,p->y);
  if(p->down!=s->down){
   s->down=p->down;MBTICKS=TICKS+(s->down?3:0);BUTTON=s->down?0:0x80;
   if(hil_post(s->down?mouseDown:mouseUp,0,s->down?0:btnState,p->x,p->y)){
    s->error=1;hil_release(s);return;
   }
  }
 }
 if(s->index>=s->count){hil_release(s);return;}
 PrimeTime((QElemPtr)&s->task,16);
}
