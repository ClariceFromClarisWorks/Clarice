#ifndef CLARICE_UPDATE_CONFIRMATION_H
#define CLARICE_UPDATE_CONFIRMATION_H
/* 1 accept, 0 cancel/EOF, -1 invalid. Enter accepts only a complete line;
 * a truncated read can never authorise replacement. */
static int update_confirmation(const char *s,long n){
 if(n<=0)return 0;
 if(s[n-1]!='\n'&&s[n-1]!='\r')return 0;
 while(n&&(s[n-1]=='\n'||s[n-1]=='\r'||s[n-1]==' '||s[n-1]=='\t'))n--;
 while(n&&(*s==' '||*s=='\t')){s++;n--;}
 if(!n)return 1;
 if(n==1&&(s[0]=='y'||s[0]=='Y'))return 1;
 if(n==3&&(s[0]=='y'||s[0]=='Y')&&(s[1]=='e'||s[1]=='E')&&(s[2]=='s'||s[2]=='S'))return 1;
 if((n==1&&(s[0]=='n'||s[0]=='N'))||(n==2&&(s[0]=='n'||s[0]=='N')&&(s[1]=='o'||s[1]=='O')))return 0;
 return -1;
}
#endif
