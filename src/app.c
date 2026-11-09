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

#define ROWS 128
#define COLS 132
#define STRIDE 64
#define HEIGHT 336
#define TEXT_TOP 0
#define TEXT_LEFT 4
extern unsigned long sys6_memtop(void);
static WindowPtr win;
static WindowRecord win_record;
static char text[ROWS][COLS];
static unsigned char wrapped[ROWS];
static char last_screen[40][COLS];

static unsigned char *frame_pixels,*glyph_cache;
static int retained=128;
static GrafPort frame_port,prompt_port;
static unsigned char prompt_pixels[STRIDE*64] __attribute__((aligned(2)));
static int frame_ready,glyph_ready,render_mode=0;
static RgnHandle transfer_region,visible_region;
static ControlHandle history_bar;
static void sync_history_bar(int force);
static unsigned char bar_pixels[4*342];
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
static char input[LINE_CAP],killbuf[LINE_CAP],saved_input[LINE_CAP];
static char continued[LINE_CAP];
static int continuing;
static int cursor,history_back,searching,search_back;
static char search[128];
static unsigned long start_tick,last_yield,last_paint;
static unsigned long output_paint_ticks=3;
void app_output_batch(int bulk){output_paint_ticks=bulk?32:3;}
static int select_start=-1,select_end=-1;
static short test_ref=-1;
static long initial_free;
static SysEnvRec system_info;
static short font_id=4,font_size=9,cell_w=6,line_h=12,font_ascent=9;
static int mono=1,inverse,page_mode=1,paging,page_action;
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
static void handle(EventRecord *ev);
static void execute(void);
#include "activity.inc"
/* Prompt is made only from cached shell state, with room left for input. */
static int prompt_prefix(char *out){if(tty_reading){out[0]=0;return 0;}if(continuing){strcpy(out,"> ");return 2;}char full[132];shell_prompt(full,sizeof full);int n=strlen(full),limit=columns/2;if(limit<2)limit=2;if(n>limit){memcpy(out,full+n-limit,limit);out[0]='~';out[limit]=0;return limit;}strcpy(out,full);return n;}
/* Cursor is composed into the offscreen row, then copied with its text. */
static void hide_caret(void){dirty=1;}
static int screen_has_caret(void){int r;for(r=0;r<40;r++)if(last_screen[r][COLS-1])return 1;return 0;}
static void compose_input(char screen[40][COLS],long *first,int *prompt){
 char prefix[132];int lead=prompt_prefix(prefix),rows=1,i,start=0;
 int editing=(!busy||tty_reading)&&!searching;
 if(editing){rows=(lead+strlen(input))/columns+1;
  if(rows>visible+1){start=(lead+cursor)/columns-visible;if(start<0)start=0;rows=visible+1;}}
 *first=current_line-(visible+1-rows)-scrollback;if(*first<0)*first=0;
 *prompt=(int)(current_line-*first);if(*prompt>visible+1-rows)*prompt=visible+1-rows;
 memset(screen,0,40*COLS);
 for(i=0;i<*prompt;i++){long line=*first+i;if(line>=oldest_line&&line<=current_line)memcpy(screen[i],text[line%ROWS],columns);}
 if(searching)snprintf(screen[*prompt],COLS,"search: %.100s",search);
 else if(editing){int total=lead+strlen(input);
  for(i=start*columns;i<total;i++){int row=*prompt+i/columns-start;if(row>visible)break;screen[row][i%columns]=i<lead?prefix[i]:input[i-lead];}
  if(!background&&FrontWindow()==win){int pos=lead+cursor,row=*prompt+pos/columns-start;if(row>=0&&row<=visible)screen[row][COLS-1]=pos%columns+1;}
 }else for(i=*prompt;i<=visible;i++){long line=*first+i;if(line>=oldest_line&&line<=current_line)memcpy(screen[i],text[line%ROWS],columns);}
 if(paging)snprintf(screen[visible],COLS,"-- More -- Space: next; Return: line; Q: stop");
}
static void pstr(unsigned char *p,const char *s){size_t n=strlen(s);if(n>255)n=255;p[0]=(unsigned char)n;memcpy(p+1,s,n);}
static void newline(void){current_line++;if(current_line-retained+1>oldest_line)oldest_line=current_line-retained+1;out_col=0;memset(text[current_line%ROWS],0,COLS);wrapped[current_line%ROWS]=0;if(scrollback){scrollback++;if(scrollback>scroll_limit())scrollback=scroll_limit();}}
static void soft_newline(void){wrapped[current_line%ROWS]=1;newline();}
/* Copy printable runs with one bounded operation per terminal row. */
void term_write(const char *s,long n){if(n>0){activity_hide();activity_output_tick=TickCount();}long i=0;if(test_ref>=0){long k=n;FSWrite(test_ref,&k,s);}while(i<n){unsigned char c=s[i++];if(c=='\n'){if(!last_cr)newline();last_cr=0;}else if(c=='\r'){newline();last_cr=1;}else if(c=='\t'){int k=4-(out_col%4);while(k--){if(out_col>=columns)soft_newline();text[current_line%ROWS][out_col++]=' ';}}else if(c>=32){last_cr=0;if(out_col>=columns)soft_newline();i--;long room=columns-out_col;if(room>n-i)room=n-i;long k=0;char *dst=text[current_line%ROWS]+out_col;while(k<room&&(unsigned char)s[i+k]>=32){dst[k]=s[i+k];k++;}out_col+=k;i+=k;}}dirty=1;}
void term_puts(const char *s){term_write(s,(long)asm_length(s));}
void app_clear(void){memset(text,0,sizeof text);memset(wrapped,0,sizeof wrapped);current_line=oldest_line=0;out_col=scrollback=0;dirty=force_draw=1;}
void app_quit(void){done=1;cancelled=1;}
int app_columns(void){return columns;}
void app_metrics(int human){char b[256],free_size[32],launch_size[32];
 if(human){format_size(free_size,(unsigned long)FreeMem(),1);format_size(launch_size,(unsigned long)initial_free,1);}
 else{snprintf(free_size,sizeof free_size,"%ld KB",FreeMem()/1024);snprintf(launch_size,sizeof launch_size,"%ld KB",initial_free/1024);}
 snprintf(b,sizeof b,"CPU code %d; System API %04x\nFree app heap: %s; launch: %s\nLaunch-to-ready %lu ticks; commands %lu\nHistory: disk ring 128 entries; 1 KB retrieval\nKB = 1024 bytes\n",system_info.processor,(unsigned short)system_info.systemVersion,free_size,launch_size,start_tick,shell_commands);term_puts(b);
}
static void layout(void){activity_hide();Rect r=win->portRect;int previous_columns=columns,previous_visible=visible;columns=(r.right-TEXT_LEFT-16)/cell_w;if(columns>COLS-2)columns=COLS-2;if(columns<1)columns=1;
 /* Use every complete row, from the content top through the bottom prompt. */
 visible=(r.bottom-TEXT_TOP)/line_h-1;if(visible<1)visible=1;if(visible>37)visible=37;
 if(previous_columns!=columns){TerminalReflow *next=(TerminalReflow*)NewPtr(sizeof *next);if(next){long anchor=current_line-previous_visible+1-scrollback;if(anchor<oldest_line)anchor=oldest_line;
 terminal_reflow(next,text,wrapped,oldest_line,current_line,anchor,columns,retained);memcpy(text,next->lines,sizeof text);memcpy(wrapped,next->wraps,sizeof wrapped);current_line=next->end;oldest_line=next->oldest;out_col=next->column;if(scrollback){scrollback=current_line-visible+1-next->anchor;if(scrollback<0)scrollback=0;}DisposePtr((Ptr)next);page_start=current_line;select_start=select_end=-1;}}
 force_draw=dirty=1;if(scrollback>scroll_limit())scrollback=scroll_limit();if(history_bar){MoveControl(history_bar,r.right-15,0);SizeControl(history_bar,16,r.bottom-14);}}

static void font_layout(void){FontInfo f;int c;SetPort(win);TextFont(font_id);TextSize(font_size);TextFace(0);GetFontInfo(&f);font_ascent=f.ascent;line_h=f.ascent+f.descent+f.leading;if(line_h<1)line_h=12;cell_w=CharWidth('M');mono=1;for(c=32;c<127;c++){int w=CharWidth(c);if(w!=cell_w)mono=0;}if(!mono)cell_w=f.widMax;if(cell_w<1)cell_w=6;layout();build_font_cache();}
/* Repeated metadata rows can be cheaper to patch in place than to scroll and
   repaint exposed lines. Estimate changed spans, including per-row overhead. */
static int prefer_pixel_scroll(char screen[40][COLS],int shift){
 int row,x,z,cost=0,budget=(shift<0?-shift:shift)*(columns+8)+32;
 if(shift==1||shift==-1)return 1;
 for(row=0;row<visible;row++){x=0;z=columns;while(x<z&&last_screen[row][x]==screen[row][x])x++;while(z>x&&last_screen[row][z-1]==screen[row][z-1])z--;if(z>x)cost+=z-x+8;if(cost>budget)return 1;}
 return 0;
}
static void erase_margins(void){Rect content=win->portRect;if(history_bar)content.right=(*history_bar)->contrlRect.left;Rect margin=content;int bottom=TEXT_TOP+(visible+1)*line_h;
  margin.bottom=TEXT_TOP;EraseRect(&margin);margin=content;margin.top=TEXT_TOP;margin.right=TEXT_LEFT;EraseRect(&margin);
  margin=content;margin.top=TEXT_TOP;margin.left=TEXT_LEFT+columns*cell_w;EraseRect(&margin);
  margin=content;margin.top=bottom;EraseRect(&margin);
 }
/* The same reference-tested 68000 row copier used by the optional cache.
   Keep the clipped QuickDraw fallback for covered/misaligned windows. */
static void present_native_row(const Rect *source,const Rect *dest){
 int stride=win->portBits.rowBytes&0x3fff,px=dest->left-win->portBits.bounds.left,py=dest->top-win->portBits.bounds.top,width=dest->right-dest->left,height=dest->bottom-dest->top;
 if(FrontWindow()==win&&!(win->portBits.rowBytes&0xc000)&&stride==64&&!(px&15)&&!(source->left&15)&&!(width&7)&&px>=0&&py>=0&&px+width<=512&&py+height<=342&&region_contains(win->visRgn,dest)&&region_contains(win->clipRgn,dest)){
  unsigned char *to=(unsigned char*)win->portBits.baseAddr+py*stride+px/8;HideCursor();asm_copy_rows(to,prompt_pixels+source->top*STRIDE+source->left/8,width/8,height,stride,STRIDE);ShowCursor();
 }else CopyBits(&prompt_port.portBits,&win->portBits,source,dest,srcCopy,0);
}
static void draw_native(void){int row,x;char screen[40][COLS];long first=current_line-visible+1-scrollback;int prompt_row=visible;if(first<0){first=0;prompt_row=(int)current_line;}int selection_a=select_start<select_end?select_start:select_end,selection_b=select_start>select_end?select_start:select_end;
 if(!win)return;SetPort(win);ForeColor(inverse?whiteColor:blackColor);BackColor(inverse?blackColor:whiteColor);compose_input(screen,&first,&prompt_row);
 /* Copy overlapping pixels without ScrollRect's exposed-area erase. Rows are
    composed offscreen below, so a scroll never blanks text before replacing it.
    Obscured/partially clipped views redraw from the bounded text model. */
 if(!force_draw&&select_start<0&&!screen_has_caret()&&prompt_row==visible&&drawn_prompt==visible&&first!=drawn_first&&labs(first-drawn_first)<visible&&prefer_pixel_scroll(screen,(int)(first-drawn_first))){
  int shift=(int)(first-drawn_first);Rect area,source,dest;
  SetRect(&area,TEXT_LEFT,TEXT_TOP,TEXT_LEFT+columns*cell_w,TEXT_TOP+visible*line_h);
  if(region_contains(win->visRgn,&area)&&region_contains(win->clipRgn,&area)){
   source=dest=area;if(shift>0){source.top+=shift*line_h;dest.bottom-=shift*line_h;}else{source.bottom+=shift*line_h;dest.top-=shift*line_h;}
   ForeColor(blackColor);BackColor(whiteColor);CopyBits(&win->portBits,&win->portBits,&source,&dest,srcCopy,0);ForeColor(inverse?whiteColor:blackColor);BackColor(inverse?blackColor:whiteColor);
   if(shift>0){memmove(last_screen,last_screen+shift,(visible-shift)*COLS);memset(last_screen+visible-shift,255,shift*COLS);}else{shift=-shift;memmove(last_screen+shift,last_screen,(visible-shift)*COLS);memset(last_screen,255,shift*COLS);}
  }
 }
 if(force_draw)erase_margins();
 /* Configure the reusable row port once per paint, not once per row. */
 if(line_h<=64){SetPort(&prompt_port);TextFont(font_id);TextSize(font_size);TextFace(0);TextMode(srcCopy);ForeColor(inverse?whiteColor:blackColor);BackColor(inverse?blackColor:whiteColor);SetPort(win);}
 for(row=0;row<visible+1;row++){int selected=selection_a>=0&&selection_b>selection_a&&row<visible;if(force_draw||memcmp(last_screen[row],screen[row],COLS)||selected){Rect area,destination;char rendered[COLS];int left=0,right=columns,buffered=line_h<=64;
 if(!force_draw&&!selected&&row!=prompt_row&&!screen[row][COLS-1]&&!last_screen[row][COLS-1]){while(left<right&&last_screen[row][left]==screen[row][left])left++;while(right>left&&last_screen[row][right-1]==screen[row][right-1])right--;}
 if(buffered&&!((TEXT_LEFT-win->portBits.bounds.left)&15)){int group=1;while((group*cell_w)&15)group++;left=left/group*group;right=(right+group-1)/group*group;if(right>columns)right=columns;}
 SetRect(&area,TEXT_LEFT+left*cell_w,TEXT_TOP+row*line_h,TEXT_LEFT+right*cell_w,TEXT_TOP+(row+1)*line_h);destination=area;if(buffered){OffsetRect(&area,-TEXT_LEFT,-area.top);SetPort(&prompt_port);}EraseRect(&area);for(x=left;x<right;x++)rendered[x]=screen[row][x]?screen[row][x]:' ';
 if(mono){int len=right;while(len>left&&rendered[len-1]==' ')len--;MoveTo(area.left,area.top+font_ascent);if(len>left)DrawText(rendered,left,len-left);}else for(x=left;x<right;x++){MoveTo((buffered?0:TEXT_LEFT)+x*cell_w,area.top+font_ascent);DrawChar(rendered[x]);}

 if(selected)for(x=0;x<columns;x++)if(row*columns+x>=selection_a&&row*columns+x<selection_b){Rect a;SetRect(&a,(buffered?0:TEXT_LEFT)+x*cell_w,area.top,(buffered?0:TEXT_LEFT)+(x+1)*cell_w,area.bottom);InvertRect(&a);}
 if(screen[row][COLS-1]){int cx=(unsigned char)screen[row][COLS-1]-1;Rect caret;SetRect(&caret,(buffered?0:TEXT_LEFT)+cx*cell_w,area.top,(buffered?0:TEXT_LEFT)+(cx+1)*cell_w,area.bottom);InvertRect(&caret);}
 if(buffered){SetPort(win);ForeColor(blackColor);BackColor(whiteColor);present_native_row(&area,&destination);ForeColor(inverse?whiteColor:blackColor);BackColor(inverse?blackColor:whiteColor);}memcpy(last_screen[row],screen[row],COLS);}}
 if(force_draw&&saved_clip){ForeColor(blackColor);BackColor(whiteColor);Rect grow=win->portRect;grow.left=grow.right-15;grow.top=grow.bottom-15;GetClip(saved_clip);ClipRect(&grow);DrawGrowIcon(win);SetClip(saved_clip);}drawn_first=first;drawn_prompt=prompt_row;dirty=force_draw=0;last_paint=TickCount();
}
/* Cached system-font cells; all addresses remain inside the bounded bitmap. */
extern void asm_cells(unsigned char *,const unsigned char *,const unsigned char *,long,long,long);
extern void asm_copy_rows(unsigned char *,const unsigned char *,long,long,long,long);
static void cells_c(unsigned char *dst,const unsigned char *s,const unsigned char *glyph,long n,long h,long w){
 long c,y;for(c=0;c<n;c++){unsigned int bit=c*w,shift=bit&7;unsigned char *d=dst+(bit>>3);const unsigned char *g=glyph+((unsigned long)s[c]<<5);for(y=0;y<h;y++){d[y*STRIDE]|=g[y]>>shift;if(shift)d[y*STRIDE+1]|=g[y]<<(8-shift);}}
}
static int scroll_limit(void){long newest=current_line-visible+1,oldest=oldest_line;if(newest<0)newest=0;if(oldest<0)oldest=0;return newest>oldest?(int)(newest-oldest):0;}
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
 if(!history_bar)return;int limit=scroll_limit();if(scrollback>limit)scrollback=limit;int value=limit-scrollback;
 if(!force&&GetControlMaximum(history_bar)==limit&&GetControlValue(history_bar)==value)return;
 SetPort(win);ForeColor(blackColor);BackColor(whiteColor);Rect r=(*history_bar)->contrlRect;
 BitMap screen=win->portBits,buffer;buffer.baseAddr=(Ptr)bar_pixels;buffer.rowBytes=4;buffer.bounds=r;buffer.bounds.left&=~15;buffer.bounds.right=buffer.bounds.left+32;
 if(r.right-r.left!=16||r.bottom-r.top>342)return;
 unsigned short before[342];if(bar_testing)bar_snapshot(before,0);memset(bar_pixels,0,sizeof bar_pixels);SetPortBits(&buffer);
 if(GetControlMaximum(history_bar)!=limit)SetControlMaximum(history_bar,limit);
 if(GetControlValue(history_bar)!=value)SetControlValue(history_bar,value);
 Draw1Control(history_bar);SetPortBits(&screen);
 if(bar_testing){bar_checks++;if(bar_snapshot(before,1))bar_failures++;}
 CopyBits(&buffer,&screen,&r,&r,srcCopy,0);
}
static void scroll_action(ControlHandle control,short part){(void)control;int delta=part==inUpButton?1:part==inDownButton?-1:part==inPageUp?visible-1:part==inPageDown?1-visible:0;scrollback+=delta;if(scrollback<0)scrollback=0;if(scrollback>scroll_limit())scrollback=scroll_limit();select_start=select_end=-1;dirty=1;draw();}
/* Track the view with the thumb instead of drawing an outline and waiting
   for release. Fixed storage; only changed positions repaint, at most 30 Hz. */
static void drag_history(Point start){
 Rect bar=(*history_bar)->contrlRect;int limit=scroll_limit(),initial=GetControlValue(history_bar),travel=bar.bottom-bar.top-48;
 Point p=start;int value=initial;unsigned long painted=TickCount();
 if(travel<1||limit<1)return;
 select_start=select_end=-1;
 do{GetMouse(&p);long next=initial+(long)(p.v-start.v)*limit/travel;if(next<0)next=0;if(next>limit)next=limit;
  if(next!=value){value=(int)next;scrollback=limit-value;dirty=1;}
  if(dirty&&TickCount()-painted>=2){draw();painted=TickCount();}
  SystemTask();
 }while(StillDown());
 if(dirty)draw();
}
static void build_font_cache(void){
 glyph_ready=0;
 if(!render_mode){if(frame_pixels)DisposePtr((Ptr)frame_pixels);frame_pixels=0;glyph_cache=0;frame_port.portBits.baseAddr=0;return;}
 if(!frame_pixels){frame_pixels=(unsigned char*)NewPtr(STRIDE*HEIGHT+256*32);if(!frame_pixels){render_mode=0;return;}glyph_cache=frame_pixels+STRIDE*HEIGHT;frame_port.portBits.baseAddr=(Ptr)frame_pixels;}
 if(cell_w>8||line_h>32)return;
 GrafPort port;unsigned char bits[64];int c,y;OpenPort(&port);port.portBits.baseAddr=(Ptr)bits;port.portBits.rowBytes=2;SetRect(&port.portBits.bounds,0,0,16,32);SetRect(&port.portRect,0,0,cell_w,line_h);ClipRect(&port.portRect);TextFont(font_id);TextSize(font_size);TextFace(0);TextMode(srcCopy);ForeColor(blackColor);BackColor(whiteColor);
 memset(glyph_cache,0,256*32);for(c=32;c<256;c++){memset(bits,0,sizeof bits);MoveTo(0,font_ascent);DrawChar(c);for(y=0;y<line_h;y++)glyph_cache[c*32+y]=bits[y*2]&(255<<(8-cell_w));}ClosePort(&port);SetPort(win);glyph_ready=1;
}
static unsigned long direct_copies,qd_copies,profile[4];
static int region_contains(RgnHandle region,const Rect *r){if(!region)return 0;Rect b=(*region)->rgnBBox;if(b.left>r->left||b.top>r->top||b.right<r->right||b.bottom<r->bottom)return 0;if((*region)->rgnSize==10)return 1;if(!transfer_region||!visible_region)return 0;RectRgn(transfer_region,r);SectRgn(transfer_region,region,visible_region);return EqualRgn(transfer_region,visible_region);}
static void present_rows(int top,int bottom,int left,int right){
 Rect src,dst;int width=(right-left)*cell_w;SetRect(&src,left*cell_w,top*line_h,right*cell_w,bottom*line_h);dst=src;OffsetRect(&dst,TEXT_LEFT,TEXT_TOP);SetPort(win);
 int stride=win->portBits.rowBytes&0x3fff,px=dst.left-win->portBits.bounds.left,py=dst.top-win->portBits.bounds.top;
 
 unsigned char *to=(unsigned char*)win->portBits.baseAddr+py*stride+px/8;
 if(FrontWindow()==win&&!(win->portBits.rowBytes&0xc000)&&stride==64&&!(px&15)&&!(width&7)&&px>=0&&py>=0&&px+width<=512&&py+dst.bottom-dst.top<=342&&region_contains(win->visRgn,&dst)&&region_contains(win->clipRgn,&dst)){
  HideCursor();if(render_mode==2)asm_copy_rows(to,frame_pixels+src.top*STRIDE+src.left/8,width/8,src.bottom-src.top,stride,STRIDE);else{int y;for(y=0;y<src.bottom-src.top;y++)memcpy(to+y*stride,frame_pixels+(src.top+y)*STRIDE+src.left/8,width/8);}ShowCursor();direct_copies++;
 }else{CopyBits(&frame_port.portBits,&win->portBits,&src,&dst,srcCopy,0);qd_copies++;}
}
static void draw_cached(void){
 unsigned long stamp=TickCount();char screen[40][COLS];int row,x,lo=40,hi=0,xmin=columns,xmax=0,prompt=visible;long first=current_line-visible+1-scrollback;if(first<0){first=0;prompt=(int)current_line;}
 compose_input(screen,&first,&prompt);
 profile[0]+=TickCount()-stamp;stamp=TickCount();
 if(!force_draw&&select_start<0&&!screen_has_caret()&&prompt==visible&&drawn_prompt==visible&&first!=drawn_first&&prefer_pixel_scroll(screen,(int)(first-drawn_first))){long d=first-drawn_first;if(d>0&&d<visible){if(render_mode==2){asm_copy_rows(frame_pixels,frame_pixels+d*line_h*STRIDE,STRIDE,(visible-d)*line_h,STRIDE,STRIDE);asm_copy_rows((unsigned char*)last_screen,(unsigned char*)(last_screen+d),COLS,visible-d,COLS,COLS);}else{memmove(frame_pixels,frame_pixels+d*line_h*STRIDE,(visible-d)*line_h*STRIDE);memmove(last_screen,last_screen+d,(visible-d)*COLS);}memset(last_screen+visible-d,255,d*COLS);lo=0;hi=visible;xmin=0;xmax=columns;}else if(d<0&&-d<visible){d=-d;if(render_mode==2){asm_copy_rows(frame_pixels+(visible*line_h-1)*STRIDE,frame_pixels+((visible-d)*line_h-1)*STRIDE,STRIDE,(visible-d)*line_h,-STRIDE,-STRIDE);asm_copy_rows((unsigned char*)last_screen[visible-1],(unsigned char*)last_screen[visible-d-1],COLS,visible-d,-COLS,-COLS);}else{memmove(frame_pixels+d*line_h*STRIDE,frame_pixels,(visible-d)*line_h*STRIDE);memmove(last_screen+d,last_screen,(visible-d)*COLS);}memset(last_screen,255,d*COLS);lo=0;hi=visible;xmin=0;xmax=columns;}}
 profile[1]+=TickCount()-stamp;stamp=TickCount();
 SetPort(&frame_port);TextFont(font_id);TextSize(font_size);TextFace(0);TextMode(srcCopy);ForeColor(blackColor);BackColor(whiteColor);
 for(row=0;row<visible+1;row++)if(force_draw||memcmp(screen[row],last_screen[row],COLS)||select_start>=0){
  int left=0,right=columns;unsigned char *pixels=frame_pixels+row*line_h*STRIDE;Rect r;
  if(glyph_ready&&!force_draw&&select_start<0&&row!=prompt&&!screen[row][COLS-1]&&!last_screen[row][COLS-1]){int group=1;while((group*cell_w)&7)group++;while(left<right&&screen[row][left]==last_screen[row][left])left++;while(right>left&&screen[row][right-1]==last_screen[row][right-1])right--;left=left/group*group;right=(right+group-1)/group*group;if(right>columns)right=columns;}
  SetRect(&r,left*cell_w,row*line_h,right*cell_w,(row+1)*line_h);
  if(glyph_ready){int y,n=right-left;unsigned char *dst=pixels+left*cell_w/8;for(y=0;y<line_h;y++)memset(dst+y*STRIDE,0,((right-left)*cell_w+7)/8);while(n&&(!screen[row][left+n-1]||screen[row][left+n-1]==' '))n--;if(render_mode==2)asm_cells(dst,(unsigned char*)screen[row]+left,glyph_cache,n,line_h,cell_w);else cells_c(dst,(unsigned char*)screen[row]+left,glyph_cache,n,line_h,cell_w);}
  else{memset(pixels,0,line_h*STRIDE);char rendered[COLS];for(x=0;x<columns;x++)rendered[x]=screen[row][x]?screen[row][x]:' ';if(mono){MoveTo(0,r.top+font_ascent);DrawText(rendered,0,columns);}else for(x=0;x<columns;x++){MoveTo(x*cell_w,r.top+font_ascent);DrawChar(rendered[x]);}}
  if(inverse)InvertRect(&r);
  int a=select_start<select_end?select_start:select_end,z=select_start>select_end?select_start:select_end;if(a>=0&&row<visible)for(x=0;x<columns;x++)if(row*columns+x>=a&&row*columns+x<z){Rect q;SetRect(&q,x*cell_w,r.top,(x+1)*cell_w,r.bottom);InvertRect(&q);}
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
static void draw(void){if(!win)return;activity_hide();unsigned long start=TickCount();int full=force_draw;hide_caret();unsigned short before[342];if(bar_testing)bar_snapshot(before,0);if(render_mode&&frame_pixels)draw_cached();else draw_native();if(bar_testing){bar_checks++;if(bar_snapshot(before,1))bar_failures++;}sync_history_bar(full);paint_ticks+=TickCount()-start;}

static int native_scroll_check(int cached){
 Rect r;SetRect(&r,TEXT_LEFT,TEXT_TOP,TEXT_LEFT+columns*cell_w,TEXT_TOP+visible*line_h);int stride=win->portBits.rowBytes&0x3fff,px=r.left-win->portBits.bounds.left,py=r.top-win->portBits.bounds.top,width=((px&7)+r.right-r.left+7)/8,height=r.bottom-r.top;
 if(FrontWindow()!=win||columns!=80||cell_w!=6||stride!=64||px<0||py<0||px+480>512||py+height>342||!region_contains(win->visRgn,&r)){term_puts("Use an uncovered Monaco 9, 80-column window for this test.\n");return 2;}
 unsigned char *reference=(unsigned char*)NewPtr(STRIDE*HEIGHT),*pixels=(unsigned char*)win->portBits.baseAddr+py*stride+(px>>3);if(!reference)return 1;
 int saved=render_mode,failed=0,i,j,n=0;char line[100],report[260];bar_testing=1;bar_failures=bar_checks=0;render_mode=cached?2:0;build_font_cache();app_clear();
 for(i=0;i<128;i++){snprintf(line,sizeof line,"----- 0000         35          0 TEXT TEST file%04d.txt\n",n++);term_puts(line);}draw();
 for(i=2;i<visible;i++){for(j=0;j<i;j++){snprintf(line,sizeof line,"----- 0000         35          0 TEXT TEST file%04d.txt\n",n++);term_puts(line);}draw();HideCursor();for(j=0;j<height;j++)memcpy(reference+j*STRIDE,pixels+j*stride,width);ShowCursor();force_draw=dirty=1;draw();HideCursor();for(j=0;j<height;j++)if(memcmp(reference+j*STRIDE,pixels+j*stride,width)){failed++;break;}ShowCursor();app_yield();}
 /* Compare reverse/page/jump views with a full redraw, including retained
    boundaries. This catches overlap-direction and stale-row mistakes. */
 for(i=0;i<20;i++){scrollback=(i*17)%(scroll_limit()+1);dirty=1;draw();HideCursor();for(j=0;j<height;j++)memcpy(reference+j*STRIDE,pixels+j*stride,width);ShowCursor();force_draw=dirty=1;draw();HideCursor();for(j=0;j<height;j++)if(memcmp(reference+j*STRIDE,pixels+j*stride,width)){failed++;break;}ShowCursor();app_yield();}
 scrollback=0;bar_testing=0;failed+=bar_failures;
 DisposePtr((Ptr)reference);render_mode=saved;build_font_cache();dirty=force_draw=1;snprintf(report,sizeof report,"%s_SCROLL_TEST %s; %d bursts + 20 views; %d pixel mismatches\n",cached?"CACHED":"NATIVE",failed?"FAIL":"PASS",visible-2,failed);int used=strlen(report);snprintf(report+used,sizeof report-used,"Scrollbar staging: %d checks, %d premature screen changes\n",bar_checks,bar_failures);
 {char full[PATH_CAP];unsigned char path[256];short ref;shell_temp_path(full,cached?"cached-scroll-report.txt":"native-scroll-report.txt");pstr(path,full);HCreate(0,0,path,'S6UX','TEXT');if(!HOpen(0,0,path,fsWrPerm,&ref)){long len=strlen(report);SetEOF(ref,0);FSWrite(ref,&len,report);FSClose(ref);FlushVol(0,0);}}
 term_puts(report);return failed?1:0;
}

/* Executed inside the 68000 guest, not host timing or inferred performance. */
int app_scroll_bench(int quick){
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
 for(w=32;w<=128;w*=2){retained=w;app_clear();for(i=0;i<300;i++)term_puts("ring test\n");if(oldest_line!=current_line-w+1||scroll_limit()!=w-visible)failed++;scrollback=5;long first=current_line-visible+1-scrollback;newline();if(current_line-visible+1-scrollback!=first)failed++;scrollback=scroll_limit();long oldest=oldest_line;newline();if(oldest_line!=oldest+1||scrollback!=scroll_limit())failed++;retained=128;if(oldest_line!=oldest+1)failed++;}
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
 SetPort(win);LocalToGlobal(&origin);p->size=font_size;p->inverse=inverse;p->paging=page_mode;p->retained=retained;p->renderer=render_mode;p->fill=filled;
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
  if(d[4]<6||d[4]>24||d[5]>1||d[6]>2||d[8]>31)return;
  if(n>=48&&((d[7]!=32&&d[7]!=64&&d[7]!=128)||d[44]>2))return;
  memcpy(p.font,d+9,d[8]);p.font[d[8]]=0;p.size=d[4];p.inverse=d[5];p.paging=d[6];if(n>=48){p.retained=d[7];p.renderer=d[44];}
  p.width=d[40]*256+d[41];p.height=d[42]*256+d[43];if(n==52){p.x=(short)((d[48]<<8)|d[49]);p.y=(short)((d[50]<<8)|d[51]);}legacy=1;
 }else if(!prefs_parse(&p,data,n))return;
 short id;pstr(name,p.font);if(installed_font(name,&id))font_id=id;font_size=p.size;inverse=p.inverse;page_mode=p.paging;retained=p.retained;render_mode=p.renderer;
 int maxw=qd.screenBits.bounds.right-8,maxh=qd.screenBits.bounds.bottom-44;
 if(p.width<180)p.width=180;if(p.width>maxw)p.width=maxw;if(p.height<100)p.height=100;if(p.height>maxh)p.height=maxh;
 SizeWindow(win,p.width,p.height,1);MoveWindow(win,p.x,p.y,1);
 {Rect frame=(*((WindowPeek)win)->strucRgn)->rgnBBox;Point origin={0,0};int dx=0,dy=0;SetPort(win);LocalToGlobal(&origin);
 if(frame.right>qd.screenBits.bounds.right-2)dx=qd.screenBits.bounds.right-2-frame.right;
 if(frame.left+dx<qd.screenBits.bounds.left+2)dx=qd.screenBits.bounds.left+2-frame.left;
 if(frame.bottom>qd.screenBits.bounds.bottom-2)dy=qd.screenBits.bounds.bottom-2-frame.bottom;
 if(frame.top+dy<qd.screenBits.bounds.top+GetMBarHeight()+2)dy=qd.screenBits.bounds.top+GetMBarHeight()+2-frame.top;
 if(dx||dy)MoveWindow(win,origin.h+dx,origin.v+dy,1);}
 if(p.fill)fill_window();if(legacy)save_preferences();
}
/* Write/flush a new file before replacing the old one. Keep the previous
   committed file as .bak, also used to recover an interrupted rename. */
static void save_preferences(void){
 struct ClaricePrefs p;char data[1024],tmp[PATH_CAP],backup[PATH_CAP];unsigned char name[256],tn[256],bn[256],base[256];short ref;int e;long n;
 prefs_snapshot(&p);n=prefs_format(data,sizeof data,&p);if(n<0||n>=(long)sizeof data)return;long want=n;
 prefs_sibling(tmp,"clarice.conf.new");prefs_sibling(backup,"clarice.conf.bak");pstr(name,prefs_path);pstr(tn,tmp);pstr(bn,backup);
 HDelete(0,0,tn);e=HCreate(0,0,tn,'S6UX','TEXT');if(!e)e=HOpen(0,0,tn,fsWrPerm,&ref);
 if(!e){e=FSWrite(ref,&n,data);if(!e&&n!=want)e=ioErr;int close=FSClose(ref);if(!e)e=close;}if(!e)e=FlushVol(0,0);
 if(!e){/* Only remove backup when the primary still exists. */
  short old;int exists=HOpen(0,0,name,fsRdPerm,&old);if(!exists){FSClose(old);e=HDelete(0,0,bn);if(e==fnfErr)e=0;pstr(base,"clarice.conf.bak");if(!e)e=shell_rename(name,base);}else if(exists!=fnfErr)e=exists;
  if(!e){const char *tail=strrchr(prefs_path,':');pstr(base,tail?tail+1:prefs_path);e=shell_rename(tn,base);if(e)shell_rename(bn,base);}
 }
 if(e){HDelete(0,0,tn);{char msg[110];snprintf(msg,sizeof msg,"Preferences save failed (%d); previous file retained.\n",e);term_puts(msg);}}FlushVol(0,0);
}
int app_pager_begin(void){int old=page_mode;page_mode=1;page_start=current_line;app_activity_interactive(1);return old;}
int app_pager_end(int old){int quit=cancelled&&!interrupted&&!done;if(quit)cancelled=0;page_mode=old;app_activity_interactive(0);return quit;}
void app_page_begin(void){page_start=current_line;}
int app_page(void){EventRecord ev;long limit=page_mode==2?100:visible-1;if(!page_mode||test_ref>=0)return app_yield();if(limit<1)limit=1;if(current_line-page_start<limit)return app_yield();paging=1;page_action=0;dirty=1;draw();while(!page_action&&!cancelled&&!done){if(WaitNextEvent(everyEvent,&ev,6,0))handle(&ev);if(dirty)draw();}paging=0;if(page_action==2)page_start=current_line-limit+1;else page_start=current_line;dirty=1;return cancelled||done;}
static void fill_window(void){if(!filled){Point origin={0,0};SetPort(win);LocalToGlobal(&origin);normal_bounds=win->portRect;OffsetRect(&normal_bounds,origin.h,origin.v);MoveWindow(win,4,40,1);SizeWindow(win,qd.screenBits.bounds.right-8,qd.screenBits.bounds.bottom-44,1);filled=1;}else{MoveWindow(win,normal_bounds.left,normal_bounds.top,1);SizeWindow(win,normal_bounds.right-normal_bounds.left,normal_bounds.bottom-normal_bounds.top,1);filled=0;}layout();}
static void dialog_text(DialogPtr d,short item,const char *s){short type;Handle h;Rect r;unsigned char p[256];GetDialogItem(d,item,&type,&h,&r);pstr(p,s);SetDialogItemText(h,p);}
static void dialog_read(DialogPtr d,short item,char *s){short type;Handle h;Rect r;unsigned char p[256];GetDialogItem(d,item,&type,&h,&r);GetDialogItemText(h,p);memcpy(s,p+1,p[0]);s[p[0]]=0;}
static ControlHandle dialog_control(DialogPtr d,short item){short type;Handle h;Rect r;GetDialogItem(d,item,&type,&h,&r);return (ControlHandle)h;}
static MenuHandle pref_popup_menus[4];
static int pref_popup_choices[4];
static int popup_slot(int item){return item==5?0:item==12?1:item==14?2:3;}
static pascal void draw_pref_popup(DialogPtr d,short item){
 short type;Handle h;Rect r;GrafPtr old;unsigned char title[256];int k=popup_slot(item);if(!pref_popup_menus[k])return;
 GetPort(&old);SetPort(d);GetDialogItem(d,item,&type,&h,&r);EraseRect(&r);FrameRect(&r);GetMenuItemText(pref_popup_menus[k],pref_popup_choices[k],title);
 RgnHandle saved=NewRgn();if(saved){GetClip(saved);Rect text=r;text.left+=4;text.right-=22;ClipRect(&text);MoveTo(r.left+6,r.top+16);DrawString(title);SetClip(saved);DisposeRgn(saved);}for(int i=0;i<5;i++){MoveTo(r.right-17+i,r.top+9+i);LineTo(r.right-9-i,r.top+9+i);}SetPort(old);
}
static int dialog_popup(DialogPtr d,int item,MenuHandle m,int selected){short type;Handle h;Rect r;GrafPtr old;GetPort(&old);SetPort(d);GetDialogItem(d,item,&type,&h,&r);Point pt={r.top,r.left};LocalToGlobal(&pt);CalcMenuSize(m);HiliteMenu(0);long choice=PopUpMenuSelect(m,pt.v,pt.h,selected);SetPort(old);int result=(short)choice;return result?result:selected;}
static void popup_title(DialogPtr d,int item,MenuHandle m,int selected){short type;Handle h;Rect r;int k=popup_slot(item);pref_popup_menus[k]=m;pref_popup_choices[k]=selected;GetDialogItem(d,item,&type,&h,&r);SetDialogItem(d,item,userItem,(Handle)draw_pref_popup,&r);draw_pref_popup(d,item);}
static void preferences_window(void){
 hide_caret();DialogPtr d=GetNewDialog(140,0,(WindowPtr)-1);if(!d){SysBeep(1);return;}
 MenuHandle fonts=GetMenuHandle(132),lines=GetMenuHandle(134),pages=GetMenuHandle(135),cache=GetMenuHandle(136);
 unsigned char name[256],other[256];char b[256];short item=0;int chosen=1,cap=retained==32?1:retained==64?2:3,page=page_mode+1,render=render_mode+1;
 GetFontName(font_id,name);for(int i=1;i<=CountMItems(fonts);i++){GetMenuItemText(fonts,i,other);if(other[0]==name[0]&&!memcmp(other+1,name+1,name[0]))chosen=i;}
 popup_title(d,5,fonts,chosen);snprintf(b,sizeof b,"%d",font_size);dialog_text(d,7,b);snprintf(b,sizeof b,"%d",columns);dialog_text(d,9,b);
 SetControlValue(dialog_control(d,10),inverse);SetControlValue(dialog_control(d,17),filled);popup_title(d,12,lines,cap);popup_title(d,14,pages,page);popup_title(d,16,cache,render);
 SelectDialogItemText(d,7,0,32767);
 while(item!=2){ModalDialog(0,&item);
  if(item==10||item==17){ControlHandle c=dialog_control(d,item);SetControlValue(c,!GetControlValue(c));}
  if(item==5){chosen=dialog_popup(d,5,fonts,chosen);popup_title(d,5,fonts,chosen);}
  if(item==12){cap=dialog_popup(d,12,lines,cap);popup_title(d,12,lines,cap);}
  if(item==14){page=dialog_popup(d,14,pages,page);popup_title(d,14,pages,page);}
  if(item==16){render=dialog_popup(d,16,cache,render);popup_title(d,16,cache,render);}
  if(item==1){char size[256],cols[256];short id;dialog_read(d,7,size);dialog_read(d,9,cols);GetMenuItemText(fonts,chosen,name);
   int n=preference_number(size),count=preference_number(cols);if(!installed_font(name,&id)||n<6||n>24||count<20||count>130){StopAlert(141,0);continue;}
   int want_fill=GetControlValue(dialog_control(d,17)),dark=GetControlValue(dialog_control(d,10));
   /* Preserve pixel geometry unless columns were explicitly edited; changing
      fonts is allowed even when the former column count no longer fits. */
   int change_columns=count!=columns;SetPort(win);TextFont(id);TextSize(n);FontInfo f;GetFontInfo(&f);int width=CharWidth('M'),fixed=1;
   for(int c=32;c<127;c++)if(CharWidth(c)!=width)fixed=0;if(!fixed)width=f.widMax;if(width<1)width=6;
   int pixels=count*width+TEXT_LEFT+16;TextFont(font_id);TextSize(font_size);
   if(change_columns&&(pixels<180||pixels>qd.screenBits.bounds.right-8)){SetPort(d);StopAlert(141,0);continue;}
   DisposeDialog(d);SetPort(win);if(filled&&(!want_fill||change_columns))fill_window();
   if(change_columns)SizeWindow(win,pixels,win->portRect.bottom,1);
   font_id=id;font_size=n;retained=cap==1?32:cap==2?64:128;inverse=dark;page_mode=page-1;render_mode=render-1;
   if(current_line-retained+1>oldest_line)oldest_line=current_line-retained+1;scrollback=0;
   if(want_fill&&!filled)fill_window();font_layout();save_preferences();return;
  }
 }
 DisposeDialog(d);SetPort(win);dirty=force_draw=1;
}

int app_preferences(int argc,char **argv){char b[256];if(argc==1){unsigned char name[256];GetFontName(font_id,name);if(name[0]>31)name[0]=31;name[name[0]+1]=0;snprintf(b,sizeof b,"Font %.31s, %d point; %d columns x %d rows\nColours %s; paging %s\n",name+1,font_size,columns,visible,inverse?"white on black":"black on white",page_mode==0?"off":page_mode==1?"screen":"100 lines");term_puts(b);snprintf(b,sizeof b,"Scrollback %d lines (fixed 16896-byte ring); cache %s (%d bytes)\n",retained,render_mode?"on":"off",frame_pixels?STRIDE*HEIGHT+256*32:0);term_puts(b);return 0;}

 if(!strcmp(argv[1],"window")){preferences_window();return 0;}
 if(!strcmp(argv[1],"fonts")){MenuHandle m=GetMenuHandle(132);short i;for(i=1;i<=CountMItems(m);i++){unsigned char name[256];GetMenuItemText(m,i,name);term_write((char*)name+1,name[0]);term_puts("\n");}return 0;}
 if(!strcmp(argv[1],"fill"))fill_window();
 else if(argc==3&&!strcmp(argv[1],"font")){unsigned char name[256];short id;pstr(name,argv[2]);if(!installed_font(name,&id)){term_puts("Font is not installed. Use prefs fonts.\n");return 2;}font_id=id;}
 else if(argc==3&&!strcmp(argv[1],"size")){int size=preference_number(argv[2]);if(size<6||size>24)return 2;font_size=size;}
 else if(argc==3&&!strcmp(argv[1],"colours")){if(!strcmp(argv[2],"dark"))inverse=1;else if(!strcmp(argv[2],"light"))inverse=0;else return 2;}
 else if(argc==3&&!strcmp(argv[1],"paging")){if(!strcmp(argv[2],"off"))page_mode=0;else if(!strcmp(argv[2],"screen"))page_mode=1;else if(!strcmp(argv[2],"100"))page_mode=2;else return 2;}
 else if(argc==3&&!strcmp(argv[1],"scrollback")){int n=preference_number(argv[2]);if(n!=32&&n!=64&&n!=128)return 2;retained=n;if(current_line-retained+1>oldest_line)oldest_line=current_line-retained+1;scrollback=0;}
 else if(argc==3&&!strcmp(argv[1],"cache")){if(!strcmp(argv[2],"on"))render_mode=2;else if(!strcmp(argv[2],"off"))render_mode=0;else return 2;}
 else if(argc==3&&!strcmp(argv[1],"renderer")){if(!strcmp(argv[2],"native"))render_mode=0;else if(!strcmp(argv[2],"c"))render_mode=1;else if(!strcmp(argv[2],"asm"))render_mode=2;else return 2;}
 else if(argc==3&&!strcmp(argv[1],"columns")){int n=preference_number(argv[2]),width=n*cell_w+TEXT_LEFT+16;if(n<20||n>130||width>qd.screenBits.bounds.right-8){term_puts("Requested columns do not fit this screen/font.\n");return 2;}if(filled)fill_window();SizeWindow(win,width,win->portRect.bottom,1);}
 else {term_puts("prefs [window | fonts | font name | size 6..24 | colours dark/light | scrollback 32/64/128 | cache on/off | renderer native/c/asm | paging off/screen/100 | columns N | fill]\n");return 2;}
 font_layout();save_preferences();return 0;}
int app_yield(void){EventRecord ev;unsigned long now=TickCount();if(now-last_yield<2)return cancelled||done;if(dirty&&now-last_paint>=output_paint_ticks)draw();unsigned long t=TickCount();if(WaitNextEvent(everyEvent,&ev,0,0))handle(&ev);event_ticks+=TickCount()-t;last_yield=TickCount();activity_tick(last_yield);return cancelled||done;}
/* Canonical terminal stdin: fixed 1 KiB queue, EOF on Ctrl-D, cancellable. */
long app_terminal_read(void *data,unsigned long count){
 if(!count)return 0;
 if(tty_offset>=tty_count){EventRecord ev;tty_count=tty_offset=tty_ready=0;input[0]=0;cursor=0;tty_reading=1;activity_hide();dirty=1;
  while(!tty_ready&&!cancelled&&!done){if(dirty)draw();if(WaitNextEvent(everyEvent,&ev,6,0))handle(&ev);}
  tty_reading=0;dirty=1;if(cancelled||done){input[0]=0;cursor=0;return -1;}
 }
 unsigned long n=tty_count-tty_offset;if(n>count)n=count;memcpy(data,tty_buffer+tty_offset,n);tty_offset+=n;return n;
}
int cli_pause_ticks(unsigned long ticks){EventRecord ev;unsigned long start=TickCount();while(TickCount()-start<ticks&&!cancelled&&!done){if(dirty)draw();if(WaitNextEvent(everyEvent,&ev,6,0))handle(&ev);activity_tick(TickCount());}return cancelled||done;}
void term_present(void){if(dirty)draw();last_yield=TickCount();}
static void copy_selection(void){char b[ROWS*COLS];long n=0;int a=select_start<select_end?select_start:select_end,z=select_start>select_end?select_start:select_end,i;if(a<0||z<=a){ZeroScrap();PutScrap(strlen(input),'TEXT',input);return;}for(i=a;i<z&&i<visible*columns;i++){char c=last_screen[i/columns][i%columns];b[n++]=c?c:' ';if(i%columns==columns-1)b[n++]='\r';}ZeroScrap();PutScrap(n,'TEXT',b);}
static void insert_char(unsigned char c){size_t n=strlen(input);if(n>=LINE_CAP-1){SysBeep(1);return;}memmove(input+cursor+1,input+cursor,n-cursor+1);input[cursor++]=(char)c;dirty=1;}
static void paste(void){Handle h=NewHandle(0);long off=0,n,i;if(!h)return;n=GetScrap(h,'TEXT',&off);if(n>0){HLock(h);for(i=0;i<n;i++){unsigned char c=(unsigned char)(*h)[i];if(c=='\n'||c=='\r')c=' ';if(c>=32||c=='\t')insert_char(c);}HUnlock(h);}DisposeHandle(h);}
static void history_move(int direction){char b[LINE_CAP];if(!history_back)strcpy(saved_input,input);if(direction<0){if(history_get(history_back+1,b)){history_back++;strcpy(input,b);}}else if(history_back>0){history_back--;if(!history_back)strcpy(input,saved_input);else if(history_get(history_back,b))strcpy(input,b);}cursor=strlen(input);dirty=1;}
static void do_search(void){char b[LINE_CAP];int k;busy=1;cancelled=0;k=history_search(search,search_back?search_back+1:1,b);busy=0;if(k){search_back=k;strcpy(input,b);cursor=strlen(input);}dirty=1;}
/* Fixed queue, replayed only by the outer event loop: no recursive execute. */
static unsigned char ahead_keys[LINE_CAP];
static unsigned short ahead_mods[LINE_CAP];
static unsigned int ahead_head,ahead_count;
static unsigned char serial_keys[32];
static int serial_input,serial_head,serial_count;
int app_serial_input(int enabled){serial_input=enabled!=0;serial_head=serial_count=0;return 0;}
int app_serial_key(void){if(!serial_count)return -1;int c=serial_keys[serial_head];serial_head=(serial_head+1)%32;serial_count--;return c;}
static void key(unsigned char c,unsigned short mods){int n=strlen(input);if(paging){if((mods&shiftKey)&&(c==30||c==31)){scrollback+=c==30?visible:-visible;if(scrollback<0)scrollback=0;if(scrollback>scroll_limit())scrollback=scroll_limit();dirty=1;return;}if(c==' '){page_action=1;return;}if(c==13){page_action=2;return;}if(c=='q'||c=='Q'){cancelled=1;return;}}if((mods&(ControlKey|optionKey))&&c>=32&&c<127)c&=31;if((mods&cmdKey)&&c=='.')c=3;if(tty_reading&&c!=3){
 if(c==13||c==4){if(c==4&&!n){tty_ready=1;return;}memcpy(tty_buffer,input,n);tty_count=n;if(c==13)tty_buffer[tty_count++]='\n';tty_offset=0;term_write(input,n);if(c==13)term_puts("\n");input[0]=0;cursor=0;tty_ready=1;dirty=1;return;}
 if(c==8||c==127){if(cursor){memmove(input+cursor-1,input+cursor,n-cursor+1);cursor--;}}
 else if(c==21){input[0]=0;cursor=0;}
 else if(c>=32&&!(mods&cmdKey))insert_char(c);
 dirty=1;return;
 }if(busy){if(c==3){interrupted=1;cancelled=1;}else if(serial_input){if(serial_count<32){serial_keys[(serial_head+serial_count)%32]=c;serial_count++;}else SysBeep(1);}else if(c==27||(c=='.'&&(mods&cmdKey)))cancelled=1;
 else if(!paging&&!activity_interactive){if(ahead_count<LINE_CAP){unsigned int at=(ahead_head+ahead_count)%LINE_CAP;ahead_keys[at]=c;ahead_mods[at]=mods&shiftKey;ahead_count++;}else SysBeep(1);}return;}
 if(c==3){char prefix[132];shell_prompt(prefix,sizeof prefix);term_puts(prefix);term_puts(input);term_puts("^C\n");input[0]=saved_input[0]=continued[0]=0;continuing=0;cursor=history_back=searching=search_back=scrollback=0;shell_status=130;dirty=1;return;}
 if(mods&cmdKey){if(c=='q'||c=='Q'){app_quit();return;}if(c=='c'||c=='C'){copy_selection();return;}if(c=='v'||c=='V'){paste();return;}if(c=='.'){input[0]=0;cursor=0;searching=0;dirty=1;return;}}
 if(searching){if(c==13||c==27){searching=0;dirty=1;return;}if(c==18){do_search();return;}if(c==8||c==127){size_t l=strlen(search);if(l)search[l-1]=0;}else if(c>=32&&strlen(search)<sizeof(search)-1){size_t l=strlen(search);search[l]=c;search[l+1]=0;}search_back=0;do_search();return;}
 if(select_start>=0)force_draw=1;select_start=select_end=-1;
 if(!(mods&shiftKey))scrollback=0;
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
/* An unquoted (or double-quoted) final backslash escapes the submitted newline. */
static int continues_line(const char *s){int quote=0;for(;*s;s++){if(*s=='\\'&&quote!=1){if(!s[1])return 1;s++;continue;}if(*s=='\''&&quote!=2)quote=quote==1?0:1;else if(*s=='"'&&quote!=1)quote=quote==2?0:2;}return 0;}
static void execute(void){char command[LINE_CAP],prefix[132];size_t used=strlen(continued),n=strlen(input);
 if(used+n>=sizeof command){SysBeep(1);return;}strcpy(command,continued);strcat(command,input);
 if(out_col)newline();prompt_prefix(prefix);term_puts(prefix);term_puts(input);term_puts("\n");input[0]=0;cursor=history_back=0;
 if(continues_line(command)){strcpy(continued,command);continued[strlen(continued)-1]=0;continuing=1;dirty=1;return;}continued[0]=0;continuing=0;
 busy=1;cancelled=interrupted=0;dirty=1;draw();activity_command=1;activity_interactive=0;activity_output_tick=TickCount();shell_run(command);activity_command=0;activity_interactive=0;activity_hide();busy=0;if(out_col)newline();if(interrupted){term_puts("^C\n");shell_status=130;}dirty=1;}
void app_run_checks(void){unsigned char pn[256];char path[PATH_CAP];int i,failed=0;static const char *cases[]={"selftest","pwd","ls -la","echo 'alpha beta' > Documents/result.txt","cat Documents/result.txt","grep -F beta Documents/result.txt","export TEST=ready","echo \"$TEST\" >> Documents/result.txt","cd Documents","pwd","cd ..","echo preserved > Documents/fork-test","cat \"Documents/with space.txt\"","status",0};
 shell_temp_path(path,"phase1-report.txt");pstr(pn,path);HCreate(0,0,pn,'S6UX','TEXT');if(!HOpen(0,0,pn,fsWrPerm,&test_ref))SetEOF(test_ref,0);busy=1;cancelled=0;
 for(i=0;cases[i]&&!cancelled;i++){term_puts("$ ");term_puts(cases[i]);term_puts("\n");{char result[96];unsigned long t=TickCount();int rc=shell_run(cases[i]);if(rc)failed++;snprintf(result,sizeof result,"[status %d; %lu ticks]\n",rc,TickCount()-t);term_puts(result);}app_yield();}
 {char recalled[LINE_CAP],result[128];long before=FreeMem(),after;for(i=0;i<140&&!cancelled;i++){if(shell_run("echo"))failed++;app_yield();}after=FreeMem();if(!history_get(128,recalled)||strcmp(recalled,"echo")||history_get(129,recalled))failed++;snprintf(result,sizeof result,"History wrap: 140 commands; heap before %ld after %ld\n",before,after);term_puts(result);}
 term_puts(failed?"PHASE1_CHECKS FAIL\n":"PHASE1_CHECKS PASS\n");if(test_ref>=0){FSClose(test_ref);test_ref=-1;FlushVol(0,0);}busy=0;dirty=1;}
static void menu(long choice){short id=(short)(choice>>16),item=(short)choice;if(id==128){if(item==1){term_puts("Clarice Phase 1 - native System 6 command prompt\nInstalled system font; settings in File > Preferences.\n");}else{MenuHandle m=GetMenuHandle(128);unsigned char name[256];GetMenuItemText(m,item,name);OpenDeskAcc(name);SetPort(win);}}
 if(id==129){if(item==1)app_clear();if(item==2)key(3,0);if(item==4&&!busy)preferences_window();if(item==6)app_quit();}
 if(id==130){if(item==1)copy_selection();if(item==2&&!busy)paste();if(item==4&&!busy)history_move(-1);if(item==5&&!busy)history_move(1);}
 if(id==131&&!busy){if(item==1)app_run_checks();if(item==2)app_metrics(0);}
 if(id==132){unsigned char name[256];GetMenuItemText(GetMenuHandle(132),item,name);GetFNum(name,&font_id);font_layout();save_preferences();}
 if(id==133){if(item==1)inverse=!inverse;if(item==2)fill_window();if(item==4)font_size=9;if(item==5)font_size=12;if(item==6)font_size=14;if(item>=8&&item<=10)page_mode=item-8;font_layout();save_preferences();}
 HiliteMenu(0);dirty=1;}
static int cell(Point p){int x=(p.h-TEXT_LEFT)/cell_w,y=(p.v-TEXT_TOP)/line_h;if(x<0)x=0;if(x>=columns)x=columns-1;if(y<0)y=0;if(y>=visible)y=visible-1;return y*columns+x;}
static void handle(EventRecord *ev){if(ev->what==mouseDown||ev->what==activateEvt||ev->what==updateEvt)activity_hide();WindowPtr clicked;switch(ev->what){case mouseDown:{short part=FindWindow(ev->where,&clicked);if(part==inMenuBar)menu(MenuSelect(ev->where));else if(part==inSysWindow)SystemClick(ev,clicked);else if(clicked==win){SetPort(win);if(part==inDrag){Rect bounds=qd.screenBits.bounds;bounds.top+=20;DragWindow(win,ev->where,&bounds);save_preferences();}else if(part==inGoAway){if(TrackGoAway(win,ev->where))app_quit();}else if(part==inGrow){Rect bounds;long size;SetRect(&bounds,180,100,qd.screenBits.bounds.right-8,qd.screenBits.bounds.bottom-40);size=GrowWindow(win,ev->where,&bounds);if(size){filled=0;SizeWindow(win,(short)size,(short)(size>>16),1);layout();save_preferences();}}else if(part==inContent){if(FrontWindow()!=win)SelectWindow(win);else{Point p=ev->where;ControlHandle control;GlobalToLocal(&p);short hit=FindControl(p,win,&control);if(hit&&control==history_bar){if(hit==inThumb)drag_history(p);else{unsigned long repeat=TickCount()+12;scroll_action(control,hit);while(StillDown()){GetMouse(&p);if(TickCount()>=repeat){if(TestControl(control,p)==hit)scroll_action(control,hit);repeat=TickCount()+3;}SystemTask();}}}else if(paging&&p.v>=TEXT_TOP+visible*line_h&&p.v<TEXT_TOP+(visible+1)*line_h){page_action=1;}else if(p.v<TEXT_TOP+visible*line_h){select_start=select_end=cell(p);while(StillDown()){GetMouse(&p);select_end=cell(p);force_draw=dirty=1;draw();}force_draw=dirty=1;}}}}break;}
 case keyDown:case autoKey:{unsigned char c=(unsigned char)(ev->message&charCodeMask);/* On the Plus, shifted M0110A arrow packets translate to keypad / and =.
     Normalize their physical codes before character editing; main keyboard
     slash/equal use different codes. ADB machines retain their OS translation. */
 if(system_info.machineType==envMacPlus&&(ev->modifiers&shiftKey)){unsigned char code=(ev->message>>8)&255;if(code==0x4d)c=30;else if(code==0x48)c=31;}/* Option-C is the Plus substitute: Option changes the character to cedilla. */if(ev->modifiers&optionKey){unsigned char code=(ev->message>>8)&255;if(code==8)c=3;else if(code==2)c=4;else if(code==32)c=21;}if((ev->modifiers&cmdKey)&&c!='.')menu(MenuKey(c));else key(c,ev->modifiers);break;}
 case updateEvt:if((WindowPtr)ev->message==win){hide_caret();SetPort(win);BeginUpdate(win);force_draw=dirty=1;draw();EndUpdate(win);}break;
 case activateEvt:if((WindowPtr)ev->message==win){background=!(ev->modifiers&activeFlag);dirty=1;}break;
 case osEvt:if((ev->message>>24)==1){background=!(ev->message&1);dirty=force_draw=1;}break;
 default:break;}}
int main(void){Rect r;MenuHandle m;EventRecord ev;unsigned long launch=TickCount();InitGraf(&qd.thePort);InitFonts();InitWindows();InitMenus();TEInit();InitDialogs(0);InitCursor();/* Reserve 64 KiB beyond the monochrome Mac default stack before growing the heap. */SetApplLimit(LMGetApplLimit()-65536);MaxApplZone();MoreMasters();SysEnvirons(1,&system_info);
 m=NewMenu(128,(const unsigned char*)"\p\024");AppendMenu(m,(const unsigned char*)"\pAbout Clarice;(-");AppendResMenu(m,'DRVR');InsertMenu(m,0);
 m=NewMenu(129,(const unsigned char*)"\pFile");AppendMenu(m,(const unsigned char*)"\pClear terminal;Cancel command/.;(-;Preferences.../,;(-;Quit/Q");InsertMenu(m,0);
 m=NewMenu(130,(const unsigned char*)"\pEdit");AppendMenu(m,(const unsigned char*)"\pCopy/C;Paste/V;(-;Previous command/H;Next command/J");InsertMenu(m,0);
 m=NewMenu(131,(const unsigned char*)"\pTests");AppendMenu(m,(const unsigned char*)"\pRun checks/T;Memory and system");InsertMenu(m,0);
 m=NewMenu(132,(const unsigned char*)"\pFont");AppendResMenu(m,'FONT');InsertMenu(m,-1);
 m=NewMenu(134,(const unsigned char*)"\pScrollback");AppendMenu(m,(const unsigned char*)"\p32 lines;64 lines;128 lines");InsertMenu(m,-1);
 m=NewMenu(135,(const unsigned char*)"\pPaging");AppendMenu(m,(const unsigned char*)"\pOff;By screen;100 lines");InsertMenu(m,-1);
 m=NewMenu(136,(const unsigned char*)"\pCache");AppendMenu(m,(const unsigned char*)"\pOff - native;On - C;On - 68000 ASM");InsertMenu(m,-1);
 m=NewMenu(133,(const unsigned char*)"\pView");AppendMenu(m,(const unsigned char*)"\pReverse colours;Fill screen / restore;(-;9 point;12 point;14 point;(-;Paging off;Page by screen;Page by 100 lines");InsertMenu(m,0);DrawMenuBar();
 SetRect(&r,8,48,512,326);win=NewWindow(&win_record,&r,(const unsigned char*)"\pClarice",1,documentProc,(WindowPtr)-1,1,0);if(!win)return 1;SetPort(win);
 {Rect frame=(*((WindowPeek)win)->strucRgn)->rgnBBox;MoveWindow(win,r.left+qd.screenBits.bounds.left+2-frame.left,r.top+qd.screenBits.bounds.top+GetMBarHeight()+2-frame.top,1);}
 scroll_region=NewRgn();saved_clip=NewRgn();transfer_region=NewRgn();visible_region=NewRgn();
 OpenPort(&frame_port);frame_port.portBits.baseAddr=(Ptr)frame_pixels;frame_port.portBits.rowBytes=STRIDE;SetRect(&frame_port.portBits.bounds,0,0,512,HEIGHT);frame_port.portRect=frame_port.portBits.bounds;ClipRect(&frame_port.portRect);frame_ready=1;OpenPort(&prompt_port);prompt_port.portBits.baseAddr=(Ptr)prompt_pixels;prompt_port.portBits.rowBytes=STRIDE;SetRect(&prompt_port.portBits.bounds,0,0,512,64);prompt_port.portRect=prompt_port.portBits.bounds;ClipRect(&prompt_port.portRect);SetPort(win);
 SetRect(&r,win->portRect.right-15,0,win->portRect.right+1,win->portRect.bottom-14);history_bar=NewControl(win,&r,(const unsigned char*)"\p",1,0,0,0,scrollBarProc,0);
 shell_init();shell_state_path(prefs_path,"clarice.conf","Clarice Preferences");load_preferences();font_layout();
 term_puts("Clarice  |  Native System 6 command prompt\nMacintosh Plus / 68000  |  HFS files\nType help. Tab completes; Option-C or Cmd-period interrupts.\n\n");busy=1;shell_profile();busy=0;dirty=1;initial_free=FreeMem();start_tick=TickCount()-launch;last_yield=TickCount();
 while(!done){if(!busy&&ahead_count){unsigned int at=ahead_head;ahead_head=(ahead_head+1)%LINE_CAP;ahead_count--;key(ahead_keys[at],ahead_mods[at]);}if(dirty)draw();if(ahead_count)continue;if(WaitNextEvent(everyEvent,&ev,6,0))handle(&ev);}
 history_shutdown();if(history_bar)DisposeControl(history_bar);if(frame_ready){ClosePort(&prompt_port);ClosePort(&frame_port);}if(frame_pixels)DisposePtr((Ptr)frame_pixels);if(transfer_region)DisposeRgn(transfer_region);if(visible_region)DisposeRgn(visible_region);if(scroll_region)DisposeRgn(scroll_region);if(saved_clip)DisposeRgn(saved_clip);CloseWindow(win);ExitToShell();return 0;}
