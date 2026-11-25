/* Disposable 24-bit System 6 fixture: SysZone low-memory global at 0x2A6.
 * Read native heap sizes, restore current zone before any output/allocation. */
#include <Memory.h>
#include <stdio.h>
extern void say(const char *);
int clarice_main(int argc,char **argv){(void)argc;(void)argv;THz old=GetZone();long a=FreeMem(),am=MaxBlock();SetZone(LMGetSysZone());long s=FreeMem(),sm=MaxBlock();SetZone(old);char b[160];snprintf(b,sizeof b,"app=%ld largest=%ld system=%ld largest=%ld\n",a,am,s,sm);say(b);return 0;}
