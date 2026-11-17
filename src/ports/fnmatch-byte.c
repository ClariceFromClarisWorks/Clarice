/* Genuine musl matching, adapted only at the filename character boundary.
   Native HFS paths are MacRoman bytes, not UTF-8; named classes are ASCII/C. */
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>
#include <limits.h>
static int byte_mbtowc(wchar_t *out,const char *s,size_t n) {
 if (!s) return 0;
 if (!n) return -1;
 *out=(unsigned char)*s; return *s ? 1 : 0;
}
static int byte_upper(int c){return c>='a'&&c<='z'?c-'a'+'A':c;}
static int byte_lower(int c){return c>='A'&&c<='Z'?c-'A'+'a':c;}
static int byte_class(const char *s) {
 static const char *const names[]={"alnum","alpha","blank","cntrl","digit","graph","lower","print","punct","space","upper","xdigit"};
 for(int i=0;i<12;i++)if(!strcmp(s,names[i]))return i+1;
 return 0;
}
static int byte_isclass(int c,int type) {
 int upper=c>='A'&&c<='Z',lower=c>='a'&&c<='z',digit=c>='0'&&c<='9',alpha=upper||lower;
 if(c<0||c>127)return 0;
 switch(type){
 case 1:return alpha||digit;case 2:return alpha;case 3:return c==' '||c=='\t';
 case 4:return c<32||c==127;case 5:return digit;case 6:return c>32&&c<127;
 case 7:return lower;case 8:return c>=32&&c<127;case 9:return c>32&&c<127&&!alpha&&!digit;
 case 10:return c==' '||(c>=9&&c<=13);case 11:return upper;
 case 12:return digit||(c>='a'&&c<='f')||(c>='A'&&c<='F');default:return 0;
 }
}
#undef MB_CUR_MAX
#define MB_CUR_MAX 1
#define mbtowc byte_mbtowc
#define towupper byte_upper
#define towlower byte_lower
#define wctype byte_class
#define iswctype byte_isclass
#define fnmatch clarice_fnmatch
#include "../vendor/musl-fnmatch/fnmatch.c"
