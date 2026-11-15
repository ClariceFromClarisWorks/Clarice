#include <assert.h>
#include <stdio.h>
#include "../../src/update/manifest.h"
static char text[1024];
static const char valid[]="CLARICE-UPDATE 2\nbuild=2026111402\nversion=0.4-dev.2\nsystem=1536\ncpu=68000\npackage=Clarice-2026111402.bin\nbytes=832512\ncrc32=01234567\n";
int main(void){UpdateManifest m;strcpy(text,valid);assert(!update_manifest(text,&m)&&m.build==2026111402&&m.system==1536&&m.bytes==832512);
 for(unsigned i=0;i<sizeof valid-1;i++){memcpy(text,valid,i);text[i]=0;assert(update_manifest(text,&m));}
 strcpy(text,valid);strcat(text,"extra\n");assert(update_manifest(text,&m));
 const char *bad[]={"../evil.bin",".hidden", "http://evil", "a/b", "a:b", "a b", ""};
 for(unsigned i=0;i<sizeof bad/sizeof *bad;i++){snprintf(text,sizeof text,"CLARICE-UPDATE 2\nbuild=1\nversion=test\nsystem=1536\ncpu=68000\npackage=%s\nbytes=128\ncrc32=01234567\n",bad[i]);assert(update_manifest(text,&m));}
 strcpy(text,valid);strstr(text,"crc32=")[6]='g';assert(update_manifest(text,&m));
 strcpy(text,valid);strstr(text,"UPDATE 2")[7]='1';assert(update_manifest(text,&m));
 uint32_t n;assert(update_decimal("4294967295",&n)&&n==0xffffffff);assert(!update_decimal("4294967296",&n));assert(!update_decimal("-1",&n));assert(!update_decimal("",&n));
 puts("PASS: bounded strict manifest, every truncation, trailing/unknown fields, filename traversal/control, integer overflow");}
