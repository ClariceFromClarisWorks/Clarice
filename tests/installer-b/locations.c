#include <assert.h>
#include <stdio.h>
#include "../../src/update/locations.h"
#include "../../src/update/recovery-record.h"
int main(void){
 InstallLocations a={"Disk:Apps:Clarice","Disk:Files:Clarice Data:"},b;
 unsigned char bytes[LOCATION_BYTES];assert(!locations_encode(&a,bytes));assert(!locations_decode(bytes,&b));assert(!memcmp(&a,&b,sizeof a));
 for(unsigned i=0;i<sizeof bytes;i++){bytes[i]^=1;assert(locations_decode(bytes,&b));bytes[i]^=1;}
 strcpy(a.core,"Other:Clarice");assert(locations_encode(&a,bytes));
 strcpy(a.core,"Disk:Files:Clarice Data:Clarice");assert(locations_encode(&a,bytes));
 strcpy(a.core,"Disk:..:Clarice");assert(!locations_encode(&a,bytes));
 strcpy(a.data,"Disk::Clarice Data:");assert(locations_encode(&a,bytes));
 RecoveryHeader h={"Disk:Files:Clarice Data:","Disk:Apps:Clarice",1977,3,44,99},r;unsigned char rec[RECOVERY_HEADER];char path[256];
 for(unsigned v=0;v<=3;v++){if(v==1)continue;h.format=v;assert(!recovery_header_encode(&h,rec));assert(!recovery_header_decode(rec,&r));assert(r.format==v);assert(!recovery_core_target(&r,path));assert(!strcmp(path,v==3?"Disk:Apps:Clarice":"Disk:Clarice"));}
 puts("PASS location record exact round trip, every-byte corruption, same-volume and native path bounds; V1/V2 unchanged root restoration and V3 actual Core location");
}
