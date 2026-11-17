/* Disposable native heap-pressure probe. Runs after renderer handles have
   been released, without event dispatch while holding pressure allocations. */
static Ptr admission_blocks[128];
static int admission_count;
static void admission_release(void){while(admission_count)DisposePtr(admission_blocks[--admission_count]);}
static int admission_pressure(long target){
 while(FreeMem()>target+64){
  if(admission_count==128)return 0;
  long n=FreeMem()-target-64;if(n>8192)n=8192;
  Ptr p=NewPtr(n);if(!p)return 0;admission_blocks[admission_count++]=p;
 }
 return FreeMem()<target+128;
}
static void admission_probe(void){
 char report[1500];int used=0,failures=0;Handle cache=0;Ptr p=0;
 direct_stage_dispose();glyph_planes_dispose();history_cache_dispose();
 /* Test setup only: remove unrelated purgeable resource capacity so the
    locked/no-capacity cases cannot succeed through another cache. */
 PurgeMem(0x7fffffffL);long before=FreeMem(),largest_before=MaxBlock();
 cache=NewHandle(65536);if(cache){HLock(cache);memset(*cache,0x5a,65536);}
 int setup=cache&&admission_pressure(49152);long low=FreeMem();int reclaimed=0,allocated=0,pointer_ok=0;
 if(setup){HUnlock(cache);HPurge(cache);reclaimed=native_heap_room(32768,32768,1);if(reclaimed)p=NewPtr(32768);allocated=p!=0;if(p){p[0]=0x19;p[32767]=0x27;pointer_ok=p[0]==0x19&&p[32767]==0x27;DisposePtr(p);p=0;}}
 int emptied=cache&&!*cache;int pass=setup&&reclaimed&&allocated&&pointer_ok&&emptied;if(!pass)failures++;
 used+=snprintf(report+used,sizeof report-used,"reclaim %s setup=%d low=%ld admitted=%d allocated=%d data=%d emptied=%d\n",pass?"PASS":"FAIL",setup,low,reclaimed,allocated,pointer_ok,emptied);
 admission_release();if(cache){DisposeHandle(cache);cache=0;}
 PurgeMem(0x7fffffffL);cache=NewHandle(65536);Ptr locked_address=0;if(cache){HLock(cache);HPurge(cache);locked_address=*cache;}
 setup=cache&&admission_pressure(49152);int refused=setup&&!native_heap_room(0,65536,1);int unchanged=cache&&*cache==locked_address;int recovery=0;
 if(cache){HUnlock(cache);if(setup)recovery=native_heap_room(0,65536,1)&&!*cache;}
 pass=setup&&refused&&unchanged&&recovery;if(!pass)failures++;
 used+=snprintf(report+used,sizeof report-used,"locked %s setup=%d refused=%d unchanged=%d unlock-recovery=%d\n",pass?"PASS":"FAIL",setup,refused,unchanged,recovery);
 admission_release();if(cache){DisposeHandle(cache);cache=0;}
 PurgeMem(0x7fffffffL);setup=admission_pressure(8192);low=FreeMem();refused=setup&&!native_heap_room(0,16384,1);admission_release();recovery=native_heap_room(0,16384,1);pass=setup&&refused&&recovery;if(!pass)failures++;
 used+=snprintf(report+used,sizeof report-used,"exhaustion %s low=%ld refused=%d recovery=%d\n",pass?"PASS":"FAIL",low,refused,recovery);
 long after=FreeMem();used+=snprintf(report+used,sizeof report-used,"failures=%d owned=%d heap=%ld/%ld delta=%ld largest=%ld/%ld\n",failures,admission_count,before,after,after-before,largest_before,MaxBlock());
 char full[PATH_CAP];unsigned char path[256];short ref;shell_temp_path(full,"heap-admission.txt");pstr(path,full);HCreate(0,0,path,'CLRC','TEXT');if(!HOpen(0,0,path,fsWrPerm,&ref)){long n=used;SetEOF(ref,0);FSWrite(ref,&n,report);FSClose(ref);shell_flush_native(full);}
 native_glyph_valid=0;force_draw=dirty=1;term_puts(report);
}
