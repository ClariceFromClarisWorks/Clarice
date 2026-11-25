/* _PostEvent register interface; Multiversal lacks the POSTEVENT glue. */
static int performance_post(unsigned char c){register long kind __asm__("a0")=keyDown;register long message __asm__("d0")=c;__asm__ volatile(".word 0xa02f" : "+a"(kind), "+d"(message) : : "d1","a1","cc","memory");return (short)message;}
/* Matched boundary events, no command execution or disk I/O in draw timing. */
static void performance_record(const char *kind,int variant,unsigned long ticks,unsigned long worst,unsigned long delay,int n){
 char b[240];snprintf(b,sizeof b,"PERF %s variant=%d samples=%d ticks=%lu worst=%lu event-to-visible=%lu rows=%lu raster-pixels=%lu moved-pixels=%lu saves=%lu writes=%lu layouts=%lu\n",kind,variant,n,ticks,worst,delay,ux_rows,ux_pixels,ux_moves,ux_saves,ux_writes,ux_layouts);long used=strlen(b);FSWrite(usability_ref,&used,b);
}
static void performance_reset(void){ux_rows=ux_pixels=ux_moves=ux_saves=ux_writes=ux_layouts=0;}
static void performance_probe(void){
 int theme=inverse;usability_reset();app_clear();
 for(int i=0;i<110;i++){char line[COLS];for(int x=0;x<columns;x++)line[x]='!'+(i*7+x*11)%90;line[columns]='\n';line[columns+1]=0;term_puts(line);}draw();
 for(int variant=0;variant<4;variant++){
  inverse=variant&1;ux_wrap_legacy=variant<2;unsigned long ticks=0,worst=0,delay=0,rows=0,pixels=0,moves=0;
  for(int i=0;i<12;i++){
   char prefix[132];int lead=strlen(text[current_line%ROWS])+prompt_prefix(prefix),n=columns*(1+i%3)-lead-1;
   memset(input,'x',n);input[n]=0;cursor=n;scrollback=0;force_draw=dirty=1;draw();performance_reset();
   EventRecord ev;while(GetNextEvent(updateMask,&ev))handle(&ev);performance_reset();
   unsigned long queued=TickCount(),t=queued;performance_post('z');
   while(strlen(input)==(unsigned)n&&TickCount()-queued<60){if(WaitNextEvent(keyDownMask,&ev,0,0)){t=TickCount();handle(&ev);}}
   draw();unsigned long dt=TickCount()-t,age=TickCount()-queued;
   usability_record("queued boundary character processed once",strlen(input)==(unsigned)n+1&&input[n]=='z');
   ticks+=dt;if(dt>worst)worst=dt;if(age>delay)delay=age;rows+=ux_rows;pixels+=ux_pixels;moves+=ux_moves;
   correctness_pixels("wrap-boundary exact pixels");
   key(8,0);draw();correctness_pixels("unwrap backspace exact pixels");
  }
  performance_reset();ux_rows=rows;ux_pixels=pixels;ux_moves=moves;performance_record("wrap",variant,ticks,worst,delay,12);
 }
 ux_wrap_legacy=0;inverse=theme;usability_reset();force_draw=dirty=1;draw();save_preferences();
 char cols[16],rows[16],size[16];snprintf(cols,sizeof cols,"%d",columns);snprintf(rows,sizeof rows,"%d",visible+1);snprintf(size,sizeof size,"%d",font_size);
 char *values[]={cols,rows,size,inverse?"dark":"light",(char*)control_name(control_mode)};char *names[]={"columns","rows","size","colours","control"};
 for(int i=0;i<5;i++){char *args[]={"prefs",names[i],values[i]};performance_reset();unsigned long t=TickCount();int rc=app_preferences(3,args);unsigned long dt=TickCount()-t;performance_record(names[i],0,dt,dt,0,1);usability_record("unchanged prefs succeeds",rc==0);}
 /* Equal OK and Cancel still let the Window Manager restore exposed pixels. */
 for(int i=0;i<2;i++){EventRecord ev;while(GetNextEvent(updateMask,&ev))handle(&ev);draw();performance_reset();unsigned long t=TickCount();ux_dialog_action=i?2:1;preferences_window();unsigned long dt=TickCount()-t;performance_record(i?"dialog-cancel":"dialog-ok",0,dt,dt,0,1);usability_record("unchanged dialog avoids layout/save",!ux_layouts&&!ux_saves);while(GetNextEvent(updateMask,&ev))handle(&ev);if(dirty)draw();correctness_pixels("dialog restoration pixels");}
 performance_reset();menu((133L<<16)|4);usability_record("unchanged size menu avoids layout/save",!ux_layouts&&!ux_saves);
 char saved[PATH_CAP];strcpy(saved,prefs_path);strcpy(prefs_path,"ClariceMissingVolume:clarice.conf");preferences_unsaved=1;
 char *same[]={"prefs","size",size};performance_reset();int rc=app_preferences(3,same);usability_record("failed save stays retryable without layout",rc==1&&preferences_unsaved&&!ux_layouts);
 strcpy(prefs_path,saved);rc=app_preferences(3,same);usability_record("same-value retry durably saves",!rc&&!preferences_unsaved&&ux_writes==1&&!ux_layouts);
 usability_reset();force_draw=dirty=1;draw();
}
