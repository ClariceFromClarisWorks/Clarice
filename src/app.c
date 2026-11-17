#include "sys6.h"
#include <Quickdraw.h>
#include <Fonts.h>
#include <Windows.h>
#include <Menus.h>
#include <Events.h>
#include <Dialogs.h>
#include <OSUtils.h>
#include <ToolUtils.h>
#include <MacMemory.h>
#include <Processes.h>
#include <Multiverse.h>
#include <Files.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "reflow.h"
#include "display-size.h"
#include "hardware.h"
#include "update/version.h"
#ifdef CLARICE_UPDATE_DIAGNOSTICS
static void handoff_trace(const char *s){unsigned char p[]="\053Sys6Unix:HIL Results:handoff-trace.txt";p[0]=strlen((char*)p+1);short ref;Create(p,0,'CLRC','TEXT');if(!FSOpen(p,0,&ref)){SetFPos(ref,fsFromLEOF,0);long n=strlen(s);FSWrite(ref,&n,s);n=1;FSWrite(ref,&n,"\n");FSClose(ref);FlushVol(0,0);}}
#define UPDATE_LAUNCH_TRACE(s) handoff_trace(s)
#else
#define UPDATE_LAUNCH_TRACE(s) ((void)0)
#endif
#include "update/native-launch.h"

/* Multiversal's fixed low-memory pointer triggers GCC array-bounds warnings
   after inlining. Read the same documented word explicitly on the 68000. */
static short menu_bar_height(void){short h;__asm__ volatile("move.w 0x0baa,%0":"=d"(h)::"memory");return h;}
#define ROWS 128
#define COLS 132
#define STRIDE 64
#define HEIGHT 336
#define TEXT_TOP 0
#define TEXT_LEFT 4
extern unsigned long sys6_memtop(void);
static WindowPtr win;
static WindowRecord win_record;
static char text[ROWS][COLS] __attribute__((aligned(2)));
extern long asm_copy_printable(char *,const char *,long);
extern void asm_clear_terminal_row(char *);
_Static_assert(COLS==132,"assembly terminal-row clear must match COLS");
static unsigned char wrapped[ROWS];
static char last_screen[40][COLS] __attribute__((aligned(2)));
extern long asm_terminal_row_changed(const char *,const char *);

static unsigned char *frame_pixels,*glyph_cache;
static int retained=128;
static GrafPort frame_port,prompt_port;
static unsigned char prompt_pixels[STRIDE*64] __attribute__((aligned(2)));
/* Matched real listings favour native for everyday commands without a resident
   framebuffer. Cached backends remain available to diagnostic comparisons only. */
static int frame_ready,glyph_ready,render_mode=0;
static RgnHandle transfer_region,visible_region;
static ControlHandle history_bar;
static void sync_history_bar(int force);
static unsigned char bar_pixels[4*342];
static Ptr tall_bar_pixels;static int tall_bar_rows;
static int bar_testing,bar_failures,bar_checks;
static int bar_snapshot(unsigned short *,int);
static int scroll_limit(void);
static void build_font_cache(void);
static void draw_native(void);
static int region_contains(RgnHandle,const Rect *);
extern void asm_copy_rows(unsigned char *,const unsigned char *,long,long,long,long);
static long current_line,oldest_line;
static int last_cr;
static int out_col,columns=55,visible=15,scrollback;
static int tty_reading,tty_ready,tty_count,tty_offset;
static char tty_buffer[LINE_CAP];
static int done,busy,cancelled,interrupted,background,dirty=1,force_draw=1;
/* Allocated only for an explicitly confirmed update handoff. */
static char *update_handoff;
static char input[LINE_CAP],killbuf[LINE_CAP],saved_input[LINE_CAP];
static char continued[LINE_CAP];
static int continuing,input_blocked;
static int cursor,history_back,searching,search_back;
static char search[128];
static unsigned long start_tick,last_yield,last_paint;
static unsigned long output_paint_ticks=3;
#ifdef CLARICE_DRAW_DIAGNOSTICS
static unsigned long listing_paint_interval=96,listing_partial_frames;
static int output_legacy;
#else
#define listing_paint_interval 96
#endif
void app_output_batch(int bulk){output_paint_ticks=bulk?listing_paint_interval:3;}
static int select_start=-1,select_end=-1;
static short test_ref=-1;
static long initial_free;
static SysEnvRec system_info;
static short font_id=4,font_size=9,cell_w=6,line_h=12,font_ascent=9;
static int mono=1,inverse,page_mode,paging,page_action;
static int control_mode=1; /* Option substitution preserves existing Plus behaviour. */
#include "control-keys.h"
static int native_glyph_valid;
static long page_start;
static char prefs_path[PATH_CAP];
static Rect normal_bounds;
static int filled;
static void save_preferences(void);
static void font_layout(void);
static void scroll_action(ControlHandle control,short part);
static RgnHandle scroll_region, saved_clip;
static long drawn_first=-1;
static int drawn_prompt=-1;
static void draw(void);
static void selection_clear(void);
static void handle(EventRecord *ev);
#ifdef CLARICE_INPUT_DIAGNOSTICS
static void input_probe_tick(void);
static unsigned long probe_events,probe_age,probe_age_max;
static int probe_curses;
#endif
static void execute(void);
static void input_fail(const char *reason);
#include "activity.inc"
static int claim_pending_screen(void);
#include "nano-perf.inc"
#include "editor-screen.inc"
/* Prompt is made only from cached shell state, with room left for input. */
static int prompt_prefix(char *out){if(tty_reading){out[0]=0;return 0;}if(continuing){strcpy(out,"> ");return 2;}char full[132];shell_prompt(full,sizeof full);int n=strlen(full),limit=columns/2;if(limit<2)limit=2;if(n>limit){memcpy(out,full+n-limit,limit);out[0]='~';out[limit]=0;return limit;}strcpy(out,full);return n;}
/* Cursor is composed into the offscreen row, then copied with its text. */
static void hide_caret(void){dirty=1;}
#ifdef CLARICE_SELECTION_DIAGNOSTICS
static int scroll_probe_legacy;
#endif
/* The pixel-move rectangle excludes the last row; that row is composed below. */
static int scroll_area_has_caret(void){int limit=visible;
#ifdef CLARICE_SELECTION_DIAGNOSTICS
 if(scroll_probe_legacy)limit=40;
#endif
 for(int r=0;r<limit;r++)if(last_screen[r][COLS-1])return 1;return 0;}

static void input_position(int lead,int end,int *row,int *col){
 int r=lead/columns,c=lead%columns;for(int i=0;i<end;i++){if(input[i]=='\n'){r++;c=0;}else if(++c==columns){r++;c=0;}}*row=r;*col=c;
}
/* The editable line extends the current output row. Its wrapped rows belong
   to the same view coordinates as history, rather than a fixed bottom slot. */
static long viewport_live_first(void){
 int rows=1;
 if((!busy||tty_reading)&&!searching){char prefix[132];int lead=(int)strlen(text[current_line%ROWS])+prompt_prefix(prefix),r,c;
  input_position(lead,(int)strlen(input),&r,&c);rows=r+1;
  if(rows>visible+1){input_position(lead,cursor,&r,&c);rows=r+1;if(rows<visible+1)rows=visible+1;}
 }
 long first=current_line+rows-1-visible;return first<oldest_line?oldest_line:first;
}
static void compose_input(char screen[40][COLS],long *first,int *prompt){
 char prefix[132];int base=(int)strlen(text[current_line%ROWS]),lead=prompt_prefix(prefix);
 int editing=(!busy||tty_reading)&&!searching;
 *first=viewport_live_first()-scrollback;if(*first<oldest_line)*first=oldest_line;
 *prompt=(int)(current_line-*first);memset(screen,0,40*COLS);
 for(int row=0;row<=visible;row++){long line=*first+row;if(line>=oldest_line&&line<=current_line)memcpy(screen[row],text[line%ROWS],columns);}
 if(searching){if(*prompt>=0&&*prompt<=visible)snprintf(screen[*prompt],COLS,"search: %.100s",search);}
 else if(editing){int total=base+lead+(int)strlen(input),r=0,c=0;
  for(int i=0;i<total;i++){unsigned char ch=i<base?text[current_line%ROWS][i]:i<base+lead?prefix[i-base]:input[i-base-lead];
   int row=*prompt+r;if(ch=='\n'){r++;c=0;continue;}
   if(row>=0&&row<=visible)screen[row][c]=ch;if(++c==columns){r++;c=0;}}
  if(!scrollback&&!background&&FrontWindow()==win){int cr,cc;input_position(base+lead,cursor,&cr,&cc);int row=*prompt+cr;if(row>=0&&row<=visible)screen[row][COLS-1]=cc+1;}
 }
 /* Explicit more owns its status row; ordinary output never enters this mode. */
 if(paging)snprintf(screen[visible],COLS,"-- More -- Space: next; Return: line; Q: stop");
}
static void pstr(unsigned char *p,const char *s){size_t n=strlen(s);if(n>255)n=255;p[0]=(unsigned char)n;memcpy(p+1,s,n);}
static void clear_terminal_row(char *row){
#ifdef CLARICE_DRAW_DIAGNOSTICS
 if(output_legacy){memset(row,0,COLS);return;}
#endif
 asm_clear_terminal_row(row);
}
static long copy_printable(char *dst,const char *s,long n){
#ifdef CLARICE_DRAW_DIAGNOSTICS
 if(output_legacy){const unsigned char *start=(const unsigned char*)s,*next=start,*end=start+n;while(next<end&&*next>=32)*dst++=*next++;return next-start;}
#endif
 return asm_copy_printable(dst,s,n);
}
static void newline(void){current_line++;if(current_line-retained+1>oldest_line)oldest_line=current_line-retained+1;out_col=0;clear_terminal_row(text[current_line%ROWS]);wrapped[current_line%ROWS]=0;if(scrollback){scrollback++;if(scrollback>scroll_limit())scrollback=scroll_limit();}}
static void soft_newline(void){wrapped[current_line%ROWS]=1;newline();}
static int serial_input; /* mode 2: remote tty byte/control semantics */
/* Copy printable runs with one bounded operation per terminal row. */
void term_write(const char *s,long n){if(n>0){activity_hide();activity_output_tick=TickCount();}long i=0;if(test_ref>=0){long k=n;FSWrite(test_ref,&k,s);}while(i<n){unsigned char c=s[i++];if(serial_input==2&&c=='\r'){out_col=0;last_cr=0;}else if(serial_input==2&&c==8){if(out_col)out_col--;}else if(c=='\n'){if(!last_cr)newline();last_cr=0;}else if(c=='\r'){newline();last_cr=1;}else if(c=='\t'){int k=4-(out_col%4);while(k--){if(out_col>=columns)soft_newline();text[current_line%ROWS][out_col++]=' ';}}else if(c>=32){last_cr=0;if(out_col>=columns)soft_newline();i--;long room=columns-out_col;if(room>n-i)room=n-i;long k=copy_printable(text[current_line%ROWS]+out_col,s+i,room);out_col+=k;i+=k;}}dirty=1;}
void term_puts(const char *s){term_write(s,(long)asm_length(s));}
/* One bounded local progress row. No input ownership, allocation or ANSI mode. */
void app_progress(const char *s,int finish){
 if(editor_screen)return;
 out_col=0;last_cr=0;memset(text[current_line%ROWS],0,COLS);wrapped[current_line%ROWS]=0;
 for(int i=0;s&&s[i]&&i<columns-1;i++){unsigned char c=s[i];text[current_line%ROWS][out_col++]=c>=32&&c<127?c:'?';}
 activity_hide();activity_output_tick=TickCount();if(finish)newline();dirty=1;term_present();
}

void app_clear(void){memset(text,0,sizeof text);memset(wrapped,0,sizeof wrapped);current_line=oldest_line=0;out_col=scrollback=0;dirty=force_draw=1;}
void app_quit(void){done=1;cancelled=1;}
int app_update_handoff(const char *native){
 if(update_handoff||!native||strlen(native)>255||!strchr(native,':')||native[0]==':')return paramErr;
 char *copy=(char*)NewPtr(strlen(native)+1);if(!copy)return memFullErr;
 strcpy(copy,native);update_handoff=copy;app_quit();return 0;
}
int app_columns(void){return columns;}
void app_metrics(int human){char b[256],free_size[32],launch_size[32];
 (void)human;display_size(free_size,(unsigned long)FreeMem());display_size(launch_size,(unsigned long)initial_free);
 snprintf(b,sizeof b,"CPU code %d; System API %04x\nFree app heap: %s; launch: %s\nLaunch-to-ready %lu ticks; commands %lu\nHistory: disk ring 128 entries; 1 KB retrieval\nKB = 1024 bytes\n",system_info.processor,(unsigned short)system_info.systemVersion,free_size,launch_size,start_tick,shell_commands);term_puts(b);
}
static void layout(void){activity_hide();Rect r=win->portRect;int previous_columns=columns,previous_visible=visible,was_scrolled=scrollback!=0;long anchor=viewport_live_first()-scrollback;if(anchor<oldest_line)anchor=oldest_line;
 /* Continuation rows of the editable command are virtual: reflow only owns
    committed history. Anchor these by input byte position, not a history index. */
 int input_anchor=-1;
 if(was_scrolled&&anchor>current_line&&(!busy||tty_reading)&&!searching){
  char prefix[132];int lead=(int)strlen(text[current_line%ROWS])+prompt_prefix(prefix),row=lead/columns,col=lead%columns;input_anchor=0;
  while(input[input_anchor]&&row<anchor-current_line){if(input[input_anchor++]=='\n'){row++;col=0;}else if(++col==columns){row++;col=0;}}
 }
 columns=(r.right-TEXT_LEFT-16)/cell_w;if(columns>COLS-2)columns=COLS-2;if(columns<1)columns=1;
 /* Use every complete row, from the content top through the bottom prompt. */
 visible=(r.bottom-TEXT_TOP)/line_h-1;if(visible<1)visible=1;if(visible>37)visible=37;
 if(previous_columns!=columns){TerminalReflow *next=(TerminalReflow*)NewPtr(sizeof *next);if(next){terminal_reflow(next,text,wrapped,oldest_line,current_line,anchor,columns,retained);memcpy(text,next->lines,sizeof text);memcpy(wrapped,next->wraps,sizeof wrapped);current_line=next->end;oldest_line=next->oldest;out_col=next->column;anchor=next->anchor;DisposePtr((Ptr)next);page_start=current_line;
   if(input_anchor>=0){char prefix[132];int row,col;input_position((int)strlen(text[current_line%ROWS])+prompt_prefix(prefix),input_anchor,&row,&col);anchor=current_line+row;}
  }else columns=previous_columns;}
 if(was_scrolled){scrollback=viewport_live_first()-anchor;if(scrollback<0)scrollback=0;}
 if(previous_columns!=columns||previous_visible!=visible)select_start=select_end=-1;
 force_draw=dirty=1;if(scrollback>scroll_limit())scrollback=scroll_limit();if(history_bar){MoveControl(history_bar,r.right-15,0);SizeControl(history_bar,16,r.bottom-14);}}

static void font_layout(void){native_glyph_valid=0;FontInfo f;int c;SetPort(win);TextFont(font_id);TextSize(font_size);TextFace(0);GetFontInfo(&f);font_ascent=f.ascent;line_h=f.ascent+f.descent+f.leading;if(line_h<1)line_h=12;cell_w=CharWidth('M');mono=1;for(c=32;c<127;c++){int w=CharWidth(c);if(w!=cell_w)mono=0;}if(!mono)cell_w=f.widMax;if(cell_w<1)cell_w=6;layout();build_font_cache();}
/* Repeated metadata rows can be cheaper to patch in place than to scroll and
   repaint exposed lines. Estimate changed spans, including per-row overhead. */
#ifdef CLARICE_DRAW_DIAGNOSTICS
static int catalog_force_scroll;
#endif
static int prefer_pixel_scroll(char screen[40][COLS],int shift){
#ifdef CLARICE_DRAW_DIAGNOSTICS
 if(catalog_force_scroll>0)return 1;
#endif
 /* Native per-row Toolbox work costs more than the cached glyph loop.
    Measured 4-row listing scroll: 57 vs 82 ticks over six frames. Keep the
    cheaper patch path for larger, nearly-identical metadata views. */
 int row,x,z,cost=0,row_cost=render_mode?8:24;
#ifdef CLARICE_DRAW_DIAGNOSTICS
 if(catalog_force_scroll<0)row_cost=8;
#endif
 int budget=(shift<0?-shift:shift)*(columns+row_cost)+32;
 if(shift==1||shift==-1)return 1;
 for(row=0;row<visible;row++){x=0;z=columns;while(x<z&&last_screen[row][x]==screen[row][x])x++;while(z>x&&last_screen[row][z-1]==screen[row][z-1])z--;if(z>x)cost+=z-x+row_cost;if(cost>budget)return 1;}
 return 0;
}
static void erase_margins(void){Rect content=win->portRect;if(history_bar)content.right=(*history_bar)->contrlRect.left;Rect margin=content;int bottom=TEXT_TOP+(visible+1)*line_h;
  margin.bottom=TEXT_TOP;EraseRect(&margin);margin=content;margin.top=TEXT_TOP;margin.right=TEXT_LEFT;EraseRect(&margin);
  margin=content;margin.top=TEXT_TOP;margin.left=TEXT_LEFT+columns*cell_w;EraseRect(&margin);
  margin=content;margin.top=bottom;EraseRect(&margin);
 }
/* The same reference-tested 68000 row copier used by the optional cache.
   Keep the clipped QuickDraw fallback for covered/misaligned windows. */
#include "native-align.inc"
#ifdef CLARICE_DRAW_DIAGNOSTICS
static int align_legacy;
static unsigned long align_ticks;
#define ALIGN_LEGACY align_legacy
#else
#define ALIGN_LEGACY 0
#endif
static void present_native_row(const Rect *original,const Rect *dest){
 Rect adjusted=*original;const Rect *source=&adjusted;
 int stride=win->portBits.rowBytes&0x3fff,px=dest->left-win->portBits.bounds.left,py=dest->top-win->portBits.bounds.top,width=dest->right-dest->left,height=dest->bottom-dest->top;
 /* Short changed spans already copy cheaply; shifting them regressed the
    matched directory workload. Reserve alignment for substantial row spans. */
 if(!ALIGN_LEGACY&&width>=128&&!(win->portBits.rowBytes&0xc000)&&stride==64&&width==source->right-source->left&&height==source->bottom-source->top){
#ifdef CLARICE_DRAW_DIAGNOSTICS
  unsigned long start=TickCount();
#endif
  align_native_row(&adjusted,px&15);
#ifdef CLARICE_DRAW_DIAGNOSTICS
  align_ticks+=TickCount()-start;
#endif
 }
 if(FrontWindow()==win&&!(win->portBits.rowBytes&0xc000)&&stride==64&&!(px&15)&&!(source->left&15)&&!(width&7)&&px>=0&&py>=0&&px+width<=512&&py+height<=342&&region_contains(win->visRgn,dest)&&region_contains(win->clipRgn,dest)){
  unsigned char *to=(unsigned char*)win->portBits.baseAddr+py*stride+px/8;HideCursor();asm_copy_rows(to,prompt_pixels+source->top*STRIDE+source->left/8,width/8,height,stride,STRIDE);ShowCursor();
 }else CopyBits(&prompt_port.portBits,&win->portBits,source,dest,srcCopy,0);
}
static unsigned long direct_copies,qd_copies,profile[4];
#ifdef CLARICE_DRAW_DIAGNOSTICS
static unsigned long draw_bar_ticks,draw_frames,frame_detail[2],jump_detail[4];
static int draw_probe_legacy,frame_legacy;
#define FRAME_LEGACY frame_legacy
#define DRAW_LEGACY draw_probe_legacy
#define DRAW_STAMP(name) unsigned long name=TickCount()
#define DRAW_ADD(index,name) (profile[index]+=TickCount()-(name))
#else
#define DRAW_LEGACY 0
#define FRAME_LEGACY 0
#define DRAW_STAMP(name) ((void)0)
#define DRAW_ADD(index,name) ((void)0)
#endif
/* Reuse the already displayed pixels when the entire moved area is visible.
   CopyBits handles overlapping vertical copies without blanking exposed rows. */
static int scroll_window_rows(int shift){
 Rect area,source,dest;SetRect(&area,TEXT_LEFT,TEXT_TOP,TEXT_LEFT+columns*cell_w,TEXT_TOP+visible*line_h);
 if(!region_contains(win->visRgn,&area)||!region_contains(win->clipRgn,&area))return 0;
 source=dest=area;if(shift>0){source.top+=shift*line_h;dest.bottom-=shift*line_h;}else{source.bottom+=shift*line_h;dest.top-=shift*line_h;}
 SetPort(win);ForeColor(blackColor);BackColor(whiteColor);DRAW_STAMP(move_start);CopyBits(&win->portBits,&win->portBits,&source,&dest,srcCopy,0);
#ifdef CLARICE_DRAW_DIAGNOSTICS
 frame_detail[0]+=TickCount()-move_start;
#endif
 return 1;
}
#include "glyph-planes.inc"
static int planes_enabled=2;
#include "native-glyphs.inc"
static unsigned long scroll_exposed_ticks,scroll_exposed_worst;
static void draw_native(void){unsigned long exposed_start=0;int row,x;char screen[40][COLS] __attribute__((aligned(2)));long first=current_line-visible+1-scrollback;int prompt_row=visible;if(first<0){first=0;prompt_row=(int)current_line;}int selection_a=select_start<select_end?select_start:select_end,selection_b=select_start>select_end?select_start:select_end;
 if(!win)return;SetPort(win);ForeColor(inverse?whiteColor:blackColor);BackColor(inverse?blackColor:whiteColor);DRAW_STAMP(native_stage);compose_input(screen,&first,&prompt_row);DRAW_ADD(0,native_stage);
#ifdef CLARICE_DRAW_DIAGNOSTICS
 native_stage=TickCount();
#endif
 /* Copy overlapping pixels without ScrollRect's exposed-area erase. Rows are
    composed offscreen below, so a scroll never blanks text before replacing it.
    Obscured/partially clipped views redraw from the bounded text model. */
 if(!force_draw&&select_start<0&&!scroll_area_has_caret()&&prompt_row>=visible&&drawn_prompt>=visible&&first!=drawn_first&&labs(first-drawn_first)<visible&&prefer_pixel_scroll(screen,(int)(first-drawn_first))){
  int shift=(int)(first-drawn_first);
  if(scroll_window_rows(shift)){
   exposed_start=TickCount();
   ForeColor(inverse?whiteColor:blackColor);BackColor(inverse?blackColor:whiteColor);
   /* System 6 BlockMove handles overlap and uses word/long transfers at
      either even alignment; libc memmove falls back to bytes at 2 mod 4. */
   DRAW_STAMP(model_start);
   if(shift>0){if(FRAME_LEGACY)memmove(last_screen,last_screen+shift,(visible-shift)*COLS);else BlockMove(last_screen+shift,last_screen,(visible-shift)*COLS);memset(last_screen+visible-shift,255,shift*COLS);}else{shift=-shift;if(FRAME_LEGACY)memmove(last_screen+shift,last_screen,(visible-shift)*COLS);else BlockMove(last_screen,last_screen+shift,(visible-shift)*COLS);memset(last_screen,255,shift*COLS);}
#ifdef CLARICE_DRAW_DIAGNOSTICS
   frame_detail[1]+=TickCount()-model_start;
#endif
  }
 }
 DRAW_ADD(1,native_stage);
#ifdef CLARICE_DRAW_DIAGNOSTICS
 native_stage=TickCount();
#endif
 if(force_draw)erase_margins();
 /* Configure the reusable row port once per paint, not once per row. */
 if(line_h<=64){SetPort(&prompt_port);TextFont(font_id);TextSize(font_size);TextFace(0);TextMode(srcCopy);ForeColor(inverse?whiteColor:blackColor);BackColor(inverse?blackColor:whiteColor);SetPort(win);}
 for(row=0;row<visible+1;row++){DRAW_STAMP(plan_start);int selected=selection_a>=0&&selection_b>selection_a&&row<visible+1;if(force_draw||(FRAME_LEGACY?memcmp(last_screen[row],screen[row],COLS):asm_terminal_row_changed(last_screen[row],screen[row]))||selected){Rect area,destination;char rendered[COLS];int left=0,right=columns,buffered=line_h<=64;
 if(!force_draw&&!selected){while(left<right&&last_screen[row][left]==screen[row][left])left++;while(right>left&&last_screen[row][right-1]==screen[row][right-1])right--;
  /* Include both composed cursor cells without repainting the whole prompt. */
  int old=(unsigned char)last_screen[row][COLS-1]-1,now=(unsigned char)screen[row][COLS-1]-1;
  if(old>=0&&old<columns){if(old<left)left=old;if(old>=right)right=old+1;}if(now>=0&&now<columns){if(now<left)left=now;if(now>=right)right=now+1;}
 }
 if(buffered&&!((TEXT_LEFT-win->portBits.bounds.left)&15)){int group=1;while((group*cell_w)&15)group++;left=left/group*group;right=(right+group-1)/group*group;if(right>columns)right=columns;}

#ifdef CLARICE_DRAW_DIAGNOSTICS
 jump_detail[0]+=TickCount()-plan_start;
#endif
 DRAW_STAMP(prepare_start);
 SetRect(&area,TEXT_LEFT+left*cell_w,TEXT_TOP+row*line_h,TEXT_LEFT+right*cell_w,TEXT_TOP+(row+1)*line_h);destination=area;if(buffered){OffsetRect(&area,-TEXT_LEFT,-area.top);SetPort(&prompt_port);}for(x=left;x<right;x++)rendered[x]=screen[row][x]?screen[row][x]:' ';

#ifdef CLARICE_DRAW_DIAGNOSTICS
 jump_detail[1]+=TickCount()-prepare_start;
#endif
 DRAW_STAMP(text_start);
 int glyph_phase=0;if(!DIRECT_LEGACY&&!ALIGN_LEGACY&&buffered&&(right-left)*cell_w>=128&&!(win->portBits.rowBytes&0xc000)&&(win->portBits.rowBytes&0x3fff)==64){glyph_phase=(destination.left-win->portBits.bounds.left)&15;if(glyph_phase+(right-left)*cell_w>512)glyph_phase=0;}
 history_raster_line=first+row;
 if(!force_draw&&buffered&&row<prompt_row&&!selected&&!screen[row][COLS-1]&&(!last_screen[row][COLS-1]||(unsigned char)last_screen[row][COLS-1]==255)&&native_glyph_row(rendered+left,right-left,glyph_phase)){OffsetRect(&area,glyph_phase-area.left,0);}
 else {EraseRect(&area);if(mono){int len=right;while(len>left&&rendered[len-1]==' ')len--;MoveTo(area.left,area.top+font_ascent);if(len>left)DrawText(rendered,left,len-left);}else for(x=left;x<right;x++){MoveTo((buffered?0:TEXT_LEFT)+x*cell_w,area.top+font_ascent);DrawChar(rendered[x]);}}

#ifdef CLARICE_DRAW_DIAGNOSTICS
 jump_detail[2]+=TickCount()-text_start;jump_detail[3]++;
#endif
 if(selected)for(x=0;x<columns;x++)if(row*columns+x>=selection_a&&row*columns+x<selection_b){Rect a;SetRect(&a,(buffered?0:TEXT_LEFT)+x*cell_w,area.top,(buffered?0:TEXT_LEFT)+(x+1)*cell_w,area.bottom);InvertRect(&a);}
 if(screen[row][COLS-1]){int cx=(unsigned char)screen[row][COLS-1]-1;Rect caret;SetRect(&caret,(buffered?0:TEXT_LEFT)+cx*cell_w,area.top,(buffered?0:TEXT_LEFT)+(cx+1)*cell_w,area.bottom);InvertRect(&caret);}
 if(buffered){SetPort(win);ForeColor(blackColor);BackColor(whiteColor);DRAW_STAMP(copy_start);present_native_row(&area,&destination);DRAW_ADD(3,copy_start);ForeColor(inverse?whiteColor:blackColor);BackColor(inverse?blackColor:whiteColor);}memcpy(last_screen[row],screen[row],COLS);}}
 DRAW_ADD(2,native_stage);
 if(exposed_start){unsigned long dt=TickCount()-exposed_start;scroll_exposed_ticks+=dt;if(dt>scroll_exposed_worst)scroll_exposed_worst=dt;}
 if(force_draw&&saved_clip){ForeColor(blackColor);BackColor(whiteColor);Rect grow=win->portRect;grow.left=grow.right-15;grow.top=grow.bottom-15;GetClip(saved_clip);ClipRect(&grow);DrawGrowIcon(win);SetClip(saved_clip);}history_cache_end();glyph_planes_end();drawn_first=first;drawn_prompt=prompt_row;dirty=force_draw=0;last_paint=TickCount();
}
/* Cached system-font cells; all addresses remain inside the bounded bitmap. */
extern void asm_cells(unsigned char *,const unsigned char *,const unsigned char *,long,long,long);
extern void asm_copy_rows(unsigned char *,const unsigned char *,long,long,long,long);
static void cells_c(unsigned char *dst,const unsigned char *s,const unsigned char *glyph,long n,long h,long w){
 long c,y;for(c=0;c<n;c++){unsigned int bit=c*w,shift=bit&7;unsigned char *d=dst+(bit>>3);const unsigned char *g=glyph+((unsigned long)s[c]<<5);for(y=0;y<h;y++){d[y*STRIDE]|=g[y]>>shift;if(shift)d[y*STRIDE+1]|=g[y]<<(8-shift);}}
}
static int scroll_limit(void){long newest=viewport_live_first(),oldest=oldest_line;if(newest<0)newest=0;if(oldest<0)oldest=0;return newest>oldest?(int)(newest-oldest):0;}
/* Test-only readback: verify that composing text/control updates never
   changes the live bar before the single completed-control copy. */
static int bar_snapshot(unsigned short *saved,int compare){
 Rect r=(*history_bar)->contrlRect;BitMap *b=&win->portBits;int stride=b->rowBytes&0x3fff,fail=0;
 if(r.right>b->bounds.right)r.right=b->bounds.right;if(r.bottom>b->bounds.bottom)r.bottom=b->bounds.bottom;
 int px=r.left-b->bounds.left,shift=px&7,width=r.right-r.left;
 for(int y=r.top;y<r.bottom&&y-r.top<342;y++){unsigned char *p=(unsigned char*)b->baseAddr+(y-b->bounds.top)*stride+(px>>3);
 unsigned long bits=((unsigned long)p[0]<<16)|((unsigned long)p[1]<<8);if(shift)bits|=p[2];
 unsigned short value=(bits>>(8-shift))&(0xffffUL<<(16-width));int i=y-r.top;if(compare){if(saved[i]!=value)fail++;}else saved[i]=value;}
 return fail;
}
/* Control Manager setters erase before redrawing. Compose the complete
   16-pixel control offscreen, then copy it once; never hide the live control. */
static void sync_history_bar(int force){
 if(!history_bar||!transfer_region)return;int limit=scroll_limit();if(scrollback>limit)scrollback=limit;int value=limit-scrollback;
 if(!force&&GetControlMaximum(history_bar)==limit&&GetControlValue(history_bar)==value)return;
 SetPort(win);ForeColor(blackColor);BackColor(whiteColor);Rect r=(*history_bar)->contrlRect;
 int height=r.bottom-r.top;Ptr pixels=(Ptr)bar_pixels;long bytes=sizeof bar_pixels;
 if(height>342){
  if(height!=tall_bar_rows){if(tall_bar_pixels)DisposePtr(tall_bar_pixels);tall_bar_pixels=0;tall_bar_rows=0;
   if(height>0&&height<=qd.screenBits.bounds.bottom-qd.screenBits.bounds.top){tall_bar_pixels=NewPtr((long)height*4);if(tall_bar_pixels){tall_bar_rows=height;force=1;}}}
  if(!tall_bar_pixels){SetControlMaximum(history_bar,limit);SetControlValue(history_bar,value);Draw1Control(history_bar);return;}
  pixels=tall_bar_pixels;bytes=(long)height*4;
 }else if(tall_bar_pixels){DisposePtr(tall_bar_pixels);tall_bar_pixels=0;tall_bar_rows=0;force=1;}
 BitMap screen=win->portBits,buffer;buffer.baseAddr=pixels;buffer.rowBytes=4;buffer.bounds=r;buffer.bounds.left&=~15;buffer.bounds.right=buffer.bounds.left+32;
 if(r.right-r.left!=16||height<=0)return;
 /* The staging image must be complete even during a clipped window update.
    Restore the window regions before copying to the real display. */
 RgnHandle clip=win->clipRgn,vis=win->visRgn;RectRgn(transfer_region,&r);
 unsigned short before[342];if(bar_testing)bar_snapshot(before,0);if(force||DRAW_LEGACY)memset(pixels,0,bytes);SetPortBits(&buffer);
 win->clipRgn=win->visRgn=transfer_region;
 if(GetControlMaximum(history_bar)!=limit)SetControlMaximum(history_bar,limit);
 if(GetControlValue(history_bar)!=value)SetControlValue(history_bar,value);
 /* SetControlValue/Maximum redraw changed parts. Preserve the staged pixels
    between updates instead of clearing and redrawing the control twice. */
 if(force||DRAW_LEGACY)Draw1Control(history_bar);
 if(bar_testing){unsigned short candidate[342];bar_snapshot(candidate,0);Draw1Control(history_bar);if(bar_snapshot(candidate,1))bar_failures++;}
 win->clipRgn=clip;win->visRgn=vis;SetPortBits(&screen);
 if(bar_testing){bar_checks++;if(bar_snapshot(before,1))bar_failures++;}
 CopyBits(&buffer,&screen,&r,&r,srcCopy,0);
}
static void scroll_action(ControlHandle control,short part){selection_clear();(void)control;int delta=part==inUpButton?1:part==inDownButton?-1:part==inPageUp?visible-1:part==inPageDown?1-visible:0;scrollback+=delta;if(scrollback<0)scrollback=0;if(scrollback>scroll_limit())scrollback=scroll_limit();select_start=select_end=-1;dirty=1;draw();}
/* Track the view with the thumb instead of drawing an outline and waiting
   for release. Fixed storage; only changed positions repaint, at most 30 Hz. */
static void drag_history(Point start){
 Rect bar=(*history_bar)->contrlRect;int limit=scroll_limit(),initial=GetControlValue(history_bar),travel=bar.bottom-bar.top-48;
#ifdef CLARICE_DRAW_DIAGNOSTICS
 unsigned long drag_start=TickCount(),drag_draws=0,drag_ticks=0,drag_worst=0,drag_wait=0,pending=0;long drag_heap=FreeMem();
#endif
 Point p=start;int value=initial;unsigned long painted=TickCount();
 if(travel<1||limit<1)return;
 selection_clear();
 do{GetMouse(&p);long next=initial+(long)(p.v-start.v)*limit/travel;if(next<0)next=0;if(next>limit)next=limit;
  if(next!=value){value=(int)next;scrollback=limit-value;dirty=1;
#ifdef CLARICE_DRAW_DIAGNOSTICS
 if(!pending)pending=TickCount();
#endif
 }
  if(dirty&&TickCount()-painted>=2){
#ifdef CLARICE_DRAW_DIAGNOSTICS
 unsigned long t=TickCount();
#endif
 draw();painted=TickCount();
#ifdef CLARICE_DRAW_DIAGNOSTICS
 unsigned long dt=painted-t;drag_draws++;drag_ticks+=dt;if(dt>drag_worst)drag_worst=dt;if(pending&&painted-pending>drag_wait)drag_wait=painted-pending;pending=0;
#endif
 }
  SystemTask();
 }while(StillDown());
 if(dirty)draw();
#ifdef CLARICE_DRAW_DIAGNOSTICS
 char report[256],full[PATH_CAP];unsigned char path[256];short ref;
 snprintf(report,sizeof report,"DRAG mode=%d duration=%lu draws=%lu paint=%lu worst=%lu detected-to-painted=%lu start=%d end=%d heap_delta=%ld\n",render_mode,TickCount()-drag_start,drag_draws,drag_ticks,drag_worst,drag_wait,initial,value,FreeMem()-drag_heap);
 shell_temp_path(full,"drag-profile.txt");pstr(path,full);HCreate(0,0,path,'CLRC','TEXT');if(!HOpen(0,0,path,fsWrPerm,&ref)){long n=strlen(report);SetEOF(ref,0);FSWrite(ref,&n,report);FSClose(ref);}
#endif
}
static void build_font_cache(void){
 glyph_ready=0;
 if(!render_mode){if(frame_pixels)DisposePtr((Ptr)frame_pixels);frame_pixels=0;glyph_cache=0;frame_port.portBits.baseAddr=0;return;}
 if(!frame_pixels){frame_pixels=(unsigned char*)NewPtr(STRIDE*HEIGHT+256*32);if(!frame_pixels){render_mode=0;return;}glyph_cache=frame_pixels+STRIDE*HEIGHT;frame_port.portBits.baseAddr=(Ptr)frame_pixels;}
 if(cell_w>8||line_h>32)return;
 GrafPort port;unsigned char bits[64];int c,y;OpenPort(&port);port.portBits.baseAddr=(Ptr)bits;port.portBits.rowBytes=2;SetRect(&port.portBits.bounds,0,0,16,32);SetRect(&port.portRect,0,0,cell_w,line_h);ClipRect(&port.portRect);TextFont(font_id);TextSize(font_size);TextFace(0);TextMode(srcCopy);ForeColor(blackColor);BackColor(whiteColor);
 memset(glyph_cache,0,256*32);for(c=32;c<256;c++){memset(bits,0,sizeof bits);MoveTo(0,font_ascent);DrawChar(c);for(y=0;y<line_h;y++)glyph_cache[c*32+y]=bits[y*2]&(255<<(8-cell_w));}ClosePort(&port);SetPort(win);glyph_ready=1;
}
static int region_contains(RgnHandle region,const Rect *r){if(!region)return 0;Rect b=(*region)->rgnBBox;if(b.left>r->left||b.top>r->top||b.right<r->right||b.bottom<r->bottom)return 0;if((*region)->rgnSize==10)return 1;if(!transfer_region||!visible_region)return 0;RectRgn(transfer_region,r);SectRgn(transfer_region,region,visible_region);return EqualRgn(transfer_region,visible_region);}
static void present_rows(int top,int bottom,int left,int right){
 Rect src,dst;int width=(right-left)*cell_w;SetRect(&src,left*cell_w,top*line_h,right*cell_w,bottom*line_h);dst=src;OffsetRect(&dst,TEXT_LEFT,TEXT_TOP);SetPort(win);
 int stride=win->portBits.rowBytes&0x3fff,px=dst.left-win->portBits.bounds.left,py=dst.top-win->portBits.bounds.top;
 
 unsigned char *to=(unsigned char*)win->portBits.baseAddr+py*stride+px/8;
 if(FrontWindow()==win&&!(win->portBits.rowBytes&0xc000)&&stride==64&&!(px&15)&&!((left*cell_w)&15)&&!(width&7)&&px>=0&&py>=0&&px+width<=512&&py+dst.bottom-dst.top<=342&&region_contains(win->visRgn,&dst)&&region_contains(win->clipRgn,&dst)){
  HideCursor();if(render_mode==2)asm_copy_rows(to,frame_pixels+src.top*STRIDE+src.left/8,width/8,src.bottom-src.top,stride,STRIDE);else{int y;for(y=0;y<src.bottom-src.top;y++)memcpy(to+y*stride,frame_pixels+(src.top+y)*STRIDE+src.left/8,width/8);}ShowCursor();direct_copies++;
 }else{CopyBits(&frame_port.portBits,&win->portBits,&src,&dst,srcCopy,0);qd_copies++;}
}
/* Move displayed history and cached pixels using the shared logical viewport. */
static void draw_cached(void){
 unsigned long stamp=TickCount();char screen[40][COLS];int row,x,lo=40,hi=0,xmin=columns,xmax=0,prompt=visible;long first=current_line-visible+1-scrollback;if(first<0){first=0;prompt=(int)current_line;}
 compose_input(screen,&first,&prompt);
 profile[0]+=TickCount()-stamp;stamp=TickCount();
 if(!force_draw&&select_start<0&&!scroll_area_has_caret()&&!((render_mode==1||DRAW_LEGACY)&&last_screen[visible][COLS-1])&&prompt>=visible&&drawn_prompt>=visible&&first!=drawn_first&&prefer_pixel_scroll(screen,(int)(first-drawn_first))){long d=first-drawn_first;int moved=!DRAW_LEGACY&&d&&labs(d)<visible?scroll_window_rows((int)d):0;if(d>0&&d<visible){if(render_mode==2){asm_copy_rows(frame_pixels,frame_pixels+d*line_h*STRIDE,STRIDE,(visible-d)*line_h,STRIDE,STRIDE);asm_copy_rows((unsigned char*)last_screen,(unsigned char*)(last_screen+d),COLS,visible-d,COLS,COLS);}else{memmove(frame_pixels,frame_pixels+d*line_h*STRIDE,(visible-d)*line_h*STRIDE);memmove(last_screen,last_screen+d,(visible-d)*COLS);}memset(last_screen+visible-d,255,d*COLS);if(!moved){lo=0;hi=visible;xmin=0;xmax=columns;}}else if(d<0&&-d<visible){d=-d;if(render_mode==2){asm_copy_rows(frame_pixels+(visible*line_h-1)*STRIDE,frame_pixels+((visible-d)*line_h-1)*STRIDE,STRIDE,(visible-d)*line_h,-STRIDE,-STRIDE);asm_copy_rows((unsigned char*)last_screen[visible-1],(unsigned char*)last_screen[visible-d-1],COLS,visible-d,-COLS,-COLS);}else{memmove(frame_pixels+d*line_h*STRIDE,frame_pixels,(visible-d)*line_h*STRIDE);memmove(last_screen+d,last_screen,(visible-d)*COLS);}memset(last_screen,255,d*COLS);if(!moved){lo=0;hi=visible;xmin=0;xmax=columns;}}}
 profile[1]+=TickCount()-stamp;stamp=TickCount();
 SetPort(&frame_port);TextFont(font_id);TextSize(font_size);TextFace(0);TextMode(srcCopy);ForeColor(blackColor);BackColor(whiteColor);
 for(row=0;row<visible+1;row++)if(force_draw||memcmp(screen[row],last_screen[row],COLS)||select_start>=0){
  int left=0,right=columns;unsigned char *pixels=frame_pixels+row*line_h*STRIDE;Rect r;
  if(glyph_ready&&!force_draw&&select_start<0&&row!=prompt&&!screen[row][COLS-1]&&!last_screen[row][COLS-1]){int group=1;while((group*cell_w)&7)group++;while(left<right&&screen[row][left]==last_screen[row][left])left++;while(right>left&&screen[row][right-1]==last_screen[row][right-1])right--;left=left/group*group;right=(right+group-1)/group*group;if(right>columns)right=columns;}
  SetRect(&r,left*cell_w,row*line_h,right*cell_w,(row+1)*line_h);
  if(glyph_ready){int y,n=right-left;unsigned char *dst=pixels+left*cell_w/8;for(y=0;y<line_h;y++)memset(dst+y*STRIDE,0,((right-left)*cell_w+7)/8);while(n&&(!screen[row][left+n-1]||screen[row][left+n-1]==' '))n--;if(render_mode==2)asm_cells(dst,(unsigned char*)screen[row]+left,glyph_cache,n,line_h,cell_w);else cells_c(dst,(unsigned char*)screen[row]+left,glyph_cache,n,line_h,cell_w);}
  else{memset(pixels,0,line_h*STRIDE);char rendered[COLS];for(x=0;x<columns;x++)rendered[x]=screen[row][x]?screen[row][x]:' ';if(mono){MoveTo(0,r.top+font_ascent);DrawText(rendered,0,columns);}else for(x=0;x<columns;x++){MoveTo(x*cell_w,r.top+font_ascent);DrawChar(rendered[x]);}}
  if(inverse)InvertRect(&r);
  int a=select_start<select_end?select_start:select_end,z=select_start>select_end?select_start:select_end;if(a>=0&&row<visible+1)for(x=0;x<columns;x++)if(row*columns+x>=a&&row*columns+x<z){Rect q;SetRect(&q,x*cell_w,r.top,(x+1)*cell_w,r.bottom);InvertRect(&q);}
  if(screen[row][COLS-1]){int cx=(unsigned char)screen[row][COLS-1]-1;Rect caret;SetRect(&caret,cx*cell_w,r.top,(cx+1)*cell_w,r.bottom);InvertRect(&caret);}
  if(left<xmin)xmin=left;if(right>xmax)xmax=right;if(row<lo)lo=row;if(row+1>hi)hi=row+1;memcpy(last_screen[row],screen[row],COLS);
 }
 profile[2]+=TickCount()-stamp;stamp=TickCount();
 SetPort(win);if(force_draw){ForeColor(inverse?whiteColor:blackColor);BackColor(inverse?blackColor:whiteColor);erase_margins();}
 /* CopyBits uses the destination port's colours even for a monochrome bitmap. */
 ForeColor(blackColor);BackColor(whiteColor);if(hi>lo)present_rows(lo,hi,xmin,xmax);
 if(force_draw){Rect grow=win->portRect;grow.left=grow.right-15;grow.top=grow.bottom-15;GetClip(saved_clip);ClipRect(&grow);DrawGrowIcon(win);SetClip(saved_clip);}
 profile[3]+=TickCount()-stamp;drawn_first=first;drawn_prompt=prompt;dirty=force_draw=0;last_paint=TickCount();
}
static unsigned long paint_ticks,event_ticks;
void app_profile(unsigned long *paint,unsigned long *events){*paint=paint_ticks;*events=event_ticks;}
#include "direct-view.inc"
static void draw(void){if(!win)return;if(editor_screen){native_glyph_valid=0;draw_editor();return;}activity_hide();unsigned long start=TickCount();int full=force_draw;hide_caret();unsigned short before[342];if(bar_testing)bar_snapshot(before,0);if(render_mode&&frame_pixels)draw_cached();else draw_native_direct();if(bar_testing){bar_checks++;if(bar_snapshot(before,1))bar_failures++;}DRAW_STAMP(bar_start);sync_history_bar(full);
#ifdef CLARICE_DRAW_DIAGNOSTICS
 draw_bar_ticks+=TickCount()-bar_start;draw_frames++;
#endif
 paint_ticks+=TickCount()-start;}

#ifdef CLARICE_DRAW_DIAGNOSTICS
static int native_varied_test;
#endif
static void native_test_line(char *line,int n){
#ifdef CLARICE_DRAW_DIAGNOSTICS
 if(native_varied_test){for(int x=0;x<columns;x++)line[x]=(char)(32+(n+x)%95);if(n%7==0)line[4]=(char)0xa9;line[columns]='\n';line[columns+1]=0;return;}
#endif
 snprintf(line,100,"----- 0000         35          0 TEXT TEST file%04d.txt\n",n);
}
static int native_scroll_check(int cached){
 Rect r;SetRect(&r,TEXT_LEFT,TEXT_TOP,TEXT_LEFT+columns*cell_w,TEXT_TOP+visible*line_h);int stride=win->portBits.rowBytes&0x3fff,px=r.left-win->portBits.bounds.left,py=r.top-win->portBits.bounds.top,width=((px&7)+r.right-r.left+7)/8,height=r.bottom-r.top;
 if(FrontWindow()!=win||columns!=80||cell_w!=6||stride!=64||px<0||py<0||px+480>512||py+height>342||!region_contains(win->visRgn,&r)){term_puts("Use an uncovered Monaco 9, 80-column window for this test.\n");return 2;}
 unsigned char *reference=(unsigned char*)NewPtr(STRIDE*HEIGHT),*pixels=(unsigned char*)win->portBits.baseAddr+py*stride+(px>>3);if(!reference)return 1;
 int saved=render_mode,failed=0,i,j,n=0;char line[100],report[260];bar_testing=1;bar_failures=bar_checks=0;render_mode=cached?2:0;build_font_cache();app_clear();
 for(i=0;i<128;i++){native_test_line(line,n++);term_puts(line);}draw();
 for(i=2;i<visible;i++){for(j=0;j<i;j++){native_test_line(line,n++);term_puts(line);}draw();HideCursor();for(j=0;j<height;j++)memcpy(reference+j*STRIDE,pixels+j*stride,width);ShowCursor();force_draw=dirty=1;draw();HideCursor();for(j=0;j<height;j++)if(memcmp(reference+j*STRIDE,pixels+j*stride,width)){failed++;break;}ShowCursor();app_yield();}
 /* Compare reverse/page/jump views with a full redraw, including retained
    boundaries. This catches overlap-direction and stale-row mistakes. */
 for(i=0;i<20;i++){scrollback=(i*17)%(scroll_limit()+1);dirty=1;draw();HideCursor();for(j=0;j<height;j++)memcpy(reference+j*STRIDE,pixels+j*stride,width);ShowCursor();force_draw=dirty=1;draw();HideCursor();for(j=0;j<height;j++)if(memcmp(reference+j*STRIDE,pixels+j*stride,width)){failed++;break;}ShowCursor();app_yield();}
 scrollback=0;bar_testing=0;failed+=bar_failures;
 DisposePtr((Ptr)reference);render_mode=saved;build_font_cache();dirty=force_draw=1;snprintf(report,sizeof report,"%s_SCROLL_TEST %s; %d bursts + 20 views; %d pixel mismatches\n",cached?"CACHED":"NATIVE",failed?"FAIL":"PASS",visible-2,failed);int used=strlen(report);snprintf(report+used,sizeof report-used,"Scrollbar staging: %d checks, %d staging/reference mismatches\n",bar_checks,bar_failures);
 {char full[PATH_CAP];unsigned char path[256];short ref;shell_temp_path(full,cached?"cached-scroll-report.txt":"native-scroll-report.txt");pstr(path,full);HCreate(0,0,path,'S6UX','TEXT');if(!HOpen(0,0,path,fsWrPerm,&ref)){long len=strlen(report);SetEOF(ref,0);FSWrite(ref,&len,report);FSClose(ref);FlushVol(0,0);}}
 term_puts(report);return failed?1:0;
}

#ifdef CLARICE_DRAW_DIAGNOSTICS
#include "../tests/terminal/draw-guest.inc"
#include "../tests/terminal/frame-guest.inc"
#include "../tests/terminal/jump-guest.inc"
#include "../tests/terminal/presentation-guest.inc"
#include "../tests/terminal/align-guest.inc"
#include "../tests/terminal/direct-guest.inc"
#include "../tests/terminal/live-guest.inc"
#include "../tests/terminal/arrows-guest.inc"
#include "../tests/terminal/admission-guest.inc"
#include "../tests/terminal/dispatch-guest.inc"
#include "../tests/terminal/catalog-guest.inc"
#include "../tests/terminal/listing-render-guest.inc"
#endif
/* Executed inside the 68000 guest, not host timing or inferred performance. */
int app_scroll_bench(int quick){
#ifdef CLARICE_DRAW_DIAGNOSTICS
 if(quick==20)return arrows_probe();
 if(quick==19)return live_output_probe();
 if(quick==18){admission_probe();return 0;}
 if(quick==17)return direct_view_pixels();
 if(quick==16)return alignment_pixels(1);
 if(quick==15)return jump_probe(3);
 if(quick==14)return alignment_pixels(0);
 if(quick==13)return jump_probe(2);
 if(quick==12)return presentation_probe();
 if(quick==11)return jump_probe(1);
 if(quick==10){native_varied_test=1;int result=native_scroll_check(0);native_varied_test=0;return result;}
 if(quick==9)return jump_probe(0);
 if(quick==8)return frame_probe();
 if(quick==7)return listing_render_probe();
 if(quick==6)return catalog_probe();
 if(quick==5)return dispatch_probe();
 if(quick==4)return draw_probe();
#endif
 if(quick>=2)return native_scroll_check(quick==3);
 int saved=render_mode,saved_retained=retained,failed=0,w,h,i,mode,run;unsigned char *a=(unsigned char*)NewPtr(4096),*b=(unsigned char*)NewPtr(4096);unsigned char chars[80];char report[2048],line[160];int len=0;unsigned long ticks[3][3]={{0}},worst[3][3]={{0}},timed_profile[4];int frames=quick?30:90,runs=quick?1:3;
 if(!a||!b){if(a)DisposePtr((Ptr)a);if(b)DisposePtr((Ptr)b);return 1;}
 render_mode=2;build_font_cache();if(!glyph_ready){DisposePtr((Ptr)a);DisposePtr((Ptr)b);render_mode=saved;build_font_cache();term_puts("Use Monaco 9 for scrollbench.\n");return 2;}
 /* All cell widths/heights, high-bit character indices, offsets and guard bytes. */
 for(i=0;i<80;i++)chars[i]=(unsigned char)(i*71+17);
 for(w=quick?6:1;w<=(quick?6:8);w++)for(h=quick?11:1;h<=(quick?11:32);h++){memset(a,0,4096);memset(b,0,4096);cells_c(a+16,chars,glyph_cache,60,h,w);asm_cells(b+16,chars,glyph_cache,60,h,w);if(memcmp(a,b,4096))failed++;}
 for(w=1;w<=64;w++){for(i=0;i<4096;i++)a[i]=(unsigned char)(i*13+7);memset(b,0,4096);asm_copy_rows(b+16,a+16,w,5,64,64);for(i=0;i<4096;i++){int row=(i-16)/64,col=(i-16)%64;unsigned char want=i>=16&&row<5&&col<w?a[i]:0;if(b[i]!=want){failed++;break;}}}
 DisposePtr((Ptr)a);DisposePtr((Ptr)b);
 /* Retention/wrap and anchored-view checks use the production ring operations. */
 for(w=32;w<=128;w*=2){retained=w;app_clear();for(i=0;i<300;i++)term_puts("ring test\n");if(oldest_line!=current_line-w+1||scroll_limit()!=w-visible-1)failed++;scrollback=5;long first=current_line-visible+1-scrollback;newline();if(current_line-visible+1-scrollback!=first)failed++;scrollback=scroll_limit();long oldest=oldest_line;newline();if(oldest_line!=oldest+1||scrollback!=scroll_limit())failed++;retained=128;if(oldest_line!=oldest+1)failed++;}
 render_mode=0;build_font_cache();long heap_off=FreeMem(),heap_on=0;for(i=0;i<4;i++){render_mode=2;build_font_cache();if(!frame_pixels)failed++;heap_on=FreeMem();render_mode=0;build_font_cache();if(FreeMem()!=heap_off)failed++;}render_mode=2;build_font_cache();
 retained=128;scrollback=0;select_start=select_end=-1;busy=1;cancelled=0;
 for(mode=0;mode<3&&!cancelled;mode++){render_mode=mode;force_draw=dirty=1;draw();if(mode==2)memset(profile,0,sizeof profile);for(run=0;run<runs&&!cancelled;run++){unsigned long start=TickCount();worst[mode][run]=0;for(i=0;i<frames&&!cancelled;i++){int x;for(x=0;x<columns;x++)line[x]=(char)('!'+(i*7+x*11)%90);line[columns]='\n';line[columns+1]=0;unsigned long t=TickCount();term_puts(line);term_present();unsigned long delta=TickCount()-t;if(delta>worst[mode][run])worst[mode][run]=delta;app_yield();}ticks[mode][run]=TickCount()-start;}}
 if(cancelled){render_mode=saved;retained=saved_retained;build_font_cache();dirty=force_draw=1;return 130;}
 memcpy(timed_profile,profile,sizeof profile);
 /* Incremental forward/backward views must equal a fresh full rendering. */
 render_mode=2;unsigned char *reference=(unsigned char*)NewPtr(STRIDE*HEIGHT);if(reference){for(i=0;i<20;i++){scrollback=(i*17)%(scroll_limit()+1);dirty=1;draw();memcpy(reference,frame_pixels,STRIDE*HEIGHT);force_draw=dirty=1;draw();if(memcmp(reference,frame_pixels,(visible+2)*line_h*STRIDE))failed++;}DisposePtr((Ptr)reference);}else failed++;
 len+=snprintf(report+len,sizeof report-len,"SCROLL_TEST %s (%d failures)\n%d frames per run; 60 ticks/sec; %d columns, %d rows\n",failed?"FAIL":"PASS",failed,frames,columns,visible);
 for(mode=0;mode<3;mode++)len+=snprintf(report+len,sizeof report-len,"%s ticks: %lu %lu %lu; max frame ticks: %lu %lu %lu\n",mode==0?"native":mode==1?"cached C":"cached ASM",ticks[mode][0],ticks[mode][1],ticks[mode][2],worst[mode][0],worst[mode][1],worst[mode][2]);
 len+=snprintf(report+len,sizeof report-len,"ASM phase ticks screen/shift/paint/present: %lu %lu %lu %lu\n",timed_profile[0],timed_profile[1],timed_profile[2],timed_profile[3]);
 len+=snprintf(report+len,sizeof report-len,"Cache allocation heap delta %ld; four release cycles stable\n",heap_off-heap_on);
 len+=snprintf(report+len,sizeof report-len,"Presentation totals: direct %lu, clipped QuickDraw %lu\nSystem %04x; processor %d; free heap %ld\n",direct_copies,qd_copies,(unsigned short)system_info.systemVersion,system_info.processor,FreeMem());
 {unsigned char path[256];char full[PATH_CAP];short ref;shell_temp_path(full,"scroll-report.txt");pstr(path,full);HCreate(0,0,path,'S6UX','TEXT');if(!HOpen(0,0,path,fsWrPerm,&ref)){long n=len;SetEOF(ref,0);FSWrite(ref,&n,report);FSClose(ref);FlushVol(0,0);}}
 render_mode=saved;retained=saved_retained;if(current_line-retained+1>oldest_line)oldest_line=current_line-retained+1;scrollback=0;build_font_cache();dirty=force_draw=1;term_puts(report);return failed?1:0;
}

/* Small versioned preference file; never write into the application resource fork. */
/* GetFNum also returns zero for Chicago: distinguish it from a missing font. */
static int installed_font(const unsigned char *name,short *id){
 unsigned char found[256];int i;GetFNum(name,id);if(*id)return 1;GetFontName(0,found);
 if(name[0]!=found[0]||!name[0])return 0;
 for(i=1;i<=name[0];i++){unsigned char a=name[i],b=found[i];if(a>='A'&&a<='Z')a+=32;if(b>='A'&&b<='Z')b+=32;if(a!=b)return 0;}return 1;
}
static int preference_number(const char *s){int n=0;if(!*s)return -1;for(;*s;s++){if(*s<'0'||*s>'9'||n>1000)return -1;n=n*10+*s-'0';}return n;}
#include "preferences-format.h"
static void fill_window(void);
static void save_preferences(void);
static void prefs_snapshot(struct ClaricePrefs *p){
 unsigned char font[256];Point origin={0,0};GetFontName(font_id,font);memcpy(p->font,font+1,font[0]);p->font[font[0]]=0;
 SetPort(win);LocalToGlobal(&origin);p->size=font_size;p->inverse=inverse;p->retained=retained;p->fill=filled;p->control=control_mode;
 p->width=filled?normal_bounds.right-normal_bounds.left:win->portRect.right;p->height=filled?normal_bounds.bottom-normal_bounds.top:win->portRect.bottom;
 p->x=filled?normal_bounds.left:origin.h;p->y=filled?normal_bounds.top:origin.v;if(p->x<0)p->x=0;if(p->y<0)p->y=0;
}
static void prefs_sibling(char *out,const char *suffix){strcpy(out,prefs_path);char *end=strrchr(out,':');strcpy(end?end+1:out,suffix);}
static void load_preferences(void){
 unsigned char name[256];char data[PREFS_TEXT_MAX+1];short ref;long n=PREFS_TEXT_MAX+1;struct ClaricePrefs p;int legacy=0;
 pstr(name,prefs_path);if(HOpen(0,0,name,fsRdPerm,&ref)){
  char backup[PATH_CAP];prefs_sibling(backup,"clarice.conf.bak");pstr(name,backup);if(HOpen(0,0,name,fsRdPerm,&ref))return;
 }
 OSErr e=FSRead(ref,&n,data);FSClose(ref);if((e&&e!=eofErr)||n>PREFS_TEXT_MAX)return;prefs_snapshot(&p);
 unsigned char *d=(unsigned char*)data;
 if((n==44&&!memcmp(d,"S6P2",4))||(n==48&&!memcmp(d,"S6P3",4))||(n==52&&!memcmp(d,"S6P4",4))){
  if(d[4]<6||d[4]>24||d[5]>1||d[8]>31)return;
  if(n>=48&&((d[7]!=32&&d[7]!=64&&d[7]!=128)||d[44]>2))return;
  memcpy(p.font,d+9,d[8]);p.font[d[8]]=0;p.size=d[4];p.inverse=d[5];if(n>=48){p.retained=d[7];}
  p.width=d[40]*256+d[41];p.height=d[42]*256+d[43];if(n==52){p.x=(short)((d[48]<<8)|d[49]);p.y=(short)((d[50]<<8)|d[51]);}legacy=1;
 }else if(!prefs_parse(&p,data,n))return;
 short id;pstr(name,p.font);if(installed_font(name,&id))font_id=id;font_size=p.size;inverse=p.inverse;retained=p.retained;control_mode=p.control;
 int maxw=qd.screenBits.bounds.right-8,maxh=qd.screenBits.bounds.bottom-44;
 if(p.width<180)p.width=180;if(p.width>maxw)p.width=maxw;if(p.height<100)p.height=100;if(p.height>maxh)p.height=maxh;
 SizeWindow(win,p.width,p.height,1);MoveWindow(win,p.x,p.y,1);
 {Rect frame=(*((WindowPeek)win)->strucRgn)->rgnBBox;Point origin={0,0};int dx=0,dy=0;SetPort(win);LocalToGlobal(&origin);
 if(frame.right>qd.screenBits.bounds.right-2)dx=qd.screenBits.bounds.right-2-frame.right;
 if(frame.left+dx<qd.screenBits.bounds.left+2)dx=qd.screenBits.bounds.left+2-frame.left;
 if(frame.bottom>qd.screenBits.bounds.bottom-2)dy=qd.screenBits.bounds.bottom-2-frame.bottom;
 if(frame.top+dy<qd.screenBits.bounds.top+menu_bar_height()+2)dy=qd.screenBits.bounds.top+menu_bar_height()+2-frame.top;
 if(dx||dy)MoveWindow(win,origin.h+dx,origin.v+dy,1);}
 if(p.fill)fill_window();if(legacy)save_preferences();
}
/* Write/flush a new file before replacing the old one. Keep the previous
   committed file as .bak, also used to recover an interrupted rename. */
static void save_preferences(void){
#ifdef CLARICE_OFFLINE_INSTALLER
 return; /* Installer geometry/theme must not rewrite user preferences. */
#endif
 struct ClaricePrefs p;char data[1024],tmp[PATH_CAP],backup[PATH_CAP];unsigned char name[256],tn[256],bn[256],base[256];short ref;int e;long n;
 prefs_snapshot(&p);n=prefs_format(data,sizeof data,&p);if(n<0||n>=(long)sizeof data)return;long want=n;
 prefs_sibling(tmp,"clarice.conf.new");prefs_sibling(backup,"clarice.conf.bak");pstr(name,prefs_path);pstr(tn,tmp);pstr(bn,backup);
 HDelete(0,0,tn);e=HCreate(0,0,tn,'S6UX','TEXT');if(!e)e=HOpen(0,0,tn,fsWrPerm,&ref);
 if(!e){e=FSWrite(ref,&n,data);if(!e&&n!=want)e=ioErr;int close=FSClose(ref);if(!e)e=close;}if(!e)e=shell_flush_native(prefs_path);
 if(!e){/* Only remove backup when the primary still exists. */
  short old;int exists=HOpen(0,0,name,fsRdPerm,&old);if(!exists){FSClose(old);e=HDelete(0,0,bn);if(e==fnfErr)e=0;pstr(base,"clarice.conf.bak");if(!e)e=shell_rename(name,base);}else if(exists!=fnfErr)e=exists;
  if(!e){const char *tail=strrchr(prefs_path,':');pstr(base,tail?tail+1:prefs_path);e=shell_rename(tn,base);if(e)shell_rename(bn,base);}
 }
 if(e){HDelete(0,0,tn);{char msg[110];snprintf(msg,sizeof msg,"Preferences save failed (%d); previous file retained.\n",e);term_puts(msg);}}shell_flush_native(prefs_path);
}
int app_pager_begin(void){int old=page_mode;page_mode=1;page_start=current_line;app_activity_interactive(1);return old;}
int app_pager_end(int old){int quit=cancelled&&!interrupted&&!done;if(quit)cancelled=0;page_mode=old;app_activity_interactive(0);return quit;}
void app_page_begin(void){page_start=current_line;}
int app_page(void){EventRecord ev;long limit=visible-1;if(!page_mode||test_ref>=0)return app_yield();if(limit<1)limit=1;if(current_line-page_start<limit)return app_yield();paging=1;page_action=0;dirty=1;draw();while(!page_action&&!cancelled&&!done){if(WaitNextEvent(everyEvent,&ev,6,0))handle(&ev);if(dirty)draw();}paging=0;if(page_action==2)page_start=current_line-limit+1;else page_start=current_line;dirty=1;return cancelled||done;}
static void fill_window(void){if(!filled){Point origin={0,0};SetPort(win);LocalToGlobal(&origin);normal_bounds=win->portRect;OffsetRect(&normal_bounds,origin.h,origin.v);MoveWindow(win,4,40,1);SizeWindow(win,qd.screenBits.bounds.right-8,qd.screenBits.bounds.bottom-44,1);filled=1;}else{MoveWindow(win,normal_bounds.left,normal_bounds.top,1);SizeWindow(win,normal_bounds.right-normal_bounds.left,normal_bounds.bottom-normal_bounds.top,1);filled=0;}layout();}
static void dialog_text(DialogPtr d,short item,const char *s){short type;Handle h;Rect r;unsigned char p[256];GetDialogItem(d,item,&type,&h,&r);pstr(p,s);SetDialogItemText(h,p);}
static void dialog_read(DialogPtr d,short item,char *s){short type;Handle h;Rect r;unsigned char p[256];GetDialogItem(d,item,&type,&h,&r);GetDialogItemText(h,p);memcpy(s,p+1,p[0]);s[p[0]]=0;}
static ControlHandle dialog_control(DialogPtr d,short item){short type;Handle h;Rect r;GetDialogItem(d,item,&type,&h,&r);return (ControlHandle)h;}
static MenuHandle pref_popup_menus[3];
static int pref_popup_choices[3];
static int popup_slot(int item){return item==5?0:item==12?1:2;}
static pascal void draw_pref_popup(DialogPtr d,short item){
 short type;Handle h;Rect r;GrafPtr old;unsigned char title[256];int k=popup_slot(item);if(!pref_popup_menus[k])return;
 GetPort(&old);SetPort(d);GetDialogItem(d,item,&type,&h,&r);EraseRect(&r);FrameRect(&r);GetMenuItemText(pref_popup_menus[k],pref_popup_choices[k],title);
 RgnHandle saved=NewRgn();if(saved){GetClip(saved);Rect text=r;text.left+=4;text.right-=22;ClipRect(&text);MoveTo(r.left+6,r.top+16);DrawString(title);SetClip(saved);DisposeRgn(saved);}for(int i=0;i<5;i++){MoveTo(r.right-17+i,r.top+9+i);LineTo(r.right-9-i,r.top+9+i);}SetPort(old);
}
static int dialog_popup(DialogPtr d,int item,MenuHandle m,int selected){short type;Handle h;Rect r;GrafPtr old;GetPort(&old);SetPort(d);GetDialogItem(d,item,&type,&h,&r);Point pt={r.top,r.left};LocalToGlobal(&pt);CalcMenuSize(m);HiliteMenu(0);long choice=PopUpMenuSelect(m,pt.v,pt.h,selected);SetPort(old);int result=(short)choice;return result?result:selected;}
static void popup_title(DialogPtr d,int item,MenuHandle m,int selected){short type;Handle h;Rect r;int k=popup_slot(item);pref_popup_menus[k]=m;pref_popup_choices[k]=selected;GetDialogItem(d,item,&type,&h,&r);SetDialogItem(d,item,userItem,(Handle)draw_pref_popup,&r);draw_pref_popup(d,item);}
static void preferences_window(void){
 hide_caret();DialogPtr d=GetNewDialog(140,0,(WindowPtr)-1);if(!d){SysBeep(1);return;}
 MenuHandle fonts=GetMenuHandle(132),lines=GetMenuHandle(134),controls=GetMenuHandle(136);
 unsigned char name[256],other[256];char b[256];short item=0;int chosen=1,cap=retained==32?1:retained==64?2:3,ctrl=control_mode+1;
 GetFontName(font_id,name);for(int i=1;i<=CountMItems(fonts);i++){GetMenuItemText(fonts,i,other);if(other[0]==name[0]&&!memcmp(other+1,name+1,name[0]))chosen=i;}
 popup_title(d,5,fonts,chosen);snprintf(b,sizeof b,"%d",font_size);dialog_text(d,7,b);snprintf(b,sizeof b,"%d",columns);dialog_text(d,9,b);
 SetControlValue(dialog_control(d,10),inverse);SetControlValue(dialog_control(d,13),filled);popup_title(d,12,lines,cap);popup_title(d,15,controls,ctrl);
 SelectDialogItemText(d,7,0,32767);
 while(item!=2){ModalDialog(0,&item);
  if(item==10||item==13){ControlHandle c=dialog_control(d,item);SetControlValue(c,!GetControlValue(c));}
  if(item==5){chosen=dialog_popup(d,5,fonts,chosen);popup_title(d,5,fonts,chosen);}
  if(item==12){cap=dialog_popup(d,12,lines,cap);popup_title(d,12,lines,cap);}
  if(item==15){ctrl=dialog_popup(d,15,controls,ctrl);popup_title(d,15,controls,ctrl);}
  if(item==1){char size[256],cols[256];short id;dialog_read(d,7,size);dialog_read(d,9,cols);GetMenuItemText(fonts,chosen,name);
   int n=preference_number(size),count=preference_number(cols);if(!installed_font(name,&id)||n<6||n>24||count<20||count>130){StopAlert(141,0);continue;}
   int want_fill=GetControlValue(dialog_control(d,13)),dark=GetControlValue(dialog_control(d,10));
   /* Preserve pixel geometry unless columns were explicitly edited; changing
      fonts is allowed even when the former column count no longer fits. */
   int change_columns=count!=columns;SetPort(win);TextFont(id);TextSize(n);FontInfo f;GetFontInfo(&f);int width=CharWidth('M'),fixed=1;
   for(int c=32;c<127;c++)if(CharWidth(c)!=width)fixed=0;if(!fixed)width=f.widMax;if(width<1)width=6;
   int pixels=count*width+TEXT_LEFT+16;TextFont(font_id);TextSize(font_size);
   if(change_columns&&(pixels<180||pixels>qd.screenBits.bounds.right-8)){SetPort(d);StopAlert(141,0);continue;}
   DisposeDialog(d);SetPort(win);if(filled&&(!want_fill||change_columns))fill_window();
   if(change_columns)SizeWindow(win,pixels,win->portRect.bottom,1);
   font_id=id;font_size=n;retained=cap==1?32:cap==2?64:128;inverse=dark;control_mode=ctrl-1;
   if(current_line-retained+1>oldest_line)oldest_line=current_line-retained+1;scrollback=0;
   if(want_fill&&!filled)fill_window();font_layout();
   /* Closing a dialog invalidates only its former rectangle. Font/theme changes
      affect the entire terminal, including text outside that update region. */
   InvalRect(&win->portRect);save_preferences();return;
  }
 }
 DisposeDialog(d);SetPort(win);dirty=force_draw=1;
}

int app_preferences(int argc,char **argv){char b[256];if(argc==1){unsigned char name[256];GetFontName(font_id,name);if(name[0]>31)name[0]=31;name[name[0]+1]=0;snprintf(b,sizeof b,"Font %.31s, %d point; %d columns x %d rows\nColours %s\n",name+1,font_size,columns,visible,inverse?"white on black":"black on white");term_puts(b);snprintf(b,sizeof b,"Scrollback %d lines; rendering automatic\nControl key: %s (physical Control always works)\n",retained,control_name(control_mode));term_puts(b);return 0;}

 if(!strcmp(argv[1],"window")){preferences_window();return 0;}
 if(!strcmp(argv[1],"fonts")){MenuHandle m=GetMenuHandle(132);short i;for(i=1;i<=CountMItems(m);i++){unsigned char name[256];GetMenuItemText(m,i,name);term_write((char*)name+1,name[0]);term_puts("\n");}return 0;}
 if(!strcmp(argv[1],"fill"))fill_window();
 else if(argc==3&&!strcmp(argv[1],"font")){unsigned char name[256];short id;pstr(name,argv[2]);if(!installed_font(name,&id)){term_puts("Font is not installed. Use prefs fonts.\n");return 2;}font_id=id;}
 else if(argc==3&&!strcmp(argv[1],"size")){int size=preference_number(argv[2]);if(size<6||size>24)return 2;font_size=size;}
 else if(argc==3&&!strcmp(argv[1],"colours")){if(!strcmp(argv[2],"dark"))inverse=1;else if(!strcmp(argv[2],"light"))inverse=0;else return 2;}
 else if(argc==3&&!strcmp(argv[1],"control")){int k=control_parse(argv[2]);if(k<0)return 2;control_mode=k;}
 else if(argc==3&&!strcmp(argv[1],"scrollback")){int n=preference_number(argv[2]);if(n!=32&&n!=64&&n!=128)return 2;retained=n;if(current_line-retained+1>oldest_line)oldest_line=current_line-retained+1;scrollback=0;}

#ifdef CLARICE_DRAW_DIAGNOSTICS
 else if(argc==3&&!strcmp(argv[1],"cache")){if(!strcmp(argv[2],"on"))render_mode=2;else if(!strcmp(argv[2],"off"))render_mode=0;else return 2;}
 else if(argc==3&&!strcmp(argv[1],"renderer")){if(!strcmp(argv[2],"native"))render_mode=0;else if(!strcmp(argv[2],"c"))render_mode=1;else if(!strcmp(argv[2],"asm"))render_mode=2;else return 2;}
#else
 else if(!strcmp(argv[1],"cache")||!strcmp(argv[1],"renderer")){term_puts("Rendering is selected automatically.\n");return 2;}
#endif
 else if(argc==3&&!strcmp(argv[1],"columns")){int n=preference_number(argv[2]),width=n*cell_w+TEXT_LEFT+16;if(n<20||n>130||width>qd.screenBits.bounds.right-8){term_puts("Requested columns do not fit this screen/font.\n");return 2;}if(filled)fill_window();SizeWindow(win,width,win->portRect.bottom,1);}
 else {term_puts("prefs [window | fonts | font name | size 6..24 | colours dark/light | control control/option/command | scrollback 32/64/128 | columns N | fill]\n");return 2;}
 font_layout();SetPort(win);InvalRect(&win->portRect);save_preferences();term_present();return 0;}
int app_yield(void){EventRecord ev;unsigned long now=TickCount();if(now-last_yield<2)return cancelled||done;unsigned long t=TickCount();for(int i=0;i<16&&!cancelled&&!done;i++){if(!WaitNextEvent(everyEvent,&ev,0,0))break;handle(&ev);}event_ticks+=TickCount()-t;if(dirty&&TickCount()-last_paint>=output_paint_ticks){
#ifdef CLARICE_DRAW_DIAGNOSTICS
if(output_paint_ticks==listing_paint_interval&&out_col)listing_partial_frames++;
#endif
 draw();}last_yield=TickCount();activity_tick(last_yield);return cancelled||done;}
/* Canonical terminal stdin: fixed 1 KiB queue, EOF on Ctrl-D, cancellable. */
long app_terminal_read(void *data,unsigned long count){
 if(!count)return 0;
 if(tty_offset>=tty_count){EventRecord ev;tty_count=tty_offset=tty_ready=0;input[0]=0;cursor=0;tty_reading=1;scrollback=0;activity_hide();dirty=1;
  while(!tty_ready&&!cancelled&&!done){if(dirty)draw();if(WaitNextEvent(everyEvent,&ev,6,0))handle(&ev);}
  tty_reading=0;dirty=1;if(cancelled||done){input[0]=0;cursor=0;return -1;}
 }
 unsigned long n=tty_count-tty_offset;if(n>count)n=count;memcpy(data,tty_buffer+tty_offset,n);tty_offset+=n;return n;
}
int cli_pause_ticks(unsigned long ticks){EventRecord ev;unsigned long start=TickCount();while(TickCount()-start<ticks&&!cancelled&&!done){if(dirty)draw();if(WaitNextEvent(everyEvent,&ev,6,0))handle(&ev);activity_tick(TickCount());}return cancelled||done;}
void term_present(void){if(dirty)draw();last_yield=TickCount();}
/* Fixed queue, replayed only by the outer event loop: no recursive execute. */
static unsigned char ahead_keys[LINE_CAP];
static unsigned short ahead_mods[LINE_CAP];
static unsigned int ahead_head,ahead_count;
#define PASTED_KEY 0x8000
static void copy_selection(void){char b[ROWS*COLS];long n=0;int a=select_start<select_end?select_start:select_end,z=select_start>select_end?select_start:select_end,i;if(a<0||z<=a){ZeroScrap();PutScrap(strlen(input),'TEXT',input);return;}for(i=a;i<z&&i<(visible+1)*columns;i++){char c=last_screen[i/columns][i%columns];b[n++]=c?c:' ';if(i%columns==columns-1)b[n++]='\r';}ZeroScrap();PutScrap(n,'TEXT',b);}
static void insert_char(unsigned char c){size_t n=strlen(input);if(input_blocked)return;if(n>=LINE_CAP-1){input_fail("command exceeds 1023 bytes");return;}memmove(input+cursor+1,input+cursor,n-cursor+1);input[cursor++]=(char)c;dirty=1;}
/* Validate the complete paste before changing the line. Newlines are retained
   (CR/CRLF normalized), never submitted as Return events. */
static void paste(void){
 if(input_blocked)return;if(editor_screen||serial_input||tty_reading||paging||activity_interactive){SysBeep(1);return;}Handle h=NewHandle(0);long off=0,n,used=0;size_t old=strlen(input);
 if(!h){input_fail("clipboard allocation failed");return;}n=GetScrap(h,'TEXT',&off);
 if(n<0){DisposeHandle(h);input_fail("clipboard text unavailable");return;}
 HLock(h);int bad=0;
 for(long i=0;i<n;i++){unsigned char c=(unsigned char)(*h)[i];if(c=='\r'){if(i+1<n&&(*h)[i+1]=='\n')i++;c='\n';}if((c<32&&c!='\n'&&c!='\t')||c==127){bad=1;break;}used++;}
 if(bad||used>(long)(busy?LINE_CAP-ahead_count:LINE_CAP-1-old)){HUnlock(h);DisposeHandle(h);input_fail(bad?"clipboard contains unsupported control bytes":"paste exceeds 1023-byte command limit");return;}
 used=0;for(long i=0;i<n;i++){unsigned char c=(unsigned char)(*h)[i];if(c=='\r'){if(i+1<n&&(*h)[i+1]=='\n')i++;c='\n';}(*h)[used++]=c;}
 if(busy){for(long i=0;i<used;i++){unsigned at=(ahead_head+ahead_count++)%LINE_CAP;ahead_keys[at]=(unsigned char)(*h)[i];ahead_mods[at]=PASTED_KEY;}HUnlock(h);DisposeHandle(h);return;}
 selection_clear();scrollback=0;memmove(input+cursor+used,input+cursor,old-cursor+1);memcpy(input+cursor,*h,used);cursor+=used;dirty=1;HUnlock(h);DisposeHandle(h);
}
static void history_move(int direction){char b[LINE_CAP];if(!history_back)strcpy(saved_input,input);if(direction<0){if(history_get(history_back+1,b)){history_back++;strcpy(input,b);}}else if(history_back>0){history_back--;if(!history_back)strcpy(input,saved_input);else if(history_get(history_back,b))strcpy(input,b);}cursor=strlen(input);dirty=1;}
static void do_search(void){char b[LINE_CAP];int k;busy=1;cancelled=0;k=history_search(search,search_back?search_back+1:1,b);busy=0;if(k){search_back=k;strcpy(input,b);cursor=strlen(input);}dirty=1;}

/* Any lost application-owned input poisons the pending submission. Return and
   queued suffixes cannot execute it; an explicit interrupt resets the latch. */
static void input_fail(const char *reason){
 if(input_blocked)return;input_blocked=1;input[0]=continued[0]=0;cursor=continuing=0;ahead_head=ahead_count=0;
 if(busy||tty_reading)cancelled=1;term_puts("\nInput rejected: ");term_puts(reason);term_puts(". Ctrl+C to resume.\n");shell_status=2;dirty=1;
}

/* Reuse the idle typeahead storage while serial owns keyboard input. */
static int serial_head,serial_count,serial_overflow;
int app_serial_input(int enabled){
 if(enabled&&!serial_input){serial_head=ahead_head;serial_count=ahead_count;serial_overflow=0;ahead_head=ahead_count=0;}
 else if(!enabled&&serial_input){if(serial_overflow)input_fail("serial keyboard queue overflow");serial_head=serial_count=serial_overflow=0;}
 serial_input=enabled;last_cr=0;return 0;
}
/* Separate import lets old runtimes reject remote-tty clients safely. */
int app_serial_tty(int enabled){return app_serial_input(enabled?2:0);}
int app_serial_key(void){if(serial_overflow)return -2;if(!serial_count)return -1;int c=ahead_keys[serial_head];serial_head=(serial_head+1)%LINE_CAP;serial_count--;return c;}
static void key(unsigned char c,unsigned short mods){if(mods==PASTED_KEY&&!busy){selection_clear();scrollback=0;insert_char(c);return;}int n=strlen(input);if(scrollback&&!((mods&shiftKey)&&(c==30||c==31))){selection_clear();scrollback=0;dirty=1;}if(paging){if((mods&shiftKey)&&(c==30||c==31)){scrollback+=c==30?visible:-visible;if(scrollback<0)scrollback=0;if(scrollback>scroll_limit())scrollback=scroll_limit();dirty=1;return;}if(c==' '){page_action=1;return;}if(c==13){page_action=2;return;}if(c=='q'||c=='Q'){cancelled=1;return;}}if((mods&ControlKey)&&c>=32&&c<127)c=control_ascii(c);if((mods&cmdKey)&&c=='.')c=3;if(input_blocked){if(c!=3)return;input_blocked=0;ahead_head=ahead_count=0;}if(tty_reading&&c!=3){
 if(c==13||c==4){if(c==4&&!n){tty_ready=1;return;}memcpy(tty_buffer,input,n);tty_count=n;if(c==13)tty_buffer[tty_count++]='\n';tty_offset=0;term_write(input,n);if(c==13)term_puts("\n");input[0]=0;cursor=0;tty_ready=1;dirty=1;return;}
 if(c==8||c==127){if(cursor){memmove(input+cursor-1,input+cursor,n-cursor+1);cursor--;}}
 else if(c==21){input[0]=0;cursor=0;}
 else if(c>=32&&!(mods&cmdKey))insert_char(c);
 dirty=1;return;
 }if(editor_screen){if((mods&cmdKey)&&c==3){interrupted=cancelled=1;}else screen_key(c,mods&ControlKey);return;}if(busy){if(c==3&&(serial_input!=2||(mods&cmdKey))){interrupted=1;cancelled=1;}else if(serial_input){if(!serial_overflow&&serial_count<LINE_CAP){ahead_keys[(serial_head+serial_count)%LINE_CAP]=c;serial_count++;}else serial_overflow=1;}else if(c==27||(c=='.'&&(mods&cmdKey)))cancelled=1;
 else if(!paging&&!activity_interactive){if(ahead_count<LINE_CAP){unsigned int at=(ahead_head+ahead_count)%LINE_CAP;ahead_keys[at]=c;ahead_mods[at]=mods&(shiftKey|ControlKey);ahead_count++;}else input_fail("pending input queue overflow");}return;}
 if(c==3){char prefix[132];shell_prompt(prefix,sizeof prefix);term_puts(prefix);term_puts(input);term_puts("^C\n");input[0]=saved_input[0]=continued[0]=0;continuing=0;cursor=history_back=searching=search_back=scrollback=0;shell_status=130;dirty=1;return;}
 if(mods&cmdKey){if(c=='q'||c=='Q'){app_quit();return;}if(c=='c'||c=='C'){copy_selection();return;}if(c=='v'||c=='V'){paste();return;}if(c=='.'){input[0]=0;cursor=0;searching=0;dirty=1;return;}}
 if(searching){if(c==13||c==27){searching=0;dirty=1;return;}if(c==18){do_search();return;}if(c==8||c==127){size_t l=strlen(search);if(l)search[l-1]=0;}else if(c>=32&&strlen(search)<sizeof(search)-1){size_t l=strlen(search);search[l]=c;search[l+1]=0;}search_back=0;do_search();return;}
 /* Raw controls 28..31 have no shell action; only arrow events move history. */
 if((mods&ControlKey)&&c>=28&&c<=31)return;
 /* Clear only the existing highlight; a click alone changes no pixels. */
 selection_clear();
 if(!((mods&shiftKey)&&(c==30||c==31)))scrollback=0;
 switch(c){case 13:execute();break;case 9:{int k;busy=1;cancelled=0;k=shell_complete(input,&cursor);busy=0;if(!k)SysBeep(1);break;}case 8:case 127:if(cursor){memmove(input+cursor-1,input+cursor,n-cursor+1);cursor--;}break;
 case 1:cursor=0;break;case 5:cursor=n;break;case 2:case 28:if(cursor)cursor--;break;case 6:case 29:if(cursor<n)cursor++;break;
 case 16:case 30:if(mods&shiftKey){scrollback+=visible;if(scrollback>scroll_limit())scrollback=scroll_limit();}else history_move(-1);break;
 case 14:case 31:if(mods&shiftKey){scrollback-=visible;if(scrollback<0)scrollback=0;}else history_move(1);break;
 case 4:if(cursor<n)memmove(input+cursor,input+cursor+1,n-cursor);break;
 case 11:strcpy(killbuf,input+cursor);input[cursor]=0;break;
 case 21:memcpy(killbuf,input,cursor);killbuf[cursor]=0;memmove(input,input+cursor,n-cursor+1);cursor=0;break;
 case 23:{int old=cursor;while(cursor&&input[cursor-1]==' ')cursor--;while(cursor&&input[cursor-1]!=' ')cursor--;memcpy(killbuf,input+cursor,old-cursor);killbuf[old-cursor]=0;memmove(input+cursor,input+old,n-old+1);break;}
 case 25:{int i;for(i=0;killbuf[i];i++)insert_char(killbuf[i]);break;}
 case 18:searching=1;search[0]=0;search_back=0;break;
 case 3:term_puts("^C\n");input[0]=0;cursor=0;shell_status=130;break;
 case 12:app_clear();break;
 default:if(c>=32)insert_char(c);break;
 }dirty=1;}
static int claim_pending_screen(void){
 unsigned required=0;
 for(unsigned i=0;i<ahead_count;i++){unsigned at=(ahead_head+i)%LINE_CAP;unsigned char c=ahead_keys[at];required+=(c>=28&&c<=31&&!(ahead_mods[at]&ControlKey))?3:1;}
 if(required>128){input_fail("application input queue overflow");return -1;}
 while(ahead_count){unsigned at=ahead_head;ahead_head=(ahead_head+1)%LINE_CAP;ahead_count--;screen_key(ahead_keys[at],ahead_mods[at]&ControlKey);}
 return 0;
}
/* An unquoted (or double-quoted) final backslash escapes the submitted newline. */
static int continues_line(const char *s){int quote=0;for(;*s;s++){if(*s=='\\'&&quote!=1){if(!s[1])return 1;s++;continue;}if(*s=='\''&&quote!=2)quote=quote==1?0:1;else if(*s=='"'&&quote!=1)quote=quote==2?0:2;}return 0;}
static void execute(void){char command[LINE_CAP],prefix[132];size_t used=strlen(continued),n=strlen(input);
 if(input_blocked)return;if(used+n>=sizeof command){input_fail("continued command exceeds 1023 bytes");return;}strcpy(command,continued);strcat(command,input);
 if(out_col)newline();prompt_prefix(prefix);term_puts(prefix);term_puts(input);term_puts("\n");input[0]=0;cursor=history_back=0;
 if(continues_line(command)){strcpy(continued,command);continued[strlen(continued)-1]=0;continuing=1;dirty=1;return;}continued[0]=0;continuing=0;
 busy=1;cancelled=interrupted=0;dirty=1;draw();activity_command=1;activity_interactive=0;activity_output_tick=TickCount();nano_submit(command);shell_run(command);activity_command=0;activity_interactive=0;activity_hide();busy=0;if(out_col)newline();if(interrupted){term_puts("^C\n");shell_status=130;}dirty=1;}
void app_run_checks(void){unsigned char pn[256];char path[PATH_CAP];int i,failed=0;static const char *cases[]={"selftest","pwd","ls -la","echo 'alpha beta' > Documents/result.txt","cat Documents/result.txt","grep -F beta Documents/result.txt","export TEST=ready","echo \"$TEST\" >> Documents/result.txt","cd Documents","pwd","cd ..","echo preserved > Documents/fork-test","cat \"Documents/with space.txt\"","status",0};
 shell_temp_path(path,"phase1-report.txt");pstr(pn,path);HCreate(0,0,pn,'S6UX','TEXT');if(!HOpen(0,0,pn,fsWrPerm,&test_ref))SetEOF(test_ref,0);busy=1;cancelled=0;
 for(i=0;cases[i]&&!cancelled;i++){term_puts("$ ");term_puts(cases[i]);term_puts("\n");{char result[96];unsigned long t=TickCount();int rc=shell_run(cases[i]);if(rc)failed++;snprintf(result,sizeof result,"[status %d; %lu ticks]\n",rc,TickCount()-t);term_puts(result);}app_yield();}
 {char recalled[LINE_CAP],result[128];long before=FreeMem(),after;for(i=0;i<140&&!cancelled;i++){if(shell_run("echo"))failed++;app_yield();}after=FreeMem();if(!history_get(128,recalled)||strcmp(recalled,"echo")||history_get(129,recalled))failed++;snprintf(result,sizeof result,"History wrap: 140 commands; heap before %ld after %ld\n",before,after);term_puts(result);}
 term_puts(failed?"PHASE1_CHECKS FAIL\n":"PHASE1_CHECKS PASS\n");if(test_ref>=0){FSClose(test_ref);test_ref=-1;FlushVol(0,0);}busy=0;dirty=1;}
static void welcome(void);
static void menu(long choice){short id=(short)(choice>>16),item=(short)choice;if(id==128){if(item==1){welcome();}else{MenuHandle m=GetMenuHandle(128);unsigned char name[256];GetMenuItemText(m,item,name);OpenDeskAcc(name);SetPort(win);}}
 if(id==129){if(item==1&&!editor_screen)app_clear();if(item==2)key(3,(editor_screen||serial_input==2)?cmdKey:0);if(item==4&&!busy)preferences_window();if(item==6)app_quit();}
 if(id==130){if(item==1)copy_selection();if(item==2)paste();if(item==4&&!busy)history_move(-1);if(item==5&&!busy)history_move(1);}
 if(id==131&&!busy){if(item==1)app_run_checks();if(item==2)app_metrics(0);}
 if(id==132){unsigned char name[256];GetMenuItemText(GetMenuHandle(132),item,name);GetFNum(name,&font_id);font_layout();SetPort(win);InvalRect(&win->portRect);save_preferences();}
 if(id==133&&!editor_screen){if(item==1)inverse=!inverse;if(item==2)fill_window();if(item==4)font_size=9;if(item==5)font_size=12;if(item==6)font_size=14;font_layout();SetPort(win);InvalRect(&win->portRect);save_preferences();}
 HiliteMenu(0);dirty=1;}
/* Moving a fixed selection anchor changes only the interval between the old
   and new endpoints. Invert those pixels once per row; keep an enabled cache
   in sync so later presentation cannot restore the previous highlight. */
static void selection_extend(int end){
 int a=select_end<end?select_end:end,z=select_end>end?select_end:end;
 if(a==z)return;HideCursor();
 while(a<z){int row=a/columns,right=(row+1)*columns;if(right>z)right=z;Rect r;
  SetRect(&r,TEXT_LEFT+(a%columns)*cell_w,TEXT_TOP+row*line_h,TEXT_LEFT+(right-row*columns)*cell_w,TEXT_TOP+(row+1)*line_h);
  SetPort(win);InvertRect(&r);
  if(render_mode&&frame_pixels){OffsetRect(&r,-TEXT_LEFT,-TEXT_TOP);SetPort(&frame_port);InvertRect(&r);}
  a=right;
 }
 SetPort(win);ShowCursor();select_end=end;
}
static void selection_clear(void){
 if(select_start>=0)selection_extend(select_start);
 select_start=select_end=-1;
}
static void selection_begin(int anchor){
 /* Commit pending content before applying a screen-relative highlight. */
 if(dirty||force_draw)draw();
 selection_clear();
 select_start=select_end=anchor;
}
static int cell(Point p){int x=(p.h-TEXT_LEFT)/cell_w,y=(p.v-TEXT_TOP)/line_h;if(x<0)x=0;if(x>=columns)x=columns-1;if(y<0)y=0;if(y>=visible+1)y=visible+1-1;return y*columns+x;}
static void handle(EventRecord *ev){if(ev->what==mouseDown||ev->what==activateEvt||ev->what==updateEvt)activity_hide();WindowPtr clicked;switch(ev->what){case mouseDown:{short part=FindWindow(ev->where,&clicked);if(part==inMenuBar)menu(MenuSelect(ev->where));else if(part==inSysWindow)SystemClick(ev,clicked);else if(clicked==win){SetPort(win);if(part==inDrag){Rect bounds=qd.screenBits.bounds;bounds.top+=20;DragWindow(win,ev->where,&bounds);save_preferences();}else if(part==inGoAway){if(TrackGoAway(win,ev->where))app_quit();}else if(part==inGrow&&!editor_screen){Rect bounds;long size;SetRect(&bounds,180,100,qd.screenBits.bounds.right-8,qd.screenBits.bounds.bottom-40);size=GrowWindow(win,ev->where,&bounds);if(size){filled=0;SizeWindow(win,(short)size,(short)(size>>16),1);layout();save_preferences();}}else if(part==inContent){if(FrontWindow()!=win)SelectWindow(win);else{Point p=ev->where;ControlHandle control;GlobalToLocal(&p);short hit=FindControl(p,win,&control);if(hit&&control==history_bar){if(hit==inThumb)drag_history(p);else{unsigned long repeat=TickCount()+12;scroll_action(control,hit);while(StillDown()){GetMouse(&p);if(TickCount()>=repeat){if(TestControl(control,p)==hit)scroll_action(control,hit);repeat=TickCount()+3;}SystemTask();}}}else if(paging&&p.v>=TEXT_TOP+visible*line_h&&p.v<TEXT_TOP+(visible+1)*line_h){page_action=1;}else if(!editor_screen&&p.v<TEXT_TOP+(visible+1)*line_h){selection_begin(cell(p));while(StillDown()){GetMouse(&p);selection_extend(cell(p));SystemTask();}GetMouse(&p);selection_extend(cell(p));}}}}break;}
 case keyDown:case autoKey:{
#ifdef CLARICE_INPUT_DIAGNOSTICS
 unsigned long age=TickCount()-ev->when;probe_events++;probe_age+=age;if(age>probe_age_max)probe_age_max=age;
#endif
 unsigned char c=(unsigned char)(ev->message&charCodeMask);/* On the Plus, shifted M0110A arrow packets translate to keypad / and =.
     Normalize their physical codes before character editing; main keyboard
     slash/equal use different codes. ADB machines retain their OS translation. */
 if(system_info.machineType==envMacPlus&&(ev->modifiers&shiftKey)){unsigned char code=(ev->message>>8)&255;if(code==0x4d)c=30;else if(code==0x48)c=31;}if((ev->modifiers&cmdKey)&&c=='.'){key(3,cmdKey);break;}
 unsigned short mods=ev->modifiers;int explicit_control=control_mode==2&&(mods&cmdKey)&&(mods&optionKey);
 if((mods&cmdKey)&&!explicit_control){long action=MenuKey(c);if(action){menu(action);break;}if(control_mode!=2)break;}

 if(control_requested(control_mode,mods)){
  /* Translate with the active keyboard layout, without Option's accented
     characters or dead-key state. No cached resource or permanent buffer. */
  short script=GetScriptManagerVariable(smKeyScript);/* Keep the 16-bit Pascal result in a frame slot: this Retro68 compiler
     otherwise reuses SP+2 after pushing the resource type. */
  volatile short id=GetScriptVariable(script,smScriptKeys);
  Handle map=GetResource('KCHR',id);
if(map){uint32_t state=0;unsigned code=(ev->message>>8)&127;unsigned char base=KeyTranslate(*map,code|(mods&shiftKey),&state);
if(base)c=base;}
  c=control_ascii(c);mods=(mods&~(cmdKey|optionKey))|ControlKey;
 }

 key(c,mods);break;}
 case updateEvt:if((WindowPtr)ev->message==win){hide_caret();SetPort(win);BeginUpdate(win);force_draw=dirty=1;draw();EndUpdate(win);}break;
 case activateEvt:if((WindowPtr)ev->message==win){background=!(ev->modifiers&activeFlag);if(editor_screen)editor_screen->dirty[editor_screen->row]=1;dirty=1;}break;
 case osEvt:if((ev->message>>24)==1){background=!(ev->message&1);dirty=force_draw=1;}break;
 default:break;}}
#ifdef CLARICE_INPUT_DIAGNOSTICS
#include "../tests/terminal/input-guest.inc"
#endif
#ifdef CLARICE_SELECTION_DIAGNOSTICS
#include "../tests/terminal/selection-guest.inc"
#endif
#ifdef CLARICE_ACTIVITY_DIAGNOSTICS
#include "../tests/terminal/activity-guest.inc"
#endif
#ifdef CLARICE_USABILITY_DIAGNOSTICS
#include "../tests/terminal/usability-guest.inc"

#endif
static void welcome(void){
 ClariceHardware h;char line[192],cpu[32],fpu[40],ram[32];cli_hardware(&h,sizeof h);
 static const char *cpus[]={"unknown","68000","68010","68020","68030","68040"};
 if(h.cpu<6)strcpy(cpu,cpus[h.cpu]);else snprintf(cpu,sizeof cpu,"unknown (code %u)",h.cpu);
 if(h.fpu==0)strcpy(fpu,"none");else if(h.fpu==1)strcpy(fpu,"68881");else if(h.fpu==2)strcpy(fpu,"68882");else if(h.fpu==3)strcpy(fpu,"68040");else if(h.fpu==-2)strcpy(fpu,"present, type unknown");else strcpy(fpu,"unknown");
 if(h.ram)display_size(ram,h.ram);else strcpy(ram,"unknown");
 snprintf(line,sizeof line,"Clarice - build %lu\n",(unsigned long)CLARICE_UPDATE_BUILD);term_puts(line);
 if(!h.model_name[0])snprintf(h.model_name,sizeof h.model_name,"Macintosh (model %ld)",(long)h.model);
 snprintf(line,sizeof line,"%s | CPU %s | FPU %s\n",h.model_name,cpu,fpu);term_puts(line);
 snprintf(line,sizeof line,"RAM %s | Display %u x %u, %u-bit\n",ram,h.width,h.height,h.depth);term_puts(line);
 if(h.system)snprintf(line,sizeof line,"System %x.%x.%x (reported)\n",h.system>>8,(h.system>>4)&15,h.system&15);else strcpy(line,"System version unknown\n");term_puts(line);term_puts("Type help for commands; Tab completes.\n\n");
}
int main(void){Rect r;MenuHandle m;EventRecord ev;unsigned long launch=TickCount();InitGraf(&qd.thePort);InitFonts();InitWindows();InitMenus();TEInit();InitDialogs(0);InitCursor();/* Reserve 64 KiB beyond the monochrome Mac default stack before growing the heap. */SetApplLimit(LMGetApplLimit()-65536);MaxApplZone();MoreMasters();SysEnvirons(1,&system_info);
 m=NewMenu(128,(const unsigned char*)"\p\024");AppendMenu(m,(const unsigned char*)"\pAbout Clarice;(-");AppendResMenu(m,'DRVR');InsertMenu(m,0);
 m=NewMenu(129,(const unsigned char*)"\pFile");AppendMenu(m,(const unsigned char*)"\pClear terminal;Cancel command/.;(-;Preferences.../,;(-;Quit/Q");InsertMenu(m,0);
 m=NewMenu(130,(const unsigned char*)"\pEdit");AppendMenu(m,(const unsigned char*)"\pCopy/C;Paste/V;(-;Previous command/H;Next command/J");InsertMenu(m,0);
 m=NewMenu(131,(const unsigned char*)"\pTests");AppendMenu(m,(const unsigned char*)"\pRun checks/T;Memory and system");InsertMenu(m,0);
 m=NewMenu(132,(const unsigned char*)"\pFont");AppendResMenu(m,'FONT');InsertMenu(m,-1);
 m=NewMenu(134,(const unsigned char*)"\pScrollback");AppendMenu(m,(const unsigned char*)"\p32 lines;64 lines;128 lines");InsertMenu(m,-1);
 m=NewMenu(136,(const unsigned char*)"\pControl key");AppendMenu(m,(const unsigned char*)"\pControl only;Option;Command");InsertMenu(m,-1);
 m=NewMenu(133,(const unsigned char*)"\pView");AppendMenu(m,(const unsigned char*)"\pReverse colours;Fill screen / restore;(-;9 point;12 point;14 point");InsertMenu(m,0);DrawMenuBar();
 SetRect(&r,8,48,512,326);win=NewWindow(&win_record,&r,(const unsigned char*)"\pClarice",1,documentProc,(WindowPtr)-1,1,0);if(!win)return 1;SetPort(win);
 {Rect frame=(*((WindowPeek)win)->strucRgn)->rgnBBox;MoveWindow(win,r.left+qd.screenBits.bounds.left+2-frame.left,r.top+qd.screenBits.bounds.top+menu_bar_height()+2-frame.top,1);}
 scroll_region=NewRgn();saved_clip=NewRgn();transfer_region=NewRgn();visible_region=NewRgn();
 OpenPort(&frame_port);frame_port.portBits.baseAddr=(Ptr)frame_pixels;frame_port.portBits.rowBytes=STRIDE;SetRect(&frame_port.portBits.bounds,0,0,512,HEIGHT);frame_port.portRect=frame_port.portBits.bounds;ClipRect(&frame_port.portRect);frame_ready=1;OpenPort(&prompt_port);prompt_port.portBits.baseAddr=(Ptr)prompt_pixels;prompt_port.portBits.rowBytes=STRIDE;SetRect(&prompt_port.portBits.bounds,0,0,512,64);prompt_port.portRect=prompt_port.portBits.bounds;ClipRect(&prompt_port.portRect);SetPort(win);
 SetRect(&r,win->portRect.right-15,0,win->portRect.right+1,win->portRect.bottom-14);history_bar=NewControl(win,&r,(const unsigned char*)"\p",1,0,0,0,scrollBarProc,0);
 shell_init();shell_state_path(prefs_path,"clarice.conf","Clarice Preferences");
#ifndef CLARICE_OFFLINE_INSTALLER
 load_preferences();
#endif
 font_layout();
 welcome();busy=1;
#ifndef CLARICE_OFFLINE_INSTALLER
 shell_profile();
#endif
 busy=0;dirty=1;initial_free=FreeMem();start_tick=TickCount()-launch;last_yield=TickCount();
#ifdef CLARICE_OFFLINE_INSTALLER
 {extern int shell_install_offline(char *);char target[256];
  SetWTitle(win,(const unsigned char*)"\pInstall Clarice");app_clear();term_puts("Clarice installation\n\n");draw();
  if(CautionAlert(152,0)==1){busy=1;cancelled=0;int e=shell_install_offline(target);busy=0;draw();
   if(!e)e=app_update_handoff(target);
   if(e){char detail[150];unsigned char message[151];
    if(e==130||e==userCanceledErr)strcpy(detail,"Installation cancelled. Completed utilities are kept. Run Install Clarice again to resume.");
    else if(e==fBsyErr)strcpy(detail,"Quit the installed Clarice, then run Install Clarice again. The running application has not been replaced.");
    else snprintf(detail,sizeof detail,"Installation stopped (%d). Completed files are kept. See Read Me for retry or recovery instructions.",e);
    pstr(message,detail);ParamText(message,0,0,0);StopAlert(151,0);app_quit();}
  }else app_quit();
 }
#endif
 while(!done){
#ifdef CLARICE_ACTIVITY_DIAGNOSTICS
 static int activity_tested;if(!activity_tested&&FrontWindow()==win){activity_tested=1;activity_probe();}
#endif
#ifdef CLARICE_USABILITY_DIAGNOSTICS
 static int usability_tested;if(!usability_tested&&FrontWindow()==win){usability_tested=1;usability_probe();}
#endif
#ifdef CLARICE_SELECTION_DIAGNOSTICS
 static int selection_tested; if(!selection_tested && FrontWindow()==win){selection_tested=1;selection_probe();}
#endif
#ifdef CLARICE_INPUT_DIAGNOSTICS
 input_probe_tick();
#endif
 int serviced=0;
 for(int i=0;i<32&&!done;i++){
  if(!busy&&ahead_count){unsigned int at=ahead_head;ahead_head=(ahead_head+1)%LINE_CAP;ahead_count--;key(ahead_keys[at],ahead_mods[at]);}
  else if(WaitNextEvent(everyEvent,&ev,0,0))handle(&ev);else break;
  serviced=1;
 }
 if(dirty)draw();if(serviced||ahead_count)continue;if(WaitNextEvent(everyEvent,&ev,6,0))handle(&ev);}
 UPDATE_LAUNCH_TRACE("cleanup begin");direct_stage_dispose();history_cache_dispose();glyph_planes_dispose();history_shutdown();if(tall_bar_pixels)DisposePtr(tall_bar_pixels);if(history_bar)DisposeControl(history_bar);if(frame_ready){ClosePort(&prompt_port);ClosePort(&frame_port);}if(frame_pixels)DisposePtr((Ptr)frame_pixels);if(transfer_region)DisposeRgn(transfer_region);if(visible_region)DisposeRgn(visible_region);if(scroll_region)DisposeRgn(scroll_region);if(saved_clip)DisposeRgn(saved_clip);CloseWindow(win);UPDATE_LAUNCH_TRACE("cleanup done");
 if(update_handoff){
  /* Invocation/stream/history cleanup has completed before transfer launch. */
  int e=update_native_launch(update_handoff);DisposePtr(update_handoff);update_handoff=0;
  if(e){unsigned char message[160];char detail[150];snprintf(detail,sizeof detail,"Update helper could not launch (Mac OS %d). Original application remains installed.",e);message[0]=strlen(detail);memcpy(message+1,detail,message[0]);ParamText(message,0,0,0);StopAlert(151,0);}
 }
 ExitToShell();return 0;}
