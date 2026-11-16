#include <assert.h>
#include <fnmatch.h>
#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
extern int upstream_fnmatch(const char *,const char *,int);
extern int clarice_fnmatch(const char *,const char *,int);
int main(void){
 const char *patterns[]={"*","?","a*","*.txt","[ab]*","[!0-9]*","[[:digit:]][[:alpha:]]","a\\*","[]a]","[","[a-z]?","*a*b*c*","[[:space:]]","\\?","*/*","[[:punct:]]"};
 const char *names[]={"","a","b","a.txt","9x","a*", "?", "[", "]", " a", " ","a/b", ".hidden", "abc", "xyz", "-"};
 setlocale(LC_ALL,"C");unsigned count=0;
 for(unsigned p=0;p<sizeof patterns/sizeof *patterns;p++)for(unsigned n=0;n<sizeof names/sizeof *names;n++){
  int a=clarice_fnmatch(patterns[p],names[n],0)==0,b=upstream_fnmatch(patterns[p],names[n],0)==0;
  if(a!=b)fprintf(stderr,"mismatch %s / %s\n",patterns[p],names[n]);assert(a==b);count++;
 }
 assert(!clarice_fnmatch("?","\216",0));assert(!clarice_fnmatch("\216*","\216file",0));
 assert(clarice_fnmatch("[[:alpha:]]","\216",0));
 char p[1024],s[1024];memset(s,'a',1023);s[1023]=0;
 for(int i=0;i<500;i++){p[2*i]='*';p[2*i+1]='a';}p[1000]='b';p[1001]=0;
 assert(clarice_fnmatch(p,s,0));
 printf("PASS %u C-locale pristine-musl comparisons, MacRoman byte policy, long adversarial pattern; no matcher allocation\n",count);
}
