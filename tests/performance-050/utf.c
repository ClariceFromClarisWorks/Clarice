#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "utf.h"
int fast_fgetrune(Rune *,FILE *);int fast_fputrune(const Rune *,FILE *);int fast_isspacerune(Rune);
static void input(const unsigned char *p,size_t n){FILE *a=tmpfile(),*b=tmpfile();assert(a&&b);fwrite(p,1,n,a);fwrite(p,1,n,b);rewind(a);rewind(b);for(;;){Rune x=123,y=123;int i=fgetrune(&x,a),j=fast_fgetrune(&y,b);assert(i==j);assert(x==y);assert(feof(a)==feof(b));assert(ferror(a)==ferror(b));if(!i)break;}fclose(a);fclose(b);}
int main(void){unsigned char data[4096];for(int i=0;i<4096;i++)data[i]=(i*73+i/256)&255;for(int n=0;n<10;n++)input(data,n);input(data,sizeof data);for(int i=0;i<128;i++)data[i]=i;input(data,128);const unsigned char utf[]={0xc3,0xa9,0xe4,0xb8,0x96,0xf0,0x9f,0x98,0x80};for(int n=0;n<=9;n++)input(utf,n);
for(Rune r=-1;r<=0x110000;r++)assert(isspacerune(r)==fast_isspacerune(r));
FILE *a=tmpfile(),*b=tmpfile();assert(a&&b);for(Rune r=-1;r<2048;r++)assert(fputrune(&r,a)==fast_fputrune(&r,b));rewind(a);rewind(b);int x,y;do{x=fgetc(a);y=fgetc(b);assert(x==y);}while(x!=EOF);fclose(a);fclose(b);puts("PASS: exhaustive Unicode whitespace classification; ASCII, binary, malformed/truncated UTF input; identical rune outputs/statuses");}
