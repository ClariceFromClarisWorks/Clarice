/* Diagnostic only. Batch timings avoid per-row TickCount aliasing. No retained
   allocation, screen format changes or production presentation shortcuts. */
static int presentation_probe(void){
 char report[4096];int used=0;unsigned char scratch[704] __attribute__((aligned(2)));
 BitMap memory;Rect source,dest;long heap=FreeMem();
 if(cell_w!=6||line_h!=11||columns<80||FrontWindow()!=win)return 2;
 draw();SetPort(win);ForeColor(blackColor);BackColor(whiteColor);
 memory.baseAddr=(Ptr)scratch;memory.rowBytes=64;SetRect(&memory.bounds,0,0,512,11);
 int px=TEXT_LEFT-win->portBits.bounds.left,aligned=TEXT_LEFT+((16-(px&15))&15);
 used+=snprintf(report+used,sizeof report-used,"PRESENT_PROFILE px=%d stride=%d; 300 copies/case; monochrome srcCopy no scaling\n",px,win->portBits.rowBytes&0x3fff);
 for(int run=0;run<3&&!cancelled;run++){
  for(int variant=0;variant<2;variant++)for(int n=0;n<2;n++){
   clear_legacy=!variant;int count=n?51:80;unsigned long start=TickCount();
   for(int i=0;i<2000;i++)native_glyph_clear(count);
   used+=snprintf(report+used,sizeof report-used,"clear run=%d new=%d count=%d calls=2000 ticks=%lu\n",run,variant,count,TickCount()-start);
  }
  clear_legacy=0;
  for(int kind=0;kind<9&&!cancelled;kind++){
   int width=kind==5||kind==7?16:464;
   SetRect(&source,0,0,width,11);SetRect(&dest,TEXT_LEFT,TEXT_TOP,TEXT_LEFT+width,TEXT_TOP+11);
   if(kind==2)OffsetRect(&dest,aligned-TEXT_LEFT,0);
   if(kind==8)OffsetRect(&source,px&15,0);
   if(kind>=3&&kind<=6)SetRect(&dest,kind==3?(px&15):0,0,(kind==3?(px&15):0)+width,11);
   memset(prompt_pixels,0x5a,704);memset(scratch,0xa5,sizeof scratch);
   unsigned long start=TickCount();
   for(int i=0;i<300;i++){
    if(kind==0)present_native_row(&source,&dest);
    else if(kind==6)asm_copy_rows(scratch,prompt_pixels,58,11,64,64);
    else CopyBits(&prompt_port.portBits,kind>=3&&kind<=5?&memory:&win->portBits,&source,&dest,srcCopy,0);
   }
   used+=snprintf(report+used,sizeof report-used,"copy run=%d kind=%d width=%d ticks=%lu\n",run,kind,width,TickCount()-start);
   app_yield();SetPort(win);ForeColor(blackColor);BackColor(whiteColor);
  }
 }
 clear_legacy=0;native_glyph_valid=0;force_draw=dirty=1;
 used+=snprintf(report+used,sizeof report-used,"heap_delta=%ld; kinds=helper,screen,aligned-screen,shifted-memory,aligned-memory,tiny-memory,asm-memory,tiny-screen,matched-offset-screen\n",FreeMem()-heap);
 char full[PATH_CAP];unsigned char path[256];short ref;shell_temp_path(full,"presentation-profile.txt");pstr(path,full);HCreate(0,0,path,'CLRC','TEXT');
 if(!HOpen(0,0,path,fsWrPerm,&ref)){long n=used;SetEOF(ref,0);FSWrite(ref,&n,report);FSClose(ref);}
 term_puts(report);return cancelled?130:0;
}
