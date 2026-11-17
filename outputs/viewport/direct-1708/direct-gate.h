#ifndef CLARICE_DIRECT_GATE_H
#define CLARICE_DIRECT_GATE_H
#include <stdint.h>
/* Terminal rows are even-aligned. Compare their first up-to-32 bytes using
   native longwords. An equal prefix conservatively declines the direct path;
   otherwise preserve the original changed-span rejection. This avoids scanning
   long identical directory prefixes without admitting previously rejected
   sparse rows into the more expensive whole-viewport eligibility scan. */
typedef uint32_t DirectGateWord __attribute__((aligned(2),may_alias));
static int direct_stage_sparse_first(const char *a,const char *b,int columns){
 int prefix=columns<32?columns:32,left=0,right=columns;
 while(left+4<=prefix){
  if(*(const DirectGateWord*)(a+left)!=*(const DirectGateWord*)(b+left))break;
  left+=4;
 }
 while(left<prefix&&a[left]==b[left])left++;
 if(left==prefix)return 1;
 while(right>left&&a[right-1]==b[right-1])right--;
 return (right-left)*4<columns*3;
}
#endif
