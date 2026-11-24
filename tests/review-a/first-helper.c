#define main existing_recovery_suite
#include "../recovery/native.c"
#undef main
int main(void){
 file("Disk:Notes","user-owned");file("Disk:System:Clarice Data:etc:clarice:clarice.conf","user-owned");file("Disk:System:Clarice Data:bin:.clx-ls.old","user-owned");
 file("Disk:Clarice","old core");file("Disk:Helper","older helper");file("Disk:System:Clarice Data:etc:clarice:installed.catalog","CLARICE-INSTALLED 1\n");
 file("Disk:System:Clarice Data:bin:ls","old tool");file("Disk:System:Clarice Data:bin:nano-profile","diagnostic");
 RecoveryRotation r={{"Disk:System:Clarice Data:etc:clarice:Recovery Previous","Disk:System:Clarice Data:etc:clarice:Recovery Preparing","Disk:System:Clarice Data:etc:clarice:Recovery Retiring"}};
 assert(!cli_native_mkdir(r.path[1]));char dest[256];assert(!recovery_member(r.path[1],"@helper",dest));RecoveryFile f;assert(!recovery_copy("Disk:Helper",dest,"@helper",&f));
 /* Reset before Inventory is written, then use a corrected installer/helper. */
 fs[find("Disk:Helper")].fork[0][0]='N';
 unsigned old_id=fs[find(dest)].info.hFileInfo.ioDirID;
 char saved[256];snprintf(saved,sizeof saved,"Disk:System:Clarice Data:etc:clarice:Recovery Helper-%08lx",(unsigned long)old_id);
 memcpy(baseline,fs,sizeof fs);boundaries=0;
 int e=prepare_snapshot(&r);printf("first snapshot with old helper, retry=%d (expected success)\n",e);assert(!e);
 int total=boundaries;assert(find(saved)>=0);assert(!memcmp(fs[find(saved)].fork[0],"older helper",12));assert(!memcmp(fs[find(saved)].fork[1],"resource!",9));
 RecoverySnapshot s;assert(!recovery_inventory(r.path[0],&s,0));assert(!recovery_verify(r.path[0],&s));recovery_dispose(&s);closed_all();
 for(int fault=1;fault<=total;fault++){
  reset();fail_at=fault;(void)prepare_snapshot(&r);fail_at=0;closed_all();assert(!prepare_snapshot(&r));closed_all();assert(find(saved)>=0);
  reset();crash_at=fault;if(!setjmp(reset_point))(void)prepare_snapshot(&r);process_reset();assert(!prepare_snapshot(&r));closed_all();assert(find(saved)>=0);
 }
 reset();file(saved,"do not replace");assert(prepare_snapshot(&r)==paramErr);assert(!memcmp(fs[find(saved)].fork[0],"do not replace",14));assert(find(dest)>=0);closed_all();
 reset();fail_alloc=1;assert(prepare_snapshot(&r)==memFullErr);fail_alloc=0;assert(!prepare_snapshot(&r));closed_all();
 printf("PASS corrected first helper: %d errors + %d resets, collision preservation, allocation retry, both forks, zero retained allocations\n",total,total);
}
