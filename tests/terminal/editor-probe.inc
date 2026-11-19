/* Diagnostic-only real QuickDraw comparison and matched operation costs. */
static int editor_probe(void){
 if(app_screen_open())return 1;
 long size=(qd.screenBits.rowBytes&0x3fff)*(qd.screenBits.bounds.bottom-qd.screenBits.bounds.top);
 Ptr pixels=NewPtr(size);if(!pixels){app_screen_close();return 1;}
 char line[SCREEN_COLS+1],seq[40],report[512];int failures=0,theme=inverse;
 unsigned long ticks[2],cells[2],rows[2];
 for(int m=0;m<2;m++){
  editor_fast=m;for(int y=0;y<editor_screen->rows;y++){
   for(int x=0;x<editor_screen->cols;x++)line[x]='a'+(x+y)%26;line[editor_screen->cols]=0;
   snprintf(seq,sizeof seq,"\033[%d;1H",y+1);app_screen_write(seq,strlen(seq));app_screen_write(line,strlen(line));
  }force_draw=dirty=1;draw();editor_metrics_reset();unsigned long t=TickCount();
  for(int i=0;i<240;i++){snprintf(seq,sizeof seq,"\033[%d;%dH",2+i%15,2+i%40);app_screen_write(seq,strlen(seq));draw();}
  ticks[m]=TickCount()-t;cells[m]=ep_cells;rows[m]=ep_rows;
 }
 editor_fast=1;
 const char *ops[]={"\033[2;3H","\033[3;5H","\033[?25l","\033[?25h","\033[7mX\033[0m","\033[K","\033[2J","\033[Habc\033[2;1Hdef\033[H","\033[40;1H\n","\033[2;1H\033[3;2H\033[4;3H","\033[2;2H\033[2;3H\033[2;2H"};
 RgnHandle saved=NewRgn();if(!saved){DisposePtr(pixels);app_screen_close();return 1;}
 SetPort(win);GetClip(saved);
 for(int th=0;th<2;th++){
  inverse=th;force_draw=dirty=1;draw();
  for(int i=0;i<220;i++){
   SetPort(win);SetClip(saved);force_draw=dirty=1;draw();
   if(i%3){Rect clip;SetRect(&clip,TEXT_LEFT+cell_w/2,TEXT_TOP+line_h/2,TEXT_LEFT+(editor_screen->cols-1)*cell_w+1,TEXT_TOP+(editor_screen->rows-1)*line_h+2);ClipRect(&clip);}
   const char *s=ops[i%11];app_screen_write(s,strlen(s));draw();
   HideCursor();memcpy(pixels,qd.screenBits.baseAddr,size);force_draw=dirty=1;draw();
   if(memcmp(pixels,qd.screenBits.baseAddr,size))failures++;ShowCursor();
  }
 }
 SetPort(win);SetClip(saved);DisposeRgn(saved);DisposePtr(pixels);app_screen_close();inverse=theme;force_draw=dirty=1;draw();
 snprintf(report,sizeof report,"Editor probe 240 movements: old=%lu fast=%lu ticks; rows=%lu/%lu cells=%lu/%lu; 440 pixel comparisons failures=%d\n",ticks[0],ticks[1],rows[0],rows[1],cells[0],cells[1],failures);term_puts(report);return failures?1:0;
}
