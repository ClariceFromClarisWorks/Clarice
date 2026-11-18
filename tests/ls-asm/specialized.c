/* Investigation only, derived from src/commands/decimal.c.
 * Preserve the existing long division until the remaining value fits a word;
 * then stop repeating the high-word work. Same inline DIVU, no lookup table.
 */
void specialized_decimal_field(char *out, unsigned long value) {
 char *p = out + 10;
 unsigned short ten = 10;
 while (value > 65535UL) {
  unsigned long high = value >> 16, low;
  __asm__("divu.w %1,%0" : "+d"(high) : "d"(ten) : "cc");
  low = (high & 0xffff0000UL) | (value & 65535UL);
  __asm__("divu.w %1,%0" : "+d"(low) : "d"(ten) : "cc");
  *--p = '0' + (low >> 16);
  value = ((high & 65535UL) << 16) | (low & 65535UL);
 }
 do {
  __asm__("divu.w %1,%0" : "+d"(value) : "d"(ten) : "cc");
  *--p = '0' + (value >> 16);
  value &= 65535UL;
 } while (value);
 while (p > out) *--p = ' ';
}
