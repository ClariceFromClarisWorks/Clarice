#include <assert.h>
#include <stdio.h>
#include "../../src/update/recovery-record.h"
int main(void){
 RecoveryHeader h={"Startup Disk:System:Clarice Data:","Startup Disk:System:Clarice",1947,51,47,99},r;
 unsigned char b[RECOVERY_HEADER];assert(!recovery_header_encode(&h,b));assert(!recovery_header_decode(b,&r));assert(!memcmp(&h,&r,sizeof h));
 h.format=2;assert(!recovery_header_encode(&h,b));assert(!memcmp(b,"CLRSNAP2",8));assert(!recovery_header_decode(b,&r)&&r.format==2);h.format=0;
 char path[256];assert(!recovery_install_path(h.root,path)&&!strcmp(path,"Startup Disk:Clarice"));assert(recovery_install_path(":wrong:",path));assert(recovery_install_path("Disk::Clarice Data:",path));
 RecoveryFile f={0},g;strcpy(f.name,"@core");f.id=29;f.data=0;f.resource=395008;f.finder[0]=42;unsigned char e[RECOVERY_ENTRY];assert(!recovery_file_encode(&f,e));assert(!recovery_file_decode(e,&g)&&!memcmp(&f,&g,sizeof f));
 f.checksum_policy=1;assert(!recovery_file_encode(&f,e));assert(!recovery_file_decode(e,&g)&&g.checksum_policy==1);f.checksum_policy=3;assert(recovery_file_encode(&f,e));f.checksum_policy=0;
 assert(!recovery_member("Disk:System:Recovery Previous","@core",path)&&!strcmp(path,"Disk:System:Recovery Previous:Core"));
 assert(recovery_member("Disk:System:Recovery Previous","../ls",path));
 strcpy(f.name,"ls");assert(!recovery_file_encode(&f,e));e[95]=1;assert(recovery_file_decode(e,&g));
 strcpy(h.origin,"Other:Clarice");assert(recovery_header_encode(&h,b));
 puts("PASS recovery inventory codecs, startup-volume location, path/record rejection");
}
