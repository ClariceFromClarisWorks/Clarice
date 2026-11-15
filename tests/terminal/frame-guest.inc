/* Native frame phases; no file or screen-report I/O inside measured loops. */
static int frame_probe(void){
 char report[4096],line[COLS];int used=0,saved=render_mode,saved_page=page_mode,saved_busy=busy;
 page_mode=0;render_mode=0;build_font_cache();select_start=select_end=-1;
 long heap_before=FreeMem();
 used+=snprintf(report+used,sizeof report-used,"FRAME_PROFILE cols=%d rows=%d cell=%d line=%d font=%d/%d inverse=%d\n",columns,visible,cell_w,line_h,font_id,font_size,inverse);
 for(int run=0;run<3&&!cancelled;run++)for(int variant=0;variant<2&&!cancelled;variant++)for(int kind=0;kind<3&&!cancelled;kind++){
  frame_legacy=!variant;
  app_clear();busy=1;
  for(int i=0;i<128;i++){for(int j=0;j<columns;j++)line[j]=(char)('!'+(i*7+j*11)%90);line[columns]='\n';line[columns+1]=0;term_puts(line);}draw();
  memset(profile,0,sizeof profile);memset(frame_detail,0,sizeof frame_detail);draw_bar_ticks=draw_frames=0;
  unsigned long paint0=paint_ticks,start=TickCount(),worst=0;
  for(int i=0;i<24&&!cancelled;i++){
   if(!kind){for(int j=0;j<columns;j++)line[j]=(char)('!'+(i*7+j*11)%90);line[columns]='\n';line[columns+1]=0;term_puts(line);}
   else{busy=0;scrollback=(i<12?i+1:24-i)*(kind==1?1:4);dirty=1;}
   unsigned long t=TickCount();draw();unsigned long dt=TickCount()-t;if(dt>worst)worst=dt;
  }
  used+=snprintf(report+used,sizeof report-used,"run=%d new=%d kind=%s total=%lu paint=%lu frames=%lu compose=%lu shift=%lu pixels=%lu model=%lu raster=%lu copy=%lu bar=%lu worst=%lu\n",run,variant,kind==0?"output":kind==1?"history1":"history4",TickCount()-start,paint_ticks-paint0,draw_frames,profile[0],profile[1],frame_detail[0],frame_detail[1],profile[2],profile[3],draw_bar_ticks,worst);
 }
 frame_legacy=0;render_mode=saved;page_mode=saved_page;busy=saved_busy;scrollback=0;build_font_cache();force_draw=dirty=1;
 used+=snprintf(report+used,sizeof report-used,"heap_delta=%ld; raster includes copy, shift includes pixels/model\n",FreeMem()-heap_before);
 char full[PATH_CAP];unsigned char path[256];short ref;shell_temp_path(full,"frame-profile.txt");pstr(path,full);HCreate(0,0,path,'CLRC','TEXT');if(!HOpen(0,0,path,fsWrPerm,&ref)){long n=used;SetEOF(ref,0);FSWrite(ref,&n,report);FSClose(ref);}
 term_puts(report);return cancelled?130:0;
}
