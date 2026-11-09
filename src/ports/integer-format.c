/* Single integer conversion used by awk's printf/sprintf. Pinned 68K newlib
 * prints the high word of intmax_t instead of supporting the j/ll conversion.
 * Keep all 64 bits and format without variadic ABI or libc integer formatting. */
#include <stdint.h>
#include <stddef.h>
#include <string.h>
static void append(char *dst,size_t cap,size_t *used,char c){if(*used+1<cap)dst[*used]=c;(*used)++;}
int port_awk_integer(char *dst,size_t cap,const char *fmt,uintmax_t value){
 const char *p=fmt;int left=0,plus=0,space=0,alt=0,zero=0,width=0,precision=-1;
 if(*p++!='%')return -1;
 for(;;p++){if(*p=='-')left=1;else if(*p=='+')plus=1;else if(*p==' ')space=1;else if(*p=='#')alt=1;else if(*p=='0')zero=1;else break;}
 while(*p>='0'&&*p<='9'){if(width>16384)return -1;width=width*10+*p++-'0';}if(width>16384)return -1;
 if(*p=='.'){p++;precision=0;while(*p>='0'&&*p<='9'){if(precision>16384)return -1;precision=precision*10+*p++-'0';}if(precision>16384)return -1;}
 while(*p=='j'||*p=='l'||*p=='h')p++;char mode=*p++;if(*p||!strchr("diouxX",mode))return -1;
 int negative=0,signed_value=mode=='d'||mode=='i';if(signed_value&&(intmax_t)value<0){negative=1;value=(uintmax_t)0-value;}
 unsigned base=mode=='o'?8:mode=='x'||mode=='X'?16:10;char digits[65],prefix[3];int count=0,prefix_count=0;uintmax_t original=value;
 if(value||precision!=0)do{unsigned d=value%base;digits[count++]=(d<10?'0':'a'-10)+d;if(mode=='X'&&digits[count-1]>='a')digits[count-1]-='a'-'A';value/=base;}while(value);
 if(signed_value){if(negative)prefix[prefix_count++]='-';else if(plus)prefix[prefix_count++]='+';else if(space)prefix[prefix_count++]=' ';}
 if(alt&&base==16&&original){prefix[prefix_count++]='0';prefix[prefix_count++]=mode;}
 if(alt&&base==8&&(!count||digits[count-1]!='0')&&precision<=count)precision=count+1;
 int zeros=precision>count?precision-count:0,pad=width-prefix_count-count-zeros;if(pad<0)pad=0;
 if(zero&&!left&&precision<0){zeros+=pad;pad=0;}
 size_t used=0;if(!left)for(int i=0;i<pad;i++)append(dst,cap,&used,' ');
 for(int i=0;i<prefix_count;i++)append(dst,cap,&used,prefix[i]);for(int i=0;i<zeros;i++)append(dst,cap,&used,'0');while(count)append(dst,cap,&used,digits[--count]);if(left)for(int i=0;i<pad;i++)append(dst,cap,&used,' ');
 if(cap)dst[used<cap?used:cap-1]=0;return used;
}
