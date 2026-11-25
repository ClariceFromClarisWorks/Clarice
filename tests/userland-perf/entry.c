#include <Memory.h>
#include <stdio.h>
#include "record.h"
extern void say(const char*);
static volatile unsigned char working[65536];
int clarice_main(int argc,char**argv){(void)argc;(void)argv;long f=FreeMem(),m=MaxBlock();char s[120];working[65535]=1;snprintf(s,sizeof s,"ENTRY free=%ld largest=%ld marker=%d\n",f,m,working[65535]);record(f<32768?"/mnt/Sys6Unix/Perf/entry-low.txt":"/mnt/Sys6Unix/Perf/entry-room.txt",s);say(s);return 0;}
