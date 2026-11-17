#include <assert.h>
#include <stdio.h>
#include "../../src/system-version.h"
int main(void){
 unsigned char v[]={6,8,128,0,0,0};
 assert(clarice_system_version(v,6)==0x608);
 v[0]=7;v[1]=0x10;assert(clarice_system_version(v,6)==0x710);
 assert(!clarice_system_version(v,5)&&!clarice_system_version(0,6));
 v[1]=0xfa;assert(!clarice_system_version(v,6));
 puts("PASS System vers: 6.0.8, 7.1, truncated/missing/invalid BCD fallback");
}
