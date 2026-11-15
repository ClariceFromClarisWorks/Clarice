/* Automatic UI sizes/rates: base 1024, two decimal places above bytes.
 * Separate from Unix tools' opt-in -h formatting. No floating point. */
#ifndef CLARICE_DISPLAY_SIZE_H
#define CLARICE_DISPLAY_SIZE_H
#include <stdio.h>
static void display_size(char out[32],unsigned long long bytes){
 static const char *units[]={"B","KB","MB","GB","TB","PB","EB"};
 unsigned unit=0;unsigned long long base=1;
 while(unit<6&&(bytes>>(unit*10))>=1024){base<<=10;unit++;}
 if(!unit){snprintf(out,32,"%lu B",(unsigned long)bytes);return;}
 unsigned long whole=(unsigned long)(bytes>>(unit*10));
 /* Split before multiplying: remains exact without overflowing at EiB sizes. */
 unsigned long long remainder=bytes&(base-1),quarter=base>>2;
 unsigned fraction=(unsigned)((remainder/quarter)*25+((remainder%quarter)*25+quarter/2)/quarter);
 if(fraction==100){whole++;fraction=0;}
 if(whole==1024&&unit<6){whole=1;fraction=0;unit++;}
 snprintf(out,32,"%lu.%02u %s",whole,fraction,units[unit]);
}
#endif
