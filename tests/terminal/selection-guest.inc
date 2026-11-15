/* Test-build-only: exact drag painting path, guest ticks and pixel readback. */
static void selection_probe(void){
 char report[4096],line[100];int used=0,saved=render_mode,old_inverse=inverse;
 unsigned char *reference=(unsigned char*)NewPtr(STRIDE*HEIGHT);if(!reference)return;
 busy=1;for(int i=0;i<110;i++){snprintf(line,sizeof line,"selection row %03d abcdefghijklmnopqrstuvwxyz 0123456789\n",i);term_puts(line);}busy=0;
 for(int mode=0;mode<3;mode+=2)for(int view=0;view<2;view++){
 render_mode=mode;build_font_cache();scrollback=view?35:0;inverse=view;select_start=select_end=-1;force_draw=dirty=1;draw();
 int stride=win->portBits.rowBytes&0x3fff,px=TEXT_LEFT-win->portBits.bounds.left,py=TEXT_TOP-win->portBits.bounds.top,width=((px&7)+columns*cell_w+7)/8,height=visible*line_h;
 unsigned char *pixels=(unsigned char*)win->portBits.baseAddr+py*stride+(px>>3);int valid=px>=0&&py>=0&&py+height<=342&&width<=STRIDE;int failures=0;unsigned long total=0,worst=0,still=0;long heap=FreeMem();
 select_start=select_end=2*columns+5;
 for(int i=0;i<12;i++){
 int end=(i<6?i+2:13-i)*columns+(i*7)%columns;unsigned long start=TickCount();
#ifdef CLARICE_SELECTION_FIXED
 selection_extend(end);
#else
 select_end=end;force_draw=dirty=1;draw();
#endif
 unsigned long ticks=TickCount()-start;total+=ticks;if(ticks>worst)worst=ticks;
 if(valid){HideCursor();for(int y=0;y<height;y++)memcpy(reference+y*STRIDE,pixels+y*stride,width);ShowCursor();force_draw=dirty=1;draw();HideCursor();for(int y=0;y<height;y++)if(memcmp(reference+y*STRIDE,pixels+y*stride,width)){failures++;break;}ShowCursor();}
 }
 unsigned long start=TickCount();for(int i=0;i<12;i++){
#ifdef CLARICE_SELECTION_FIXED
 selection_extend(select_end);
#else
 force_draw=dirty=1;draw();
#endif
 }still=TickCount()-start;
#ifdef CLARICE_SELECTION_FIXED
 /* Removing a selection before changing views must not leave inverted cache
    pixels behind, including starting another selection on the new page. */
 scroll_action(history_bar,inUpButton);selection_begin(columns+3);selection_extend(4*columns+7);
 if(valid){HideCursor();for(int y=0;y<height;y++)memcpy(reference+y*STRIDE,pixels+y*stride,width);ShowCursor();force_draw=dirty=1;draw();HideCursor();for(int y=0;y<height;y++)if(memcmp(reference+y*STRIDE,pixels+y*stride,width)){failures++;break;}ShowCursor();}
 selection_clear();
 if(valid){HideCursor();for(int y=0;y<height;y++)memcpy(reference+y*STRIDE,pixels+y*stride,width);ShowCursor();force_draw=dirty=1;draw();HideCursor();for(int y=0;y<height;y++)if(memcmp(reference+y*STRIDE,pixels+y*stride,width)){failures++;break;}ShowCursor();}
#endif
 used+=snprintf(report+used,sizeof report-used,"mode=%d scrollback=%d inverse=%d updates=12 ticks=%lu worst=%lu stationary_ticks=%lu pixel_valid=%d mismatches=%d heap_delta=%ld\n",mode,scrollback,inverse,total,worst,still,valid,failures,FreeMem()-heap);
 /* Compare the prior forced repaint with the production key-after-selection
    path. Include the prompt row in the pixel oracle. No command is executed. */
 for(int selected=0;selected<2;selected++){
  unsigned long key_ticks[2];int key_failures=0;
  int key_height=(visible+1)*line_h;if(key_height>HEIGHT)key_height=HEIGHT;
  for(int variant=0;variant<2;variant++){
   input[0]=0;cursor=0;scrollback=0;selection_clear();force_draw=dirty=1;draw();
   selection_begin(columns+3);if(selected)selection_extend(4*columns+7);
   unsigned long key_start=TickCount();
   if(!variant){force_draw=1;select_start=select_end=-1;}
   key('x',0);draw();key_ticks[variant]=TickCount()-key_start;
   if(valid&&py+key_height<=342){HideCursor();
    if(!variant)for(int y=0;y<key_height;y++)memcpy(reference+y*STRIDE,pixels+y*stride,width);
    else for(int y=0;y<key_height;y++)if(memcmp(reference+y*STRIDE,pixels+y*stride,width)){key_failures++;break;}
    ShowCursor();
   }else key_failures++;
  }
  used+=snprintf(report+used,sizeof report-used,"key-after-click mode=%d inverse=%d selected=%d old=%lu new=%lu ticks mismatches=%d\n",mode,inverse,selected,key_ticks[0],key_ticks[1],key_failures);
 }
 input[0]=0;cursor=0;

 }
 /* Idle history scrolling keeps the prompt/cursor below the moved region.
    Compare the exact legacy guard with the fixed guard, including pixel oracle. */
 for(int mode=0;mode<3;mode+=2)for(int legacy=1;legacy>=0;legacy--){
  render_mode=mode;build_font_cache();inverse=0;busy=0;input[0]=0;cursor=0;
  select_start=select_end=-1;scrollback=0;force_draw=dirty=1;draw();scroll_probe_legacy=legacy;
  int stride=win->portBits.rowBytes&0x3fff,px=TEXT_LEFT-win->portBits.bounds.left,py=TEXT_TOP-win->portBits.bounds.top;
  int width=((px&7)+columns*cell_w+7)/8,height=(visible+1)*line_h,failures=0;
  unsigned char *pixels=(unsigned char*)win->portBits.baseAddr+py*stride+(px>>3);
  int offsets[]={1,2,3,4,8,12,11,10,6,2,1,0};unsigned long ticks=0,worst=0;long heap=FreeMem();
  for(unsigned i=0;i<sizeof offsets/sizeof offsets[0];i++){
   scrollback=offsets[i];dirty=1;unsigned long start=TickCount();draw();unsigned long elapsed=TickCount()-start;ticks+=elapsed;if(elapsed>worst)worst=elapsed;
   if(px>=0&&py>=0&&height<=HEIGHT&&py+height<=342&&width<=STRIDE){
    HideCursor();for(int y=0;y<height;y++)memcpy(reference+y*STRIDE,pixels+y*stride,width);ShowCursor();
    force_draw=dirty=1;draw();HideCursor();for(int y=0;y<height;y++)if(memcmp(reference+y*STRIDE,pixels+y*stride,width)){failures++;break;}ShowCursor();
   }else failures++;
  }
  used+=snprintf(report+used,sizeof report-used,"history-scroll mode=%d legacy=%d steps=12 ticks=%lu worst=%lu mismatches=%d heap_delta=%ld\n",mode,legacy,ticks,worst,failures,FreeMem()-heap);
 }
 scroll_probe_legacy=0;

 DisposePtr((Ptr)reference);render_mode=saved;inverse=old_inverse;build_font_cache();select_start=select_end=-1;scrollback=0;force_draw=dirty=1;
 char full[PATH_CAP];unsigned char path[256];short ref;shell_temp_path(full,"qol3a-selection.txt");pstr(path,full);HCreate(0,0,path,'CLRC','TEXT');if(!HOpen(0,0,path,fsWrPerm,&ref)){long n=used;SetEOF(ref,0);FSWrite(ref,&n,report);FSClose(ref);FlushVol(0,0);}term_puts(report);term_puts("SELECTION-PROBE-DONE\n");
}
