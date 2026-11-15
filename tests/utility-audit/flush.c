/* Test fixture: persist guest HFS results before offline image inspection. */
#include <Files.h>
int clarice_main(int argc,char **argv){unsigned char a[]={5,'A','u','d','i','t'},b[]={8,'S','y','s','6','U','n','i','x'};int x=FlushVol(a,0),y=FlushVol(b,0);return x||y?1:0;}
