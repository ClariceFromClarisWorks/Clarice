#include <stdio.h>
#include "../../src/update/release-journal.h"
#ifdef CLARICE_JOURNAL_GUEST
#include "../../src/commands/mac-files.h"
#endif
static ReleaseTransaction source,result,before;
static unsigned char encoded[RELEASE_JOURNAL_SIZE],good[RELEASE_JOURNAL_SIZE];
static void failed(int line){
#ifdef CLARICE_JOURNAL_GUEST
 (void)line;say("FAIL journal\n");
#else
 printf("FAIL journal line %d\n",line);
#endif
}
#define CHECK(x) do {if(!(x)){failed(__LINE__);return 1;}}while(0)
static void seal(void){UpdateCRC32 crc;update_crc32_init(&crc);update_crc32_bytes(&crc,encoded,sizeof encoded-4);release_put32(encoded+sizeof encoded-4,update_crc32_sum(&crc));}
static int rejected(void){result=before;return release_journal_decode(encoded,sizeof encoded,source.root,source.target,&result)&&!memcmp(&result,&before,sizeof result);}
static int journal_tests(void){
 memset(&source,0,sizeof source);strcpy(source.root,"Test Disk:System Folder:Clarice Data:");strcpy(source.target,"Test Disk:Applications:Clarice");
 memset(source.identity,0x5a,32);memset(&before,0xa5,sizeof before);
 for(unsigned i=0;i<=RELEASE_COUNT;i++){
  if(i%2){source.old[i].id=100+i;source.old[i].data=1000+i;source.old[i].resource=20+i;}
  source.next[i].id=200+i;source.next[i].data=2000+i;source.next[i].resource=40+i;
 }
 CHECK(!release_journal_encode(&source,encoded));memcpy(good,encoded,sizeof good);
 CHECK(!release_journal_decode(encoded,sizeof encoded,source.root,source.target,&result));
 CHECK(!memcmp(&source,&result,sizeof source));
 /* Allow caller to retain trusted paths in the output object itself. */
 CHECK(!release_journal_decode(encoded,sizeof encoded,result.root,result.target,&result));
 for(unsigned i=0;i<sizeof encoded;i++){
#ifdef CLARICE_JOURNAL_GUEST
  if(i>16&&i%127)continue;
#endif
  encoded[i]^=1;CHECK(rejected());encoded[i]^=1;
 }
 for(unsigned n=0;n<sizeof encoded;n++){
  result=before;CHECK(release_journal_decode(encoded,n,source.root,source.target,&result));CHECK(!memcmp(&result,&before,sizeof result));
 }
 CHECK(release_journal_decode(encoded,sizeof encoded+1,source.root,source.target,&result));
 CHECK(release_journal_decode(encoded,sizeof encoded,"Wrong Disk:Clarice Data:",source.target,&result));
 CHECK(release_journal_decode(encoded,sizeof encoded,source.root,"Test Disk:Other App",&result));
 memset(encoded+8,'A',256);seal();CHECK(rejected());memcpy(encoded,good,sizeof good);
 strcpy((char*)encoded+8,"Test Disk::Clarice Data:");seal();CHECK(rejected());memcpy(encoded,good,sizeof good);
 encoded[552]='!';seal();CHECK(rejected());memcpy(encoded,good,sizeof good);
 encoded[552+23]='x';seal();CHECK(rejected());memcpy(encoded,good,sizeof good);
 /* Inventory reorder must not redirect old identities to other commands. */
 memcpy(encoded+552,good+616,64);seal();CHECK(rejected());memcpy(encoded,good,sizeof good);
 memset(encoded+596,0,4);seal();CHECK(rejected());memcpy(encoded,good,sizeof good);
 encoded[580]=1;seal();CHECK(rejected());memcpy(encoded,good,sizeof good);
 memcpy(encoded+576,encoded+596,20);seal();CHECK(rejected());memcpy(encoded,good,sizeof good);
 memcpy(encoded,"CLRTX001",8);seal();CHECK(rejected());
#ifdef CLARICE_JOURNAL_GUEST
 say("PASS journal: round trip, paths, inventory, truncation, corruption, unchanged output on failure\n");
#else
 printf("PASS journal: round trip, trusted paths, inventory, truncation, corruption, unchanged output on failure; %lu bytes\n",(unsigned long)sizeof encoded);
#endif
 return 0;
}
#ifdef CLARICE_JOURNAL_GUEST
int clarice_main(int argc,char **argv){(void)argc;(void)argv;return journal_tests();}
#else
int main(void){return journal_tests();}
#endif
