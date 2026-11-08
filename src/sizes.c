#include "sys6.h"
#include <stdio.h>
#include <string.h>
/* HFS allocation-block products can exceed 32 bits. No floating point.
   Keep ordinary file-size decimal conversion on the 32-bit fast path. */
void format_size(char *out,unsigned long long bytes,int human){
 static const char *units[]={"B","KiB","MiB","GiB","TiB","PiB","EiB"};
 if(!human){if(bytes<=0xffffffffULL){unsigned long value=(unsigned long)bytes;char digits[10];int n=0,i;do{digits[n++]='0'+value%10;value/=10;}while(value);for(i=0;i<n;i++)out[i]=digits[n-1-i];out[n]=0;return;}
  char digits[24];int n=0;do{digits[n++]='0'+bytes%10;bytes/=10;}while(bytes);int i;for(i=0;i<n;i++)out[i]=digits[n-1-i];out[n]=0;return;}
 int unit=0;unsigned long long base=1;
 while(unit<6&&(bytes>>(unit*10))>=1024){base<<=10;unit++;}
 if(!unit){snprintf(out,32,"%luB",(unsigned long)bytes);return;}
 unsigned long whole=(unsigned long)(bytes>>(unit*10)),tenth=(unsigned long)((((bytes&(base-1))*10+base/2)>>(unit*10)));
 if(tenth==10){whole++;tenth=0;}
 if(whole>=1024&&unit<6){whole=1;tenth=0;unit++;}
 if(whole<10)snprintf(out,32,"%lu.%lu%s",whole,tenth,units[unit]);else snprintf(out,32,"%lu%s",whole,units[unit]);
}
