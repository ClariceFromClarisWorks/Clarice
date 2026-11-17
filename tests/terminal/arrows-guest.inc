/* Manual, paired one-row history scrolling. Test snapshots never persist. */
static unsigned char *arrows_before;
static int arrows_prepare_checks,arrows_prepare_failures;
static void arrows_observe(void){
 HideCursor();if(memcmp(arrows_before,win->portBits.baseAddr,DIRECT_STAGE_BYTES))arrows_prepare_failures++;ShowCursor();arrows_prepare_checks++;
}
static void arrows_seed(void){
 char line[COLS];app_clear();busy=1;
 for(int i=0;i<128;i++){for(int x=0;x<columns;x++)line[x]=(char)('!'+(i*7+x*11)%90);line[columns]='\n';line[columns+1]=0;term_puts(line);}
 busy=0;scrollback=0;force_draw=dirty=1;draw();
}
static int arrows_probe(void){
 if(!direct_stage_supported()||columns!=80||line_h!=11)return 2;
 unsigned char *reference=(unsigned char*)NewPtr(DIRECT_STAGE_BYTES);
 arrows_before=(unsigned char*)NewPtr(DIRECT_STAGE_BYTES);
 RgnHandle original=NewRgn(),clip=NewRgn(),hole=NewRgn();
 if(!reference||!arrows_before||!original||!clip||!hole){if(reference)DisposePtr((Ptr)reference);if(arrows_before)DisposePtr((Ptr)arrows_before);if(original)DisposeRgn(original);if(clip)DisposeRgn(clip);if(hole)DisposeRgn(hole);return 1;}
 int saved_theme=inverse,saved_busy=busy,saved_prepare=prepared_scroll_enabled,saved_direct=direct_stage_enabled;
 int used=0,checks=0,failed=0;char report[4096];SetPort(win);GetClip(original);
 input[0]=0;cursor=0;select_start=select_end=-1;arrows_prepare_checks=arrows_prepare_failures=0;
 for(int theme=0;theme<2&&!cancelled;theme++)for(int repeat=0;repeat<3&&!cancelled;repeat++)for(int variant=0;variant<2&&!cancelled;variant++){
  inverse=theme;font_layout();prepared_scroll_enabled=variant;direct_stage_enabled=0;arrows_seed();
  unsigned long frames=prepared_scroll_frames,prep=prepared_scroll_ticks,ticks=0,worst=0;scroll_exposed_ticks=scroll_exposed_worst=0;
  for(int i=0;i<24&&!cancelled;i++){
   scrollback++;dirty=1;unsigned long t=TickCount();draw();unsigned long dt=TickCount()-t;ticks+=dt;if(dt>worst)worst=dt;
   app_yield();SetPort(win);
  }
  used+=snprintf(report+used,sizeof report-used,"theme=%d repeat=%d new=%d ticks=%lu worst=%lu prepared=%lu prepare=%lu exposed=%lu exposed_worst=%lu\n",theme,repeat,variant,ticks,worst,prepared_scroll_frames-frames,prepared_scroll_ticks-prep,scroll_exposed_ticks,scroll_exposed_worst);
 }
 /* Compare the exact completed image, and observe that row preparation did
    not touch the displayed image. Narrow/hole/selected cases must fall back. */
 for(int theme=0;theme<2&&!cancelled;theme++)for(int mode=0;mode<4&&!cancelled;mode++){
  SetPort(win);SetClip(original);inverse=theme;font_layout();prepared_scroll_enabled=1;arrows_seed();
  Rect r=win->portRect;RectRgn(clip,&r);
  if(mode==1){r.left+=11;r.right-=7;r.top+=2;r.bottom-=5;RectRgn(clip,&r);}
  if(mode==2){SetRect(&r,115,65,225,107);RectRgn(hole,&r);DiffRgn(clip,hole,clip);}
  if(mode==3){select_start=columns+3;select_end=columns+12;force_draw=dirty=1;draw();}
  SetClip(clip);
  for(int i=0;i<6&&!cancelled;i++){
   HideCursor();BlockMove(win->portBits.baseAddr,arrows_before,DIRECT_STAGE_BYTES);ShowCursor();
   /* Only overwrite the output row: glyph cells live above it. */
   memset(prompt_pixels,0xa5,STRIDE*11);prepared_scroll_observer=arrows_observe;
   unsigned long frames=prepared_scroll_frames;scrollback++;dirty=1;draw();prepared_scroll_observer=0;
   if(prepared_scroll_frames-frames!=(mode==0?1:0))failed++;
   HideCursor();BlockMove(win->portBits.baseAddr,reference,DIRECT_STAGE_BYTES);ShowCursor();
   prepared_scroll_enabled=direct_stage_enabled=0;force_draw=dirty=1;draw();
   HideCursor();if(memcmp(reference,win->portBits.baseAddr,DIRECT_STAGE_BYTES))failed++;ShowCursor();checks++;
   prepared_scroll_enabled=1;app_yield();SetPort(win);SetClip(clip);
  }
  select_start=select_end=-1;
 }
 SetPort(win);SetClip(original);inverse=saved_theme;font_layout();busy=saved_busy;prepared_scroll_enabled=saved_prepare;direct_stage_enabled=saved_direct;
 prepared_scroll_observer=0;direct_stage_dispose();glyph_planes_dispose();native_glyph_valid=0;scrollback=0;force_draw=dirty=1;
 DisposePtr((Ptr)reference);DisposePtr((Ptr)arrows_before);arrows_before=0;DisposeRgn(original);DisposeRgn(clip);DisposeRgn(hole);
 failed+=arrows_prepare_failures;
 used+=snprintf(report+used,sizeof report-used,"ARROWS checks=%d prepared_unchanged=%d failures=%d cancelled=%d\n",checks,arrows_prepare_checks,failed,cancelled);
 char full[PATH_CAP];unsigned char path[256];short ref;shell_temp_path(full,"arrows.txt");pstr(path,full);HCreate(0,0,path,'CLRC','TEXT');if(!HOpen(0,0,path,fsWrPerm,&ref)){long n=used;SetEOF(ref,0);FSWrite(ref,&n,report);FSClose(ref);shell_flush_native(full);}
 term_puts(report);term_present();return cancelled?130:failed?1:0;
}
