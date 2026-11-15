/* Large retained-history jumps. Report I/O and seeding are outside paint timing. */
static int jump_probe(void){
 char report[4096],line[COLS];int used=0,saved=render_mode,saved_page=page_mode,saved_busy=busy;
 render_mode=0;page_mode=0;build_font_cache();select_start=select_end=-1;
 long heap_before=FreeMem();
 used+=snprintf(report+used,sizeof report-used,"JUMP_PROFILE cols=%d rows=%d cell=%d line=%d inverse=%d\n",columns,visible,cell_w,line_h,inverse);
 for(int run=0;run<3&&!cancelled;run++)for(int variant=0;variant<2&&!cancelled;variant++)for(int kind=0;kind<2&&!cancelled;kind++){
  jump_legacy=!variant;native_glyph_valid=0;glyph_build_ticks=0;
  app_clear();busy=1;
  for(int i=0;i<128;i++){
   if(!kind)snprintf(line,sizeof line,"---         35          0 2026-11-08 05:53 file%04d.txt\n",i);
   else{for(int j=0;j<columns;j++)line[j]=(char)('!'+(i*7+j*11)%90);line[columns]='\n';line[columns+1]=0;}
   term_puts(line);
  }
  busy=0;draw();memset(profile,0,sizeof profile);memset(jump_detail,0,sizeof jump_detail);draw_bar_ticks=draw_frames=0;
  unsigned long total=0,worst=0;
  for(int i=0;i<24&&!cancelled;i++){
   scrollback=(i*29+17)%(scroll_limit()+1);dirty=1;
   unsigned long start=TickCount();draw();unsigned long dt=TickCount()-start;total+=dt;if(dt>worst)worst=dt;
   if(i%6==5)app_yield();
  }
  used+=snprintf(report+used,sizeof report-used,"run=%d new=%d kind=%s ticks=%lu worst=%lu frames=%lu compose=%lu shift=%lu raster=%lu copy=%lu bar=%lu plan=%lu prepare=%lu text=%lu rows=%lu build=%lu\n",run,variant,kind?"varied":"listing",total,worst,draw_frames,profile[0],profile[1],profile[2],profile[3],draw_bar_ticks,jump_detail[0],jump_detail[1],jump_detail[2],jump_detail[3],glyph_build_ticks);
 }
 jump_legacy=0;render_mode=saved;page_mode=saved_page;busy=saved_busy;scrollback=0;build_font_cache();force_draw=dirty=1;
 used+=snprintf(report+used,sizeof report-used,"heap_delta=%ld; phases overlap; total is sum of completed draw calls\n",FreeMem()-heap_before);
 char full[PATH_CAP];unsigned char path[256];short ref;shell_temp_path(full,"jump-profile.txt");pstr(path,full);HCreate(0,0,path,'CLRC','TEXT');if(!HOpen(0,0,path,fsWrPerm,&ref)){long n=used;SetEOF(ref,0);FSWrite(ref,&n,report);FSClose(ref);}
 term_puts(report);return cancelled?130:0;
}
