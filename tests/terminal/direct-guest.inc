/* Compare completed viewport bytes with the unchanged per-row presenter.
   Snapshot is test-only. Deliberately retain the original clip/vis in both paths. */
static int direct_view_pixels(void){
 if(!direct_stage_supported())return 2;
 unsigned char *reference=(unsigned char*)NewPtr(DIRECT_STAGE_BYTES);
 RgnHandle original=NewRgn(),clip=NewRgn(),hole=NewRgn();
 if(!reference||!original||!clip||!hole){if(reference)DisposePtr((Ptr)reference);if(original)DisposeRgn(original);if(clip)DisposeRgn(clip);if(hole)DisposeRgn(hole);return 1;}
 int checks=0,failures=0,saved_theme=inverse,saved_coherent=direct_stage_enabled;WindowPtr cover=0;
 SetPort(win);GetClip(original);history_raster_enabled=0;planes_enabled=2;direct_stage_invert_ram=1;input[0]=0;cursor=0;Ptr real_bits=win->portBits.baseAddr;
 for(int mode=0;mode<4;mode++){
  if(mode==3){Rect r;SetRect(&r,170,75,340,145);cover=NewWindow(0,&r,(ConstStringPtr)"\007Overlap",true,0,(WindowPtr)-1,false,0);if(!cover){failures++;break;}}
  for(int theme=0;theme<2;theme++)for(int selected=0;selected<2;selected++)for(int forced=0;forced<2;forced++){
   SetPort(win);SetClip(original);inverse=theme;font_layout();direct_stage_enabled=0;select_start=select_end=-1;
   app_clear();busy=1;char line[COLS];for(int i=0;i<128;i++){for(int j=0;j<columns;j++)line[j]=(char)('!'+(i*7+j*11)%90);line[columns]='\n';line[columns+1]=0;term_puts(line);}busy=0;
   force_draw=dirty=1;draw();
   Rect c=win->portRect;RectRgn(clip,&c);
   if(mode==1){c.left+=19;c.right-=11;c.top+=3;c.bottom=TEXT_TOP+42;RectRgn(clip,&c);}
   if(mode==2){SetRect(&c,TEXT_LEFT+101,TEXT_TOP+2,TEXT_LEFT+223,TEXT_TOP+39);RectRgn(hole,&c);DiffRgn(clip,hole,clip);}
   SetClip(clip);scrollback=37;select_start=selected?columns+3:-1;select_end=selected?3*columns+9:-1;force_draw=dirty=1;draw();
   HideCursor();BlockMove(win->portBits.baseAddr,reference,DIRECT_STAGE_BYTES);ShowCursor();
   /* Repaint an older source view, then take the experimental jump. */
   scrollback=5;force_draw=dirty=1;draw();scrollback=37;direct_stage_enabled=1;force_draw=forced;dirty=1;unsigned long staged_before=direct_stage_frames;draw();
   if(direct_stage_frames!=staged_before+(selected?0:1))failures++;
   HideCursor();int mismatch=win->portBits.baseAddr!=real_bits||memcmp(reference,win->portBits.baseAddr,DIRECT_STAGE_BYTES)!=0;ShowCursor();checks++;if(mismatch)failures++;
  }
  SetPort(win);SetClip(original);if(cover){DisposeWindow(cover);cover=0;SetPort(win);}
 }
 SetPort(win);SetClip(original);select_start=select_end=-1;direct_stage_enabled=0;scrollback=37;force_draw=dirty=1;draw();
 HideCursor();BlockMove(win->portBits.baseAddr,reference,DIRECT_STAGE_BYTES);ShowCursor();
 for(int fault=0;fault<3;fault++){
  direct_stage_enabled=0;scrollback=5;force_draw=dirty=1;draw();
  if(fault==0){if(!direct_stage_handle)direct_stage_handle=NewHandle(DIRECT_STAGE_BYTES);if(direct_stage_handle&&*direct_stage_handle){HLock(direct_stage_handle);memset(*direct_stage_handle,0xa5,DIRECT_STAGE_BYTES);HUnlock(direct_stage_handle);}}
  else if(fault==1){if(direct_stage_handle)EmptyHandle(direct_stage_handle);}
  else {direct_stage_dispose();direct_stage_fail_alloc=1;}
  direct_stage_enabled=1;scrollback=37;dirty=1;unsigned long staged_before=direct_stage_frames;draw();direct_stage_fail_alloc=0;
  if(direct_stage_frames!=staged_before+(fault==2?0:1))failures++;
  HideCursor();int mismatch=win->portBits.baseAddr!=real_bits||memcmp(reference,win->portBits.baseAddr,DIRECT_STAGE_BYTES)!=0;ShowCursor();checks++;if(mismatch)failures++;
 }
 direct_stage_dispose();if(direct_stage_handle||win->portBits.baseAddr!=real_bits)failures++;
 SetClip(original);DisposeRgn(original);DisposeRgn(clip);DisposeRgn(hole);DisposePtr((Ptr)reference);
 inverse=saved_theme;direct_stage_enabled=saved_coherent;select_start=select_end=-1;scrollback=0;busy=0;force_draw=dirty=1;
 char report[256],full[PATH_CAP];unsigned char path[256];short ref;
 snprintf(report,sizeof report,"DIRECT_STAGE_PIXELS %s checks=%d failures=%d; full/narrow/hole/occluded, themes, selection, forced/incremental; dirty/purged/OOM staging\n",failures?"FAIL":"PASS",checks,failures);
 shell_temp_path(full,"direct-stage-pixels.txt");pstr(path,full);HCreate(0,0,path,'CLRC','TEXT');if(!HOpen(0,0,path,fsWrPerm,&ref)){long n=strlen(report);SetEOF(ref,0);FSWrite(ref,&n,report);FSClose(ref);shell_flush_native(full);}
 term_puts(report);return failures?1:0;
}
