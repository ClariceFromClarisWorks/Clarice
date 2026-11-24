/* Disposable Snow-only launch timer; same System 6 native sublaunch convention
   as tests/native-bench. Timestamp write/close is included in the measured span. */
#include <Multiverse.h>
int main(void){InitGraf(&qd.thePort);InitFonts();InitWindows();InitMenus();
 unsigned char stamp[]="\pSys6Unix:launch-q1",name[]="\pClarice";short ref;long n=4;
 HCreate(0,0,stamp,'CLRC','DATA');if(HOpen(0,0,stamp,fsWrPerm,&ref))return 1;
 SetVol((unsigned char*)"\pSys6Unix",0);unsigned long t=TickCount();
 FSWrite(ref,&n,&t);FSClose(ref);
 struct __attribute__((packed,aligned(2))) {unsigned char *name;short param;char lc[2];long len;short flags;unsigned long options;} block={name,0,{'L','C'},6,0,0xc0000000UL};
 register void *a0 __asm__("a0")=&block;register long d0 __asm__("d0");
 __asm__ volatile(".word 0xa9f2":"=d"(d0),"+a"(a0)::"d1","a1","cc","memory");return 0;
}
