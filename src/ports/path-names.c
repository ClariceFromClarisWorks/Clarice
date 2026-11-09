/* Slash-path adapters: never pass Unix shell paths to host Macintosh parsing. */
#include <string.h>
char *port_path_basename(char *s){if(!s||!*s)return ".";size_t n=strlen(s);while(n>1&&s[n-1]=='/')s[--n]=0;char *p=strrchr(s,'/');return p&&p[1]?p+1:s;}
char *port_path_dirname(char *s){if(!s||!*s)return ".";size_t n=strlen(s);while(n>1&&s[n-1]=='/')s[--n]=0;char *p=strrchr(s,'/');if(!p)return ".";while(p>s&&p[-1]=='/')p--;if(p==s){s[1]=0;return s;}*p=0;return s;}
