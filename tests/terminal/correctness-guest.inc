/* Uses real Toolbox clipping and the production compositors. Test-only pixels
   are allocated and released here; nothing is added to the normal build. */
static void correctness_scrap(const char *label,const char *want,long count){
 Handle h=NewHandle(0);long off=0,n=h?GetScrap(h,'TEXT',&off):-1;
 usability_record(label,h&&n==count&&!memcmp(*h,want,count));if(h)DisposeHandle(h);
}
static void correctness_pixels(const char *label){
 int stride=win->portBits.rowBytes&0x3fff,x=-win->portBits.bounds.left,y=-win->portBits.bounds.top;
 int height=win->portRect.bottom,width=((x&7)+win->portRect.right+7)/8;
 Ptr ref=NewPtr((long)width*height);if(!ref){usability_record("pixel oracle allocation",0);return;}
 unsigned char *pixels=(unsigned char*)win->portBits.baseAddr+y*stride+(x>>3);
 HideCursor();for(int r=0;r<height;r++)memcpy(ref+r*width,pixels+r*stride,width);
 force_draw=dirty=1;draw();int same=1;for(int r=0;r<height;r++)if(memcmp(ref+r*width,pixels+r*stride,width))same=0;
 ShowCursor();DisposePtr(ref);usability_record(label,same);
}
static void correctness_update(void){
 Rect tiny;SetRect(&tiny,TEXT_LEFT,TEXT_TOP,TEXT_LEFT+3,TEXT_TOP+3);SetPort(win);InvalRect(&tiny);
 EventRecord ev;memset(&ev,0,sizeof ev);ev.what=updateEvt;ev.message=(long)win;handle(&ev);
 if(dirty)draw();
}
static void correctness_probe(void){
 int theme=inverse;long heap=FreeMem();unsigned long start=TickCount(),worst=0,total=0;
 for(int dark=0;dark<2;dark++){
  inverse=dark;usability_reset();app_clear();
  for(int i=0;i<visible;i++)term_puts("history abcdefghijklmnopqrstuvwxyz\n");
  force_draw=dirty=1;draw();key('A',0);correctness_update();
  correctness_pixels("pending prompt outside partial update: exact pixels before Return");
  key('B',0);draw();correctness_pixels("additional typing after partial update");
  usability_record("input/cursor unchanged by update",!strcmp(input,"AB")&&cursor==2);
  unsigned long commands=shell_commands;key(3,0);usability_record("cancellation without command execution",!input[0]&&shell_commands==commands);
  usability_reset();busy=1;int opened=app_screen_open();usability_record("editor opens",!opened);
  if(!opened){draw();const char *seq="\033[4;5Hpending editor";app_screen_write(seq,strlen(seq));correctness_update();
   correctness_pixels("editor pending row outside partial update");
   app_screen_write("Z",1);draw();correctness_pixels("editor subsequent output/cursor");
   app_screen_write("\033[6;7H\251",7);correctness_update();correctness_pixels("editor generic glyph fallback clipped update");app_screen_close();}
  usability_reset();draw();correctness_pixels("editor close restores terminal");
  for(int i=0;i<110;i++){char line[100];snprintf(line,sizeof line,"history %03d 0123456789 abcdefghijklmnopqrstuvwxyz\n",i);term_puts(line);}draw();
  scrollback=40;dirty=1;correctness_update();correctness_pixels("large history jump outside partial update");
  for(int i=0;i<12;i++){scroll_action(history_bar,i&1?inDownButton:inUpButton);correctness_pixels("incremental history scroll pixels");}
  selection_begin(columns+2);selection_extend(3*columns+7);correctness_pixels("selection pixels");selection_clear();
  scrollback=0;usability_reset();draw();
  for(int i=0;i<20;i++){unsigned long t=TickCount();key('x',0);draw();unsigned long dt=TickCount()-t;total+=dt;if(dt>worst)worst=dt;key(8,0);draw();}
 }
 inverse=theme;usability_reset();app_clear();busy=1;
 char line[COLS*2+1];memset(line,'a',columns);memset(line+columns,'b',columns);line[columns*2]=0;term_puts(line);term_puts("\n");draw();
 select_start=0;select_end=columns*2;copy_selection();line[columns*2]='\r';correctness_scrap("history soft wrap omits newline; real ending retained",line,columns*2+1);select_start=select_end=-1;
 app_clear();term_puts("one\ntwo\n");draw();select_start=0;select_end=2*columns;copy_selection();
 char expected[COLS*2+2];memset(expected,' ',sizeof expected);memcpy(expected,"one",3);expected[columns]='\r';memcpy(expected+columns+1,"two",3);expected[2*columns+1]='\r';correctness_scrap("hard line endings retained",expected,2*columns+2);select_start=select_end=-1;
 busy=0;app_clear();tty_reading=1;memset(input,'q',columns+3);input[columns+3]=0;cursor=columns+3;draw();select_start=0;select_end=columns+3;copy_selection();correctness_scrap("editable command soft wrap",input,columns+3);select_start=select_end=-1;
 input[3]='\n';cursor=strlen(input);draw();select_start=0;select_end=columns+3;copy_selection();
 memset(expected,' ',columns);memcpy(expected,"qqq",3);expected[columns]='\r';memcpy(expected+columns+1,"qqq",3);correctness_scrap("editable explicit newline",expected,columns+4);select_start=select_end=-1;tty_reading=0;
 usability_reset();busy=1;app_clear();current_line=visible;
 for(int r=0;r<=visible;r++){memset(text[r],'z',columns);text[r][columns]=0;wrapped[r]=r<visible;}
 force_draw=dirty=1;draw();Point p={TEXT_TOP+(visible+1)*line_h-1,TEXT_LEFT+columns*cell_w};
 selection_begin((visible+1)*columns-1);selection_extend(cell(p));copy_selection();correctness_scrap("final occupied visible cell selectable", "z",1);correctness_pixels("one-past-last selection pixels");selection_clear();
 usability_reset();draw();
 char report[180];snprintf(report,sizeof report,"correctness ticks=%lu typing samples=40 total=%lu worst=%lu heap-delta=%ld\n",TickCount()-start,total,worst,FreeMem()-heap);long n=strlen(report);FSWrite(usability_ref,&n,report);
}
