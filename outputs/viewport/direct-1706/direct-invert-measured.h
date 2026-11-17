#ifndef CLARICE_DIRECT_INVERT_H
#define CLARICE_DIRECT_INVERT_H
#include <stdint.h>
/* All bytes touched belong to fully composed text scanlines. The boundary
   bytes are initialized by the glyph compositor; final CopyBits clips their
   unused outside bits. Word accesses are even on the baseline 68000. */
static int direct_stage_invert(unsigned char *base,int px,int py,int width,int height){
 if(!base||px<0||py<0||width<1||height<1||px>512-width||py>342-height)return 0;
 int bytes=((px&7)+width+7)>>3;
 for(int y=0;y<height;y++){
  unsigned char *p=base+(py+y)*64+(px>>3);int left=bytes;
  if((uintptr_t)p&1){*p=(unsigned char)~*p;p++;left--;}
  while(left>=2){uint16_t *word=(uint16_t*)p;*word=(uint16_t)~*word;p+=2;left-=2;}
  if(left)*p=(unsigned char)~*p;
 }
 return 1;
}
#endif
