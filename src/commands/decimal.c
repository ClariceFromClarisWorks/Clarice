/* Bounded decimal conversion for HFS's 32-bit logical fork lengths.
   Two DIVU steps perform long division without overflowing the 16-bit
   quotient. Values <=65535 need only one step. No heap or static cache. */
void format_decimal32(char *out,unsigned long value){
 char digits[10];int n=0;
 do{
#ifdef __m68k__
  unsigned long high=value>>16,low;
  unsigned short ten=10;
  if(high){__asm__("divu.w %1,%0":"+d"(high):"d"(ten):"cc");
   low=(high&0xffff0000UL)|(value&65535);
  }else low=value;
  __asm__("divu.w %1,%0":"+d"(low):"d"(ten):"cc");
  digits[n++]='0'+(low>>16);value=((high&65535)<<16)|(low&65535);
#else
  digits[n++]='0'+value%10;value/=10;
#endif
 }while(value);
 while(n)*out++=digits[--n];
 *out=0;
}
