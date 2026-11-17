extern void shell_dispatch_stats(unsigned long *);
/* Real submission path through execute(), including final prompt presentation.
   Controlled fixture only: /tmp/base/ls is the prior core and /tmp/next/ls the
   candidate; this probe restores the fixture's standard /bin:/usr/bin PATH.
   Fixed dataset/seed/geometry; no synthetic output inside the timed region. */
static int listing_render_probe(void){
 char report[4096],line[100];int used=0,saved=render_mode,saved_page=page_mode,saved_busy=busy;
 page_mode=0;select_start=select_end=-1;
 shell_run("ls /mnt/Sys6Unix/Bench/n10");term_present();
 for(int cadence=96;cadence<=96&&!cancelled;cadence+=64){listing_paint_interval=cadence;
 for(int size=100;size<=1000&&!cancelled;size*=10){
 for(int pass=0;pass<3&&!cancelled;pass++)for(int variant=0;variant<2&&!cancelled;variant++){
  /* Interleave prior/candidate formatting and output at the same cadence. */
  int mode=0;output_legacy=variant==0;render_mode=mode;build_font_cache();
  shell_run(variant?"export PATH=/tmp/next:/bin:/usr/bin":"export PATH=/tmp/base:/bin:/usr/bin");
  app_clear();busy=1;for(int i=0;i<128;i++){snprintf(line,sizeof line,"---         35          0 2026-11-08 05:53 file%04d.txt\n",i);term_puts(line);}draw();
  busy=0;snprintf(input,sizeof input,"ls -la /mnt/Sys6Unix/Bench/n%d",size);cursor=strlen(input);dirty=1;draw();
  unsigned long p0=paint_ticks,f0=draw_frames,e0=event_ticks;listing_partial_frames=0;unsigned long t=TickCount();execute();term_present();unsigned long total=TickCount()-t,phases[4];shell_dispatch_stats(phases);
  used+=snprintf(report+used,sizeof report-used,"variant=%d mode=%d cadence=%d files=%d pass=%d total=%lu paint=%lu frames=%lu events=%lu partial=%lu status=%d heap=%ld path=%lu load=%lu module=%lu\n",variant,mode,cadence,size,pass,total,paint_ticks-p0,draw_frames-f0,event_ticks-e0,listing_partial_frames,shell_status,FreeMem(),phases[0],phases[2],phases[3]);
 }
 }}
 output_legacy=0;shell_run("export PATH=/bin:/usr/bin");listing_paint_interval=96;render_mode=saved;page_mode=saved_page;busy=saved_busy;build_font_cache();force_draw=dirty=1;
 char full[PATH_CAP];unsigned char path[256];short ref;shell_temp_path(full,"listing-render.txt");pstr(path,full);HCreate(0,0,path,'CLRC','TEXT');if(!HOpen(0,0,path,fsWrPerm,&ref)){long n=used;SetEOF(ref,0);FSWrite(ref,&n,report);FSClose(ref);}term_puts(report);return cancelled?130:0;
}
