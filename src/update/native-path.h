/* Existing recovery native pathname rules, shared with location discovery. */
#ifndef CLARICE_NATIVE_PATH_H
#define CLARICE_NATIVE_PATH_H
#include <string.h>
static inline int recovery_path(const char *s,int dir){
 size_t n=strlen(s);if(!n||n>255||s[0]==':'||(dir?(s[n-1]!=':'):(s[n-1]==':')))return 0;
 const char *start=s;int separators=0;
 for(const char *p=s;*p;p++){if((unsigned char)*p<32)return 0;if(*p==':'){if(p==start||p-start>31)return 0;start=p+1;separators++;}}
 return separators&&strlen(start)<=31;
}
/* namespace_install resolved the running System's actual volume and folder.
   Take its volume component; do not guess the name "System Folder". */
static inline int recovery_install_path(const char *root,char out[256]){
 if(!recovery_path(root,1))return -50;size_t n=strchr(root,':')-root+1;
 memcpy(out,root,n);strcpy(out+n,"Clarice");return 0;
}
#endif
