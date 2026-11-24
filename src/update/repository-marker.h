/* Version 2 binds retries to one checked repository. Version 1 remains readable
 * with snapshot-member checks; CRC detects accidental changes, not tampering. */
#ifndef CLARICE_REPOSITORY_MARKER_H
#define CLARICE_REPOSITORY_MARKER_H
#include <stdint.h>
#include <string.h>
#include <stdio.h>
static int repo_marker_parse(const char *text,uint32_t *size,uint32_t *crc,const char **body,int *bound){
 const char *prefixes[]={"CLARICE-UPDATE-INCOMPLETE ","CLARICE-UPDATE-READY "};
 *bound=0;*size=*crc=0;
 for(int k=0;k<2;k++){size_t n=strlen(prefixes[k]);if(strncmp(text,prefixes[k],n))continue;
  const char *p=text+n;if((p[0]!='1'&&p[0]!='2')||p[1]!='\n')return -50;int v=p[0]-'0';p+=2;
  if(v==2){if(strlen(p)<18||p[8]!=' '||p[17]!='\n')return -50;
   uint32_t values[2]={0,0};for(int i=0;i<2;i++)for(int j=0;j<8;j++){unsigned c=(unsigned char)p[i*9+j];if(c>='0'&&c<='9')c-='0';else if(c>='a'&&c<='f')c=c-'a'+10;else return -50;values[i]=(values[i]<<4)|c;}
   *size=values[0];*crc=values[1];if(!*size)return -50;*bound=1;p+=18;
  }
  if(!k&&*p)return -50;*body=p;return k+1;
 }return -50;
}
static int repo_marker_header(char *text,unsigned cap,int ready,uint32_t size,uint32_t crc){
 int n=snprintf(text,cap,"CLARICE-UPDATE-%s 2\n%08lx %08lx\n",ready?"READY":"INCOMPLETE",(unsigned long)size,(unsigned long)crc);
 return n<0||(unsigned)n>=cap?-50:n;
}
#endif
