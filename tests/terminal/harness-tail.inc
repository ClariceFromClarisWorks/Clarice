
static unsigned long rng=1;
static unsigned int next(void){rng=(rng*1664525+1013904223)&0xffffffffUL;return rng>>8;}
int main(void){char saved[ROWS][COLS],input[8192];int widths[]={1,2,40,80,130};int trials=0;
for(int w=0;w<5;w++)for(int retention=32;retention<=128;retention*=2)for(int trial=0;trial<100;trial++){
 for(int i=0;i<8192;i++){unsigned int v=next();input[i]=(v%9)?(char)(32+v%224):(char)(v%32);}
 columns=widths[w];retained=retention;long expected[6];
 for(int version=0;version<2;version++){
 memset(text,0,sizeof text);current_line=oldest_line=out_col=scrollback=last_cr=dirty=0;
 for(int i=0;i<8192;){int n=1+(i*17+trial)%193;if(n>8192-i)n=8192-i;if(version)term_write(input+i,n);else reference_write(input+i,n);i+=n;if(i>4096&&scroll_limit()>5)scrollback=5;}
 if(!version){memcpy(saved,text,sizeof text);expected[0]=current_line;expected[1]=oldest_line;expected[2]=out_col;expected[3]=scrollback;expected[4]=last_cr;expected[5]=dirty;}
 else{assert(!memcmp(saved,text,sizeof text));assert(expected[0]==current_line&&expected[1]==oldest_line&&expected[2]==out_col&&expected[3]==scrollback&&expected[4]==last_cr&&expected[5]==dirty);}
 }trials++;
}printf("Terminal writer equivalence: %d randomized chunked cases PASS (5 widths, 3 retention limits, controls/high bytes/wrap/anchored history).\n",trials);}
