/* Same-build comparison: actual shell dispatch, loader, command and final paint. */
extern void shell_dispatch_legacy(int);
extern void shell_dispatch_stats(unsigned long *);
static int dispatch_probe(void){
 char report[2200],line[128];int used=0,saved=render_mode,saved_page=page_mode,saved_busy=busy;
 render_mode=0;page_mode=0;busy=1;build_font_cache();shell_run("ls /mnt/Sys6Unix/Bench/n10");term_present();
 for(int old=1;old>=0&&!cancelled;old--){shell_dispatch_legacy(old);
 for(int files=10;files<=100;files*=10){
  app_clear();for(int i=0;i<128;i++){snprintf(line,sizeof line,"---         35          0 2026-11-08 05:53 file%04d.txt\n",i);term_puts(line);}draw();
  for(int pass=0;pass<3&&!cancelled;pass++){
   unsigned long phases[4],p0=paint_ticks,t=TickCount();snprintf(line,sizeof line,"ls -la /mnt/Sys6Unix/Bench/n%d",files);
   int rc=shell_run(line);term_present();unsigned long total=TickCount()-t;shell_dispatch_stats(phases);
   used+=snprintf(report+used,sizeof report-used,"old=%d files=%d pass=%d status=%d total=%lu path=%lu kind=%lu load=%lu module=%lu paint=%lu heap=%ld\n",old,files,pass,rc,total,phases[0],phases[1],phases[2],phases[3],paint_ticks-p0,FreeMem());
  }
 }}
 shell_dispatch_legacy(0);render_mode=saved;page_mode=saved_page;busy=saved_busy;build_font_cache();dirty=force_draw=1;
 char full[PATH_CAP];unsigned char path[256];short ref;shell_temp_path(full,"dispatch-profile.txt");pstr(path,full);HCreate(0,0,path,'CLRC','TEXT');if(!HOpen(0,0,path,fsWrPerm,&ref)){long n=used;SetEOF(ref,0);FSWrite(ref,&n,report);FSClose(ref);}term_puts(report);return cancelled?130:0;
}
