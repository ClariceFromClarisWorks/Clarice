/* Diagnostic-only real QuickDraw comparison and matched operation costs. */
static int editor_probe(void){
 if(app_screen_open())return 1;editor_glyphs=1;
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
   HideCursor();memcpy(pixels,qd.screenBits.baseAddr,size);editor_glyphs=0;force_draw=dirty=1;draw();editor_glyphs=1;
   if(memcmp(pixels,qd.screenBits.baseAddr,size))failures++;ShowCursor();
  }
 }
 /* Dense printable rows, partial four-cell groups, high-byte fallback and
    clipped edges: compare the real shared m68000 compositor with DrawText. */
 int original_cols=editor_screen->cols;
 for(int th=0;th<2;th++)for(int test=0;test<64;test++){
  inverse=th;editor_screen->cols=original_cols-test%4;
  for(int row=0;row<editor_screen->rows;row++)for(int x=0;x<editor_screen->cols;x++){
   editor_screen->cells[row][x]=32+(x*13+row*7+test)%95;
   editor_screen->attrs[row][x]=(x+row+test)%9==0;
  }
  if(test%8==0)editor_screen->cells[1][2]=200;
  editor_screen->row=test%editor_screen->rows;editor_screen->col=test%editor_screen->cols;
  SetPort(win);SetClip(saved);
  if(test%3){Rect clip;SetRect(&clip,TEXT_LEFT+3,TEXT_TOP+5,TEXT_LEFT+(editor_screen->cols-1)*cell_w+2,TEXT_TOP+(editor_screen->rows-1)*line_h+3);ClipRect(&clip);}
  memset(prompt_pixels,0xa5,STRIDE*11);editor_glyphs=1;force_draw=dirty=1;draw();
  HideCursor();memcpy(pixels,qd.screenBits.baseAddr,size);
  editor_glyphs=0;force_draw=dirty=1;draw();
  if(memcmp(pixels,qd.screenBits.baseAddr,size))failures++;ShowCursor();
 }
 editor_screen->cols=original_cols;editor_glyphs=1;
 SetPort(win);SetClip(saved);DisposeRgn(saved);DisposePtr(pixels);app_screen_close();inverse=theme;force_draw=dirty=1;draw();
 snprintf(report,sizeof report,"Editor probe 240 movements: old=%lu fast=%lu ticks; rows=%lu/%lu cells=%lu/%lu; 568 pixel comparisons failures=%d\n",ticks[0],ticks[1],rows[0],rows[1],cells[0],cells[1],failures);term_puts(report);
 char full[PATH_CAP];unsigned char path[256];short ref;shell_temp_path(full,"editor-pixels.txt");pstr(path,full);
 HCreate(0,0,path,'CLRC','TEXT');if(!HOpen(0,0,path,fsWrPerm,&ref)){long n=strlen(report);SetEOF(ref,0);FSWrite(ref,&n,report);FSClose(ref);shell_flush_native(full);}
 return failures?1:0;
}
