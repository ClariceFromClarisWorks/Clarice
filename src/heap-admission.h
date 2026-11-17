#ifndef CLARICE_HEAP_ADMISSION_H
#define CLARICE_HEAP_ADMISSION_H
/* FreeMem excludes discardable handles. Admission checks must not reject a
   necessary allocation before the native Memory Manager can reclaim them.
   Cache creation passes reclaim=0: derived pixels must not consume the
   caller's recovery headroom or purge useful application resources. */
static int native_heap_room(unsigned long request,unsigned long reserve,int reclaim){
 if(reserve>0x7fffffffUL||request>0x7fffffffUL-reserve)return 0;
 unsigned long need=request+reserve;long available=FreeMem();
 if(available>=0&&(unsigned long)available>=need)return 1;
 if(!reclaim)return 0;
 PurgeMem((long)need);
 /* PurgeMem seeks contiguous space without allocating it. Fragmentation can
    produce memFullErr although the existing total-free reserve now fits.
    Keep the original admission rule; NewPtr decides actual allocation. */
 available=FreeMem();return available>=0&&(unsigned long)available>=need;
}
#endif
