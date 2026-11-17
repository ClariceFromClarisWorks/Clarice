/* Screen ownership for real curses applications. Ordinary shell output/history
   is retained and repainted when the owner exits. */
#include "ports/terminal/screen.h"
static CliScreen *editor_screen;
int app_screen_active(void){return editor_screen!=0;}
int app_screen_open(void){if(shell_session_active())return -1;if(editor_screen)return -1;editor_screen=(CliScreen*)NewPtr(sizeof *editor_screen);if(!editor_screen)return -1;screen_init(editor_screen,visible+1,columns);if(claim_pending_screen()){DisposePtr((Ptr)editor_screen);editor_screen=0;return -1;}app_activity_interactive(1);force_draw=dirty=1;return 0;}
void app_screen_close(void){if(editor_screen){DisposePtr((Ptr)editor_screen);editor_screen=0;app_activity_interactive(0);force_draw=dirty=1;}}
int app_screen_rows(void){return editor_screen?editor_screen->rows:visible+1;}
long app_screen_write(const void *p,unsigned long n){if(!editor_screen)return -1;
#ifdef CLARICE_NANO_PERF
unsigned long t=TickCount();
#endif
screen_feed(editor_screen,p,n);
#ifdef CLARICE_NANO_PERF
if(!nano_marks[8]){nano_writes++;nano_bytes+=n;nano_feed_ticks+=TickCount()-t;}
#endif
dirty=1;return n;}
int app_screen_pending(void){return editor_screen?editor_screen->count:0;}
static void screen_key(unsigned char c,int control){nano_input();CliScreen *s=editor_screen;const char *seq=0;if(!control&&c==28)seq="\033[D";if(!control&&c==29)seq="\033[C";if(!control&&c==30)seq="\033[A";if(!control&&c==31)seq="\033[B";unsigned n=seq?3:1;if(n>128-s->count){input_fail("application input queue overflow");return;}for(unsigned i=0;i<n;i++)s->keys[(s->head+s->count++)%128]=seq?seq[i]:c;}
long app_screen_read(void *p,unsigned long n){if(!editor_screen)return -1;
#ifdef CLARICE_INPUT_DIAGNOSTICS
 if(probe_curses){probe_curses=0;screen_key(30,0);screen_key('z',0);screen_key(3,1);screen_key('q',0);}
#endif
while(!editor_screen->count&&!cancelled&&!done){EventRecord ev;if(dirty)draw();
#ifdef CLARICE_NANO_PERF
app_nano_mark(8);
#endif
if(WaitNextEvent(everyEvent,&ev,1,0))handle(&ev);}if(cancelled||done)return -1;if(n>editor_screen->count)n=editor_screen->count;for(unsigned long i=0;i<n;i++){((unsigned char*)p)[i]=editor_screen->keys[editor_screen->head];editor_screen->head=(editor_screen->head+1)%128;editor_screen->count--;}return n;}
void app_screen_flush_input(void){if(editor_screen)editor_screen->head=editor_screen->count=0;}
static void draw_editor(void){
#ifdef CLARICE_NANO_PERF
 int measuring=!nano_marks[8];unsigned long paint_start=TickCount(),phase;
 if(measuring)nano_draws++;
#endif
 CliScreen *s=editor_screen;SetPort(&prompt_port);TextFont(font_id);TextSize(font_size);TextFace(0);TextMode(srcCopy);ForeColor(inverse?whiteColor:blackColor);BackColor(inverse?blackColor:whiteColor);
 /* Reuse the existing 64-pixel-high staging bitmap for adjacent dirty rows.
    Copy only completed strips; clipping stays with native QuickDraw. */
 int capacity=line_h>0&&line_h<=64?64/line_h:1;
 /* Match source and framebuffer word phase directly, within the same strip.
    Other displays retain the existing QuickDraw path. */
 int origin=0,width=s->cols*cell_w;
 if(!(win->portBits.rowBytes&0xc000)&&(win->portBits.rowBytes&0x3fff)==64){
  int candidate=(TEXT_LEFT-win->portBits.bounds.left)&15;
  if(candidate+width<=prompt_port.portBits.bounds.right)origin=candidate;
 }
 for(int base=0;base<s->rows;){
  if(!force_draw&&!s->dirty[base]){base++;continue;}
  int end=base+1;while(end<s->rows&&end-base<capacity&&(force_draw||s->dirty[end]))end++;
  for(int row=base;row<end;row++){Rect r;char line[SCREEN_COLS];int top=(row-base)*line_h;
   SetRect(&r,origin,top,origin+width,top+line_h);EraseRect(&r);
   for(int x=0;x<s->cols;x++)line[x]=s->cells[row][x]?s->cells[row][x]:' ';
   int length=s->cols;while(length>0&&line[length-1]==' ')length--;
   MoveTo(origin,top+font_ascent);
#ifdef CLARICE_NANO_PERF
   phase=TickCount();if(measuring)nano_rows++;
#endif
   if(mono){if(length)DrawText(line,0,length);}else for(int x=0;x<length;x++){MoveTo(origin+x*cell_w,top+font_ascent);DrawChar(line[x]);}
#ifdef CLARICE_NANO_PERF
   if(measuring)nano_text_ticks+=TickCount()-phase;
#endif
   /* Preserve the existing cursor XOR while combining inverse spans. */
   for(int x=0;x<s->cols;){
    if(!(s->attrs[row][x]^(s->cursor&&row==s->row&&x==s->col&&!background))){x++;continue;}
    int first=x++;
    while(x<s->cols&&(s->attrs[row][x]^(s->cursor&&row==s->row&&x==s->col&&!background)))x++;
    Rect c;SetRect(&c,origin+first*cell_w,top,origin+x*cell_w,top+line_h);InvertRect(&c);
   }
   s->dirty[row]=0;
  }
  Rect r,d;SetRect(&r,origin,0,origin+width,(end-base)*line_h);d=r;
  OffsetRect(&d,TEXT_LEFT-origin,TEXT_TOP+base*line_h);SetPort(win);ForeColor(blackColor);BackColor(whiteColor);
#ifdef CLARICE_NANO_PERF
  phase=TickCount();
#endif
  CopyBits(&prompt_port.portBits,&win->portBits,&r,&d,srcCopy,0);
#ifdef CLARICE_NANO_PERF
  if(measuring)nano_copy_ticks+=TickCount()-phase;
#endif
  SetPort(&prompt_port);base=end;
 }
 SetPort(win);dirty=force_draw=0;last_paint=TickCount();nano_painted();
#ifdef CLARICE_NANO_PERF
 if(measuring)nano_draw_ticks+=TickCount()-paint_start;
#endif
}
