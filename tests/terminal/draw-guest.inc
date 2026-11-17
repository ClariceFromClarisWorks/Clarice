/* Opt-in matched renderer timings. No I/O in timed drawing loops. */
static int draw_probe(void){
 char report[4096],line[132];int used=0,saved=render_mode,saved_page=page_mode,saved_busy=busy;
 page_mode=0;busy=1;select_start=select_end=-1;
 shell_run("ls /mnt/Sys6Unix/Bench/n10");term_present();long heap_before=FreeMem();
 used+=snprintf(report+used,sizeof report-used,"DRAW_PROFILE cols=%d rows=%d cell=%d line=%d font=%d/%d inverse=%d\n",columns,visible,cell_w,line_h,font_id,font_size,inverse);
 for(int mode=0;mode<3&&!cancelled;mode++)for(int variant=0;variant<2&&!cancelled;variant++){
  draw_probe_legacy=!variant;
  render_mode=mode;build_font_cache();
  for(int kind=0;kind<3&&!cancelled;kind++){
   app_clear();busy=1;
   for(int i=0;i<128;i++){snprintf(line,sizeof line,"---         35          0 2026-11-08 05:53 file%04d.txt\n",i);term_puts(line);}draw();
   memset(profile,0,sizeof profile);draw_bar_ticks=draw_frames=0;unsigned long paint0=paint_ticks,start=TickCount(),worst=0;
   unsigned long passes[3]={0};
   if(kind==0){/* Same real command and directory for each renderer. */
    for(int pass=0;pass<3;pass++){unsigned long t=TickCount();shell_run("ls -la /mnt/Sys6Unix/Bench/n100");term_present();passes[pass]=TickCount()-t;}
   }else for(int i=0;i<24&&!cancelled;i++){
    if(kind==1){for(int j=0;j<columns;j++)line[j]=(char)('!'+(i*7+j*11)%90);line[columns]='\n';line[columns+1]=0;term_puts(line);}
    else{busy=0;scrollback=(i<12?i+1:24-i);dirty=1;}
    unsigned long t=TickCount();draw();unsigned long elapsed=TickCount()-t;if(elapsed>worst)worst=elapsed;
   }
   unsigned long total=TickCount()-start;
   used+=snprintf(report+used,sizeof report-used,"new=%d mode=%d kind=%s total=%lu paint=%lu frames=%lu compose=%lu shift=%lu raster=%lu copy=%lu bar=%lu worst=%lu heap=%ld\n",variant,mode,kind==0?"ls100":kind==1?"output":"history",total,paint_ticks-paint0,draw_frames,profile[0],profile[1],profile[2],profile[3],draw_bar_ticks,worst,FreeMem());
   if(kind==0)used+=snprintf(report+used,sizeof report-used,"ls100 passes=%lu,%lu,%lu ticks\n",passes[0],passes[1],passes[2]);
  }
 }
 draw_probe_legacy=0;render_mode=saved;page_mode=saved_page;busy=saved_busy;scrollback=0;build_font_cache();force_draw=dirty=1;
 used+=snprintf(report+used,sizeof report-used,"heap_delta=%ld (native raster includes copy)\n",FreeMem()-heap_before);
 char full[PATH_CAP];unsigned char path[256];short ref;shell_temp_path(full,"draw-profile.txt");pstr(path,full);HCreate(0,0,path,'CLRC','TEXT');if(!HOpen(0,0,path,fsWrPerm,&ref)){long n=used;SetEOF(ref,0);FSWrite(ref,&n,report);FSClose(ref);FlushVol(0,0);}
 term_puts(report);return cancelled?130:0;
}
