/* Shift only the staged row span, backwards for overlap. Whole boundary words
   may change outside the source rectangle; CopyBits never presents those bits.
   The 64-byte stride and even base keep every access inside this existing port. */
extern void asm_align_row_bits(unsigned char *,long,long,long);
static int align_native_row(Rect *source,int phase){
 int shift=(phase-source->left)&15;
 if(!shift||source->left<0||source->right<=source->left||source->right+shift>512||source->top<0||source->bottom>64||source->bottom<=source->top)return 0;
 int first=source->left>>4,last=(source->right+shift-1)>>4;
 asm_align_row_bits(prompt_pixels+source->top*STRIDE+last*2,last-first+1,source->bottom-source->top,shift);
 source->left+=shift;source->right+=shift;return 1;
}
