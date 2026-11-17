/* Paired, actual terminal-output + completed-paint probe. Test-only snapshot.
   Runs with command input ownership; yields only between complete comparisons. */
static int live_output_probe(void){
 if(!direct_stage_supported()||columns!=80||line_h!=11)return 2;
 unsigned char *reference=(unsigned char*)NewPtr(DIRECT_STAGE_BYTES);
 char (*batch)[COLS]=(char(*)[COLS])NewPtr(48L*COLS);
 RgnHandle saved_clip_live=NewRgn(),probe_clip=NewRgn(),hole=NewRgn();
 if(!batch||!reference||!saved_clip_live||!probe_clip||!hole){if(batch)DisposePtr((Ptr)batch);if(reference)DisposePtr((Ptr)reference);if(saved_clip_live)DisposeRgn(saved_clip_live);if(probe_clip)DisposeRgn(probe_clip);if(hole)DisposeRgn(hole);return 1;}
 char report[8192],line[COLS];int used=0,failed=0,checks=0;
 int saved_theme=inverse,saved_busy=busy,saved_live=direct_live_enabled,saved_direct=direct_stage_enabled,saved_page=paging;
 SetPort(win);GetClip(saved_clip_live);busy=1;paging=0;input[0]=0;cursor=0;select_start=select_end=-1;
 for(int theme=0;theme<2&&!cancelled;theme++)for(int pass=0;pass<3&&!cancelled;pass++)for(int dense=0;dense<2&&!cancelled;dense++)for(int chunk=4;chunk<=48&&!cancelled;chunk+=44)for(int variant=0;variant<2&&!cancelled;variant++){
  inverse=theme;SetPort(win);SetClip(saved_clip_live);font_layout();app_clear();busy=1;direct_live_enabled=0;direct_stage_enabled=1;
  int serial=0;
  for(int i=0;i<128;i++){native_test_line(line,serial++);term_puts(line);}force_draw=dirty=1;draw();
  /* Warm glyphs equally; stage allocation remains part of the candidate cost. */
  direct_stage_dispose();direct_live_enabled=variant;unsigned long ticks=0,worst=0,frames=0,stage=0,output_ticks=0,paint_ticks=0;
  SetPort(win);RectRgn(probe_clip,&win->portRect);
  if(pass==1){Rect r=win->portRect;r.left+=17;r.right-=13;r.top+=2;r.bottom-=9;RectRgn(probe_clip,&r);}
  if(pass==2){Rect r;SetRect(&r,115,39,225,87);RectRgn(hole,&r);DiffRgn(probe_clip,hole,probe_clip);}
  SetClip(probe_clip);
  for(int frame=0;frame<6&&!cancelled;frame++){
   /* Fixture formatting is not terminal work. Prepare identical bytes outside
      the timed interval; include only terminal ingestion and completed paint. */
   for(int i=0;i<chunk;i++){
    if(dense){for(int x=0;x<columns;x++)batch[i][x]=(char)('!'+(serial*7+x*11)%90);batch[i][columns]='\n';batch[i][columns+1]=0;serial++;}
    else snprintf(batch[i],COLS,"---         35          0 2026-11-08 05:53 file%04d.txt\n",serial++);
   }
   unsigned long paint_before=draw_frames,stage_before=direct_stage_frames,t=TickCount();
   for(int i=0;i<chunk;i++)term_puts(batch[i]);
   unsigned long middle=TickCount();draw();unsigned long end=TickCount(),dt=end-t;
   output_ticks+=middle-t;paint_ticks+=end-middle;ticks+=dt;if(dt>worst)worst=dt;
   frames+=draw_frames-paint_before;stage+=direct_stage_frames-stage_before;
   HideCursor();BlockMove(win->portBits.baseAddr,reference,DIRECT_STAGE_BYTES);ShowCursor();
   int live=direct_live_enabled,staged=direct_stage_enabled;direct_live_enabled=direct_stage_enabled=0;force_draw=dirty=1;draw();
   HideCursor();if(memcmp(reference,win->portBits.baseAddr,DIRECT_STAGE_BYTES))failed++;ShowCursor();checks++;
   direct_live_enabled=live;direct_stage_enabled=staged;
   SetClip(saved_clip_live);app_yield();SetPort(win);SetClip(probe_clip);
  }
  used+=snprintf(report+used,sizeof report-used,"theme=%d pass=%d dense=%d chunk=%d new=%d ticks=%lu output=%lu paint=%lu worst=%lu paints=%lu direct=%lu failures=%d\n",theme,pass,dense,chunk,variant,ticks,output_ticks,paint_ticks,worst,frames,stage,failed);
 }
 /* Current partial rows, canonical reads and selections must use fallback. */
 SetPort(win);SetClip(saved_clip_live);inverse=saved_theme;font_layout();app_clear();busy=1;
 for(int i=0;i<80;i++){for(int x=0;x<columns;x++)line[x]=(char)('!'+(i*7+x*11)%90);line[columns]='\n';line[columns+1]=0;term_puts(line);}
 for(int mode=0;mode<5&&!cancelled;mode++){
  tty_reading=mode==1;searching=mode==2;paging=mode==3;select_start=mode==4?columns:-1;select_end=mode==4?columns+10:-1;
  if(mode==0)term_puts("partial");
  direct_stage_enabled=0;force_draw=dirty=1;draw();HideCursor();BlockMove(win->portBits.baseAddr,reference,DIRECT_STAGE_BYTES);ShowCursor();
  direct_stage_enabled=direct_live_enabled=1;unsigned long before=direct_stage_frames;force_draw=dirty=1;draw();
  HideCursor();if(memcmp(reference,win->portBits.baseAddr,DIRECT_STAGE_BYTES)||before!=direct_stage_frames)failed++;ShowCursor();checks++;
  if(mode==0)term_puts("\n");SetClip(saved_clip_live);app_yield();
 }
 tty_reading=searching=0;paging=saved_page;select_start=select_end=-1;inverse=saved_theme;busy=saved_busy;direct_live_enabled=saved_live;direct_stage_enabled=saved_direct;
 direct_stage_dispose();glyph_planes_dispose();native_glyph_valid=0;SetPort(win);SetClip(saved_clip_live);font_layout();scrollback=0;force_draw=dirty=1;
 DisposePtr((Ptr)batch);DisposePtr((Ptr)reference);DisposeRgn(saved_clip_live);DisposeRgn(probe_clip);DisposeRgn(hole);
 used+=snprintf(report+used,sizeof report-used,"LIVE_OUTPUT checks=%d failures=%d cancelled=%d\n",checks,failed,cancelled);
 char full[PATH_CAP];unsigned char path[256];short ref;shell_temp_path(full,"live-output.txt");pstr(path,full);HCreate(0,0,path,'CLRC','TEXT');if(!HOpen(0,0,path,fsWrPerm,&ref)){long n=used;SetEOF(ref,0);FSWrite(ref,&n,report);FSClose(ref);shell_flush_native(full);}
 term_puts(report);term_present();return cancelled?130:failed?1:0;
}
