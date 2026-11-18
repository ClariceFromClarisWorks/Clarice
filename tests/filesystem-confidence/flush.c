#include <Files.h>
int clarice_main(int argc,char **argv){unsigned char a[]={10,70,83,32,65,117,100,105,116,32,65},b[]={10,70,83,32,65,117,100,105,116,32,66};int x=FlushVol(a,0),y=FlushVol(b,0),z=FlushVol(0,0);return x||y||z?1:0;}
