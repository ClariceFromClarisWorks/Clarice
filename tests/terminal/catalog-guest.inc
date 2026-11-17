/* Matched actual rendering cost for small streamed listing updates. */
static int catalog_probe(void){
 char report[4096],line[100];int used=0,saved=render_mode,saved_busy=busy;int steps[]={1,2,4,8,12,24};
 busy=1;select_start=select_end=-1;
 for(int mode=0;mode<=2;mode+=2){render_mode=mode;build_font_cache();
 for(int variant=0;variant<2;variant++){catalog_force_scroll=variant?1:-1;
 for(int k=0;k<6;k++){
  app_clear();for(int i=0;i<128;i++){snprintf(line,sizeof line,"---         35          0 2026-11-08 05:53 file%04d.txt\n",i);term_puts(line);}draw();
  unsigned long total=0,worst=0;memset(profile,0,sizeof profile);draw_bar_ticks=0;
  for(int pass=0;pass<6;pass++){
   for(int j=0;j<steps[k];j++){snprintf(line,sizeof line,"---         35          0 2026-11-08 05:53 file%04d.txt\n",128+pass*steps[k]+j);term_puts(line);}
   unsigned long t=TickCount();draw();unsigned long dt=TickCount()-t;total+=dt;if(dt>worst)worst=dt;
  }
  used+=snprintf(report+used,sizeof report-used,"mode=%d scroll=%d step=%d frames=6 ticks=%lu worst=%lu compose=%lu shift=%lu raster=%lu copy=%lu bar=%lu heap=%ld\n",mode,variant,steps[k],total,worst,profile[0],profile[1],profile[2],profile[3],draw_bar_ticks,FreeMem());
  if(app_yield())goto finish;
 }}}
finish:
 catalog_force_scroll=0;render_mode=saved;busy=saved_busy;build_font_cache();force_draw=dirty=1;
 char full[PATH_CAP];unsigned char path[256];short ref;shell_temp_path(full,"catalog-profile.txt");pstr(path,full);HCreate(0,0,path,'CLRC','TEXT');if(!HOpen(0,0,path,fsWrPerm,&ref)){long n=used;SetEOF(ref,0);FSWrite(ref,&n,report);FSClose(ref);}term_puts(report);return cancelled?130:0;
}
