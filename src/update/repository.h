/* Small line protocol for the private repository and installed catalog.
 * No network or filesystem implementation here. Names are /bin leaves, not
 * arbitrary paths. CRC-32 detects corruption; it is not authentication. */
#ifndef CLARICE_REPOSITORY_H
#define CLARICE_REPOSITORY_H
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "manifest.h"
typedef struct {char name[21],file[100];uint32_t version,size,crc;int core;} RepoRecord;
static int repo_word(const char *p,int filename){
 size_t n=strlen(p);if(!n||n>(filename?99:20)||p[0]=='.')return 0;
 for(;*p;p++)if(!((*p>='a'&&*p<='z')||(*p>='A'&&*p<='Z'&&filename)||(*p>='0'&&*p<='9')||*p=='-'||*p=='_'||(filename&&*p=='.')))return 0;
 return 1;
}
static int repo_record(char *line,RepoRecord *r,int catalog){
 char *f[6];unsigned count=0;f[count++]=line;
 for(char *p=line;*p;p++)if(*p=='\t'){if(count==6)return -50;*p=0;f[count++]=p+1;}
 if(count!=(catalog?5:6))return -50;memset(r,0,sizeof *r);
 if(!strcmp(f[0],"core"))r->core=1;else if(strcmp(f[0],"tool"))return -50;
 if(r->core?strcmp(f[1],"Clarice")!=0:!repo_word(f[1],0))return -50;
 strcpy(r->name,f[1]);if(!update_decimal(f[2],&r->version)||!r->version||!update_decimal(f[3],&r->size))return -50;
 if((!catalog&&r->size<(r->core?128:36))||r->size>(r->core?33554560UL:368688UL))return -50;
 if(strlen(f[4])!=8)return -50;
 for(unsigned i=0;i<8;i++){unsigned c=(unsigned char)f[4][i];if(c>='0'&&c<='9')c-='0';else if(c>='a'&&c<='f')c=c-'a'+10;else return -50;r->crc=(r->crc<<4)|c;}
 if(!catalog){if(!repo_word(f[5],1))return -50;strcpy(r->file,f[5]);}
 return 0;
}
static int repo_line(const RepoRecord *r,char *out,unsigned cap){
 int n=snprintf(out,cap,"%s\t%s\t%lu\t%lu\t%08lx\n",r->core?"core":"tool",r->name,(unsigned long)r->version,(unsigned long)r->size,(unsigned long)r->crc);
 return n<0||(unsigned)n>=cap?-50:0;
}
#endif
