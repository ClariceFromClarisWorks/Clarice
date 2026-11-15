/* Dirty-buffer equivalence against unchanged QuickDraw, including actual window
   occlusion and nonrectangular clipping. All storage is diagnostic and released. */
static int alignment_pixels(void){
 if((win->portBits.rowBytes&0xffff)!=64)return 2;
 long heap=FreeMem();unsigned char *reference=(unsigned char*)NewPtr(64*342);RgnHandle original=NewRgn(),clip=NewRgn(),hole=NewRgn();
 if(!reference||!original||!clip||!hole){if(reference)DisposePtr((Ptr)reference);if(original)DisposeRgn(original);if(clip)DisposeRgn(clip);if(hole)DisposeRgn(hole);return 1;}
 SetPort(win);GetClip(original);WindowPtr cover=0;int checks=0,failures=0;
 for(int mode=0;mode<4&&!cancelled;mode++){
  if(mode==3){Rect r;SetRect(&r,170,75,340,145);cover=NewWindow(0,&r,(ConstStringPtr)"\007Overlap",true,0,(WindowPtr)-1,false,0);if(!cover){failures++;break;}SetPort(cover);EraseRect(&cover->portRect);}
  for(int phase=0;phase<16&&!cancelled;phase++){
   SetPort(win);Rect c;SetRect(&c,TEXT_LEFT,TEXT_TOP,TEXT_LEFT+columns*cell_w,TEXT_TOP+visible*line_h);RectRgn(clip,&c);
   if(mode==1){c.left+=19;c.right-=11;c.top+=3;c.bottom=TEXT_TOP+42;RectRgn(clip,&c);}
   if(mode==2){SetRect(&c,TEXT_LEFT+101,TEXT_TOP+2,TEXT_LEFT+223,TEXT_TOP+39);RectRgn(hole,&c);DiffRgn(clip,hole,clip);}
   SetClip(clip);ForeColor(blackColor);BackColor(whiteColor);
   for(int n=0;n<5;n++){
    int widths[]={6,18,51,464,510};Rect source,dest;int left=n==2?13:0;
    int y=mode==3?50:5;SetRect(&source,left,2,left+widths[n],11);SetRect(&dest,TEXT_LEFT+phase-7,TEXT_TOP+y,TEXT_LEFT+phase-7+widths[n],TEXT_TOP+y+9);
    HideCursor();
    for(int variant=0;variant<2;variant++){
     for(int i=0;i<4096;i++)prompt_pixels[i]=(unsigned char)(i*13+phase*7+n*31);
     align_legacy=!variant;present_native_row(&source,&dest);
     if(!variant)memcpy(reference,win->portBits.baseAddr,64*342);
     else if(memcmp(reference,win->portBits.baseAddr,64*342))failures++;
    }
    checks++;ShowCursor();
   }
   native_glyph_valid=0;SetClip(original);app_yield();
  }
 }
 /* Compare complete terminal redraws while actually obscured and after
    uncovering, not just a dirty row above the covering window. */
 for(int uncovered=0;uncovered<2;uncovered++){
  if(uncovered&&cover){DisposeWindow(cover);cover=0;SelectWindow(win);}
  SetPort(win);SetClip(original);native_glyph_valid=0;HideCursor();
  align_legacy=1;force_draw=dirty=1;draw();memcpy(reference,win->portBits.baseAddr,64*342);
  align_legacy=0;force_draw=dirty=1;draw();if(memcmp(reference,win->portBits.baseAddr,64*342))failures++;checks++;ShowCursor();
 }
 SetPort(win);SetClip(original);
 InvalRect(&win->portRect);native_glyph_valid=0;force_draw=dirty=1;DisposeRgn(original);DisposeRgn(clip);DisposeRgn(hole);DisposePtr((Ptr)reference);char report[200];snprintf(report,sizeof report,"ALIGN_PIXELS %s checks=%d failures=%d heap_delta=%ld\n",failures?"FAIL":"PASS",checks,failures,FreeMem()-heap);
 char full[PATH_CAP];unsigned char path[256];short ref;shell_temp_path(full,"align-pixels.txt");pstr(path,full);HCreate(0,0,path,'CLRC','TEXT');if(!HOpen(0,0,path,fsWrPerm,&ref)){long n=strlen(report);SetEOF(ref,0);FSWrite(ref,&n,report);FSClose(ref);}
 term_puts(report);return cancelled?130:failures?1:0;
}
