/* Screen ownership for real curses applications. Ordinary shell output/history
   is retained and repainted when the owner exits. */
#include "ports/terminal/screen.h"
static CliScreen *editor_screen;
int app_screen_active(void){return editor_screen!=0;}
int app_screen_open(void){if(editor_screen)return -1;editor_screen=(CliScreen*)NewPtr(sizeof *editor_screen);if(!editor_screen)return -1;screen_init(editor_screen,visible+1,columns);app_activity_interactive(1);force_draw=dirty=1;return 0;}
void app_screen_close(void){if(editor_screen){DisposePtr((Ptr)editor_screen);editor_screen=0;app_activity_interactive(0);force_draw=dirty=1;}}
int app_screen_rows(void){return editor_screen?editor_screen->rows:visible+1;}
long app_screen_write(const void *p,unsigned long n){if(!editor_screen)return -1;screen_feed(editor_screen,p,n);dirty=1;return n;}
int app_screen_pending(void){return editor_screen?editor_screen->count:0;}
static void screen_key(unsigned char c){CliScreen *s=editor_screen;const char *seq=0;if(c==28)seq="\033[D";if(c==29)seq="\033[C";if(c==30)seq="\033[A";if(c==31)seq="\033[B";unsigned n=seq?3:1;if(n>128-s->count){input_fail("application input queue overflow");return;}for(unsigned i=0;i<n;i++)s->keys[(s->head+s->count++)%128]=seq?seq[i]:c;}
long app_screen_read(void *p,unsigned long n){if(!editor_screen)return -1;
#ifdef CLARICE_INPUT_DIAGNOSTICS
 if(probe_curses){probe_curses=0;screen_key(30);screen_key('z');screen_key(3);screen_key('q');}
#endif
while(!editor_screen->count&&!cancelled&&!done){EventRecord ev;if(dirty)draw();if(WaitNextEvent(everyEvent,&ev,1,0))handle(&ev);}if(cancelled||done)return -1;if(n>editor_screen->count)n=editor_screen->count;for(unsigned long i=0;i<n;i++){((unsigned char*)p)[i]=editor_screen->keys[editor_screen->head];editor_screen->head=(editor_screen->head+1)%128;editor_screen->count--;}return n;}
void app_screen_flush_input(void){if(editor_screen)editor_screen->head=editor_screen->count=0;}
static void draw_editor(void){CliScreen *s=editor_screen;SetPort(&prompt_port);TextFont(font_id);TextSize(font_size);TextFace(0);TextMode(srcCopy);ForeColor(inverse?whiteColor:blackColor);BackColor(inverse?blackColor:whiteColor);
 for(int row=0;row<s->rows;row++)if(force_draw||s->dirty[row]){Rect r,d;char line[SCREEN_COLS];SetRect(&r,0,0,s->cols*cell_w,line_h);EraseRect(&r);for(int x=0;x<s->cols;x++)line[x]=s->cells[row][x]?s->cells[row][x]:' ';MoveTo(0,font_ascent);if(mono)DrawText(line,0,s->cols);else for(int x=0;x<s->cols;x++){MoveTo(x*cell_w,font_ascent);DrawChar(line[x]);}
 for(int x=0;x<s->cols;x++)if(s->attrs[row][x]^(s->cursor&&row==s->row&&x==s->col&&!background)){Rect c;SetRect(&c,x*cell_w,0,(x+1)*cell_w,line_h);InvertRect(&c);}
 d=r;OffsetRect(&d,TEXT_LEFT,TEXT_TOP+row*line_h);SetPort(win);ForeColor(blackColor);BackColor(whiteColor);CopyBits(&prompt_port.portBits,&win->portBits,&r,&d,srcCopy,0);SetPort(&prompt_port);s->dirty[row]=0;}
 SetPort(win);dirty=force_draw=0;last_paint=TickCount();}
