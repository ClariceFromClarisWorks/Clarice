/* Classic HFS unsigned seconds since 1904; full range ends in 2040.
   Within this range every fourth year is leap, including 2000. */
/* All callers guarantee a 16-bit quotient. DIVU returns remainder:quotient.
   The portable branch is the executable reference for the same operation. */
static unsigned long divmod16(unsigned long value,unsigned short divisor){
#ifdef __m68k__
 __asm__("divu.w %1,%0" : "+d"(value) : "d"(divisor) : "cc");
 return value;
#else
 return ((value%divisor)<<16)|(value/divisor);
#endif
}
static void pair(char *p,unsigned int v){unsigned int tens=0;while(v>=10){v-=10;tens++;}p[0]='0'+tens;p[1]='0'+v;}
void format_created(char out[17],unsigned long seconds){
 static const unsigned char month_days[]={31,28,31,30,31,30,31,31,30,31,30,31};
 /* Halving makes the day divisor fit DIVU without losing the low second. */
 unsigned long split=divmod16(seconds>>1,43200);
 unsigned int day=split&65535,rem=(split>>16)*2+(seconds&1);
 split=divmod16(day,1461);
 unsigned int year=1904+(split&65535)*4,within=split>>16,month=0;
 if(within>=366){within-=366;year++;while(within>=365){within-=365;year++;}}
 while(month<11){unsigned int n=month_days[month]+(month==1&&!(year&3));if(within<n)break;within-=n;month++;}
 pair(out,year>=2000?20:19);pair(out+2,year-(year>=2000?2000:1900));out[4]='-';pair(out+5,month+1);out[7]='-';pair(out+8,within+1);
 split=divmod16(rem,60);split=divmod16(split&65535,60);
 out[10]=' ';pair(out+11,split&65535);out[13]=':';pair(out+14,split>>16);out[16]=0;
}
