/* Delayed output must not reinterpret selected pixels. Run through the real
   compositors, including a flag-only presentation with unchanged characters. */
static void closure_probe(void){
 int theme=inverse;usability_reset();
 for(int dark=0;dark<2;dark++){
  inverse=dark;app_clear();busy=1;char line[COLS*2+2],expected[COLS*2+2];
  memset(line,'a',columns);memset(line+columns,'b',columns);line[columns*2]=0;
  term_puts(line);force_draw=dirty=1;draw();select_start=0;select_end=2*columns;
  term_puts("\n");copy_selection();correctness_scrap("pending hard newline cannot change painted soft selection",line,columns*2);
  select_start=select_end=-1;draw();select_start=0;select_end=2*columns;copy_selection();line[columns*2]='\r';
  correctness_scrap("newly presented hard ending committed even when text unchanged",line,columns*2+1);
  for(int i=0;i<ROWS+2;i++)term_puts("overwrite\n");copy_selection();correctness_scrap("ring reuse cannot change selected frame",line,columns*2+1);
  usability_reset();app_clear();tty_reading=1;memset(input,'q',columns+2);input[columns+2]='\n';strcpy(input+columns+3,"xy");cursor=strlen(input);draw();
  memset(expected,'q',columns+2);memset(expected+columns+2,' ',columns-2);expected[2*columns]='\r';
  select_start=0;select_end=2*columns;strcpy(input,"changed");cursor=7;copy_selection();correctness_scrap("pending editable input preserves presented logical boundaries",expected,2*columns+1);
  usability_reset();tty_reading=0;app_clear();term_puts("dialog exposure and cursor\n");strcpy(input,"keep");cursor=2;force_draw=dirty=1;draw();
  EventRecord ev;while(GetNextEvent(updateMask,&ev))handle(&ev);if(dirty)draw();save_preferences();
  for(int action=1;action<=2;action++){
   performance_reset();ux_full_updates=ux_clipped_updates=0;ux_dialog_action=action;
   unsigned long t=TickCount();menu((129L<<16)|4);
   usability_record("menu dialog no-op preserves clean presentation state",!dirty&&!force_draw&&!ux_layouts&&!ux_saves&&!ux_writes);
   while(GetNextEvent(updateMask,&ev))handle(&ev);if(dirty)draw();
   char b[180];snprintf(b,sizeof b,"CLOSURE theme=%d dialog=%d ticks=%lu full-updates=%lu clipped-updates=%lu writes=%lu layouts=%lu\n",dark,action,TickCount()-t,ux_full_updates,ux_clipped_updates,ux_writes,ux_layouts);long n=strlen(b);FSWrite(usability_ref,&n,b);
   usability_record("dialog exposure uses clipped native update",!ux_full_updates&&ux_clipped_updates>0);
   correctness_pixels("menu dialog dismissal exact pixels");usability_record("dialog keeps editable input/cursor",!strcmp(input,"keep")&&cursor==2);
  }
  usability_reset();
 }
 inverse=theme;force_draw=dirty=1;draw();
 char *maxcols[]={"prefs","columns","999"},*maxrows[]={"prefs","rows","999"};app_preferences(3,maxcols);app_preferences(3,maxrows);
 performance_reset();unsigned long t=TickCount();int a=app_preferences(3,maxcols),b=app_preferences(3,maxrows);
 performance_record("clamped-max",0,TickCount()-t,0,0,2);
 usability_record("maximum clamp avoids layout/save/redraw",!a&&!b&&!ux_layouts&&!ux_writes&&!ux_saves&&!ux_rows);
 char *mincols[]={"prefs","columns","1"},*minrows[]={"prefs","rows","1"};app_preferences(3,mincols);app_preferences(3,minrows);
 performance_reset();t=TickCount();a=app_preferences(3,mincols);b=app_preferences(3,minrows);performance_record("clamped-min",0,TickCount()-t,0,0,2);
 usability_record("minimum clamp avoids layout/save/redraw",!a&&!b&&!ux_layouts&&!ux_writes&&!ux_saves&&!ux_rows);
 char *cols[]={"prefs","columns","66"},*rows[]={"prefs","rows","24"};app_preferences(3,cols);app_preferences(3,rows);usability_reset();force_draw=dirty=1;draw();
}
