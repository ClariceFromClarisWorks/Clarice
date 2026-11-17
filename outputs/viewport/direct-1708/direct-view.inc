/* Isolated 1708 proof: compose complete ASCII history rows straight into a
   discardable bitmap, then present once. No screen snapshot or row CopyBits.
   Sparse changes and every unsupported view use the unchanged native renderer. */
#include "direct-invert.h"
#include "direct-gate.h"
#define DIRECT_STAGE_BYTES (64L*342L)
static Handle direct_stage_handle;
static int direct_stage_enabled=1,direct_stage_fail_alloc,direct_stage_invert_ram=1;
static unsigned long direct_stage_frames,direct_stage_fallbacks,direct_stage_compose_ticks,
 direct_stage_present_ticks,direct_stage_allocate_ticks,direct_stage_invert_ticks;
static void direct_stage_dispose(void){if(direct_stage_handle){DisposeHandle(direct_stage_handle);direct_stage_handle=0;}}
static int direct_stage_supported(void){
 BitMap *b=&win->portBits;
 return !(b->rowBytes&0xc000)&&(b->rowBytes&0x3fff)==64&&
  b->bounds.right-b->bounds.left==512&&b->bounds.bottom-b->bounds.top==342;
}
/* Empty output/input cannot wrap the capped prompt at four or more columns.
   Avoid formatting it merely to reject sparse history; all other states use
   the shared viewport calculation, including tiny diagnostic widths. */
static long direct_stage_live_first(void){
 if(columns>=4&&!input[0]&&!text[current_line%ROWS][0]){long first=current_line-visible;return first<oldest_line?oldest_line:first;}
 return viewport_live_first();
}
static int direct_stage_draw(void){
#ifdef CLARICE_DRAW_DIAGNOSTICS
 if(clear_legacy)return 0; /* legacy diagnostic clears from an unoffset row */
#endif
 if(!direct_stage_enabled||!direct_stage_supported()||select_start>=0||
    !mono||font_id!=4||font_size!=9||cell_w!=6||line_h!=11||font_ascent!=9||
    columns<1||columns>85)return 0;
 long first=direct_stage_live_first()-scrollback;if(first<oldest_line)first=oldest_line;
 if(!force_draw&&(drawn_first<0||labs(first-drawn_first)<4))return 0;
 /* Cheap conservative rejection before building/scanning a second viewport.
    A sparse first row may miss an otherwise useful candidate, never correctness. */
 if(first+visible>=current_line)return 0;
 if(!force_draw&&direct_stage_sparse_first(text[first%ROWS],last_screen[0],columns))return 0;
 char screen[40][COLS] __attribute__((aligned(2)));int prompt,changed=0;
 compose_input(screen,&first,&prompt);
 /* No editable rows, even when an inactive window suppresses its caret. */
 if(prompt<=visible)return 0;
 for(int row=0;row<=visible;row++){
  if(screen[row][COLS-1])return 0;
  int left=0,right=columns;
  while(left<right&&screen[row][left]==last_screen[row][left])left++;
  while(right>left&&screen[row][right-1]==last_screen[row][right-1])right--;
  changed+=right-left;
  for(int x=0;x<columns;x++){unsigned char c=screen[row][x];if(c&&(c<32||c>126))return 0;}
 }
 /* Keep accepted partial-span rendering for mostly identical directory rows. */
 if(!force_draw&&changed*4<columns*(visible+1)*3)return 0;
 BitMap stage=win->portBits;int px=TEXT_LEFT-stage.bounds.left,py=TEXT_TOP-stage.bounds.top;
 if(px<0||py<0||px+columns*6>512||py+(visible+1)*11>342)return 0;
 int phase=px&15;if(phase+columns*6>512)return 0;
 unsigned long t;
 if(direct_stage_handle&&!*direct_stage_handle){DisposeHandle(direct_stage_handle);direct_stage_handle=0;}
 if(!direct_stage_handle){t=TickCount();direct_stage_handle=direct_stage_fail_alloc||!native_heap_room(DIRECT_STAGE_BYTES,32768L,0)?0:NewHandle(DIRECT_STAGE_BYTES);direct_stage_allocate_ticks+=TickCount()-t;}
 if(!direct_stage_handle){direct_stage_fallbacks++;return 0;}
 HNoPurge(direct_stage_handle);HLock(direct_stage_handle);
 stage.baseAddr=*direct_stage_handle;
 if(!stage.baseAddr){HUnlock(direct_stage_handle);HPurge(direct_stage_handle);direct_stage_fallbacks++;return 0;}
 t=TickCount();SetPort(&prompt_port);TextFont(font_id);TextSize(font_size);TextFace(0);TextMode(srcCopy);
 int complete=1;
 for(int row=0;row<=visible;row++){
  char rendered[COLS];for(int x=0;x<columns;x++)rendered[x]=screen[row][x]?screen[row][x]:' ';
  unsigned char *output=(unsigned char*)stage.baseAddr+(py+row*11)*64+((px&~15)>>3);
  if(!native_glyph_row_to(output,rendered,columns,phase)){complete=0;break;}
 }
 direct_stage_compose_ticks+=TickCount()-t;glyph_planes_end();SetPort(win);
 if(complete){
  if(inverse&&direct_stage_invert_ram){t=TickCount();direct_stage_invert((unsigned char*)stage.baseAddr,px,py,columns*6,(visible+1)*11);direct_stage_invert_ticks+=TickCount()-t;}
  Rect area;SetRect(&area,TEXT_LEFT,TEXT_TOP,TEXT_LEFT+columns*6,TEXT_TOP+(visible+1)*11);
  /* Only the fully overwritten text rectangle is published. Boundary bits,
     margins, controls and occluded areas remain under native clipping. */
  if(force_draw){ForeColor(inverse?whiteColor:blackColor);BackColor(inverse?blackColor:whiteColor);erase_margins();}
  long fg=win->fgColor,bg=win->bkColor;ForeColor(blackColor);BackColor(whiteColor);
  t=TickCount();CopyBits(&stage,&win->portBits,&area,&area,inverse&&!direct_stage_invert_ram?notSrcCopy:srcCopy,0);direct_stage_present_ticks+=TickCount()-t;
  ForeColor(fg);BackColor(bg);
  if(force_draw&&saved_clip){ForeColor(blackColor);BackColor(whiteColor);Rect grow=win->portRect;grow.left=grow.right-15;grow.top=grow.bottom-15;GetClip(saved_clip);ClipRect(&grow);DrawGrowIcon(win);SetClip(saved_clip);}
  memcpy(last_screen,screen,(visible+1)*COLS);drawn_first=first;drawn_prompt=prompt;dirty=force_draw=0;last_paint=TickCount();direct_stage_frames++;
 }
 HUnlock(direct_stage_handle);HPurge(direct_stage_handle);return complete;
}
static void draw_native_direct(void){if(!direct_stage_draw())draw_native();}
