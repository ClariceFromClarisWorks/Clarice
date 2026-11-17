#ifndef CLARICE_UPDATE_MANIFEST_H
#define CLARICE_UPDATE_MANIFEST_H
#include <stdint.h>
#include <string.h>

typedef struct {uint32_t build,bytes;unsigned system;char version[64],package[64],crc[9];} UpdateManifest;
static int update_decimal(const char *s,uint32_t *value){uint32_t v=0;if(!*s)return 0;for(;*s;s++){if(*s<'0'||*s>'9'||v>429496729UL||(v==429496729UL&&*s>'5'))return 0;v=v*10+*s-'0';}*value=v;return 1;}
static int update_manifest(char *text,UpdateManifest *m){
 static const char *keys[]={"CLARICE-UPDATE 2","build=","version=","system=","cpu=68000","package=","bytes=","crc32="};
 memset(m,0,sizeof *m);char *p=text;uint32_t n;
 for(unsigned i=0;i<8;i++){
  char *end=strchr(p,'\n');if(!end)return -50;*end=0;
  if(i==0||i==4){if(strcmp(p,keys[i]))return -50;}
  else {size_t k=strlen(keys[i]);if(strncmp(p,keys[i],k))return -50;char *v=p+k;size_t len=strlen(v);
   if(i==1||i==3||i==6){if(!update_decimal(v,&n))return -50;if(i==1){if(!n)return -50;m->build=n;}if(i==3){if(n<0x600||n>0xffff)return -50;m->system=n;}if(i==6){if(n<128||n>33554560UL)return -50;m->bytes=n;}}
   else if(i==2){if(!len||len>=sizeof m->version)return -50;for(size_t j=0;j<len;j++)if(v[j]<33||v[j]>126)return -50;strcpy(m->version,v);}
   else if(i==5){if(!len||len>=sizeof m->package||v[0]=='.')return -50;for(size_t j=0;j<len;j++)if(!((v[j]>='a'&&v[j]<='z')||(v[j]>='A'&&v[j]<='Z')||(v[j]>='0'&&v[j]<='9')||v[j]=='.'||v[j]=='-'||v[j]=='_'))return -50;strcpy(m->package,v);}
   else {if(len!=8)return -50;for(unsigned j=0;j<8;j++)if(!((v[j]>='0'&&v[j]<='9')||(v[j]>='a'&&v[j]<='f')))return -50;strcpy(m->crc,v);}
  }p=end+1;
 }
 return *p?-50:0;
}
#endif
