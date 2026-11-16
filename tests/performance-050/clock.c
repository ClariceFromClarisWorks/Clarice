/* Guest clock calibration: no I/O in the measured CPU loop. */
#include <OSUtils.h>
#include <stdio.h>
extern void say(const char *);
int clarice_main(int argc,char **argv){(void)argc;(void)argv;char b[100];volatile unsigned long sum=1;for(unsigned k=1;k<=4;k*=2){unsigned long t=TickCount();for(unsigned long i=0;i<100000UL*k;i++)sum=(sum<<1)^i;unsigned long elapsed=TickCount()-t;snprintf(b,sizeof b,"clock iterations=%lu ticks=%lu sum=%lu\n",100000UL*k,elapsed,sum);say(b);}return 0;}
