#include <assert.h>
#include <stdio.h>
#include "../../src/update/repository.h"
static int parse(const char *s,int catalog){char line[256];strcpy(line,s);RepoRecord r;return repo_record(line,&r,catalog);}
int main(void){
 assert(!parse("tool\tls\t2026111801\t12345\t1234abcd\tls-abc.clx",0));
 assert(!parse("core\tClarice\t2026111801\t100000\tffffffff\tClarice-abc.bin",0));
 assert(!parse("core\tClarice\t2026111801\t0\t00000000",1));
 const char *bad[]={"tool\t../ls\t1\t36\t00000000\tx.clx","tool\tLS\t1\t36\t00000000\tx.clx","tool\tls\t4294967296\t36\t00000000\tx.clx","tool\tls\t1\t35\t00000000\tx.clx","tool\tls\t1\t36\t0000000g\tx.clx","tool\tls\t1\t36\t00000000\t../x","tool\tls\t1\t36\t00000000\tx\textra","core\tOther\t1\t128\t00000000\tx"};
 for(unsigned i=0;i<sizeof bad/sizeof *bad;i++)assert(parse(bad[i],0));
 RepoRecord a={"ls","",2026111801,100,0x1234abcd,0},b;char line[256];assert(!repo_line(&a,line,sizeof line));line[strlen(line)-1]=0;assert(!repo_record(line,&b,1));assert(!strcmp(a.name,b.name)&&a.version==b.version&&a.size==b.size&&a.crc==b.crc);
 puts("PASS repository/catalog records, bounds, paths, overflow and roundtrip");
}
