/* One-cell screen overlay: never part of terminal text, copies or scrollback. */
#include "activity.h"
static unsigned long activity_output_tick;
static ActivityClock activity_clock;
static int activity_command,activity_interactive,activity_visible,activity_phase=-1;
static unsigned char activity_pixels[4*64] __attribute__((aligned(2)));
static BitMap activity_back;
static Rect activity_rect;
/* CopyBits maps monochrome source bits through the destination port colours.
   Overlay/save buffers already contain final pixels; preserve their polarity. */
static void activity_copy(BitMap *src,BitMap *dst,const Rect *from,const Rect *to){
 long fg=win->fgColor,bg=win->bkColor;ForeColor(blackColor);BackColor(whiteColor);
 CopyBits(src,dst,from,to,srcCopy,0);ForeColor(fg);BackColor(bg);
}
static void activity_hide(void){
 if(activity_visible){GrafPtr old;Rect source={0,0,0,0};GetPort(&old);SetPort(win);
  source.right=activity_rect.right-activity_rect.left;source.bottom=activity_rect.bottom-activity_rect.top;
  activity_copy(&activity_back,&win->portBits,&source,&activity_rect);SetPort(old);
 }
 activity_visible=0;activity_phase=-1;
}
void app_activity_interactive(int enabled){if(shell_session_active())return;activity_interactive=enabled!=0;if(enabled)activity_hide();}
static void activity_tick(unsigned long now){
 int phase=activity_due(&activity_clock,now,activity_output_tick,activity_command&&busy&&!cancelled&&!done,activity_interactive||tty_reading||paging||background);
 if(phase==-1){activity_hide();return;}if(phase==-2)return;
 if(dirty||!win||cell_w>32||line_h>64)return;
 Rect r;SetRect(&r,TEXT_LEFT+(columns-1)*cell_w,TEXT_TOP,TEXT_LEFT+columns*cell_w,TEXT_TOP+line_h);
 if(!region_contains(win->visRgn,&r)){activity_hide();return;}
 if(activity_visible&&phase==activity_phase)return;
 GrafPtr old;Rect source;GetPort(&old);SetPort(win);SetRect(&source,0,0,cell_w,line_h);
 if(!activity_visible){activity_back.baseAddr=(Ptr)activity_pixels;activity_back.rowBytes=4;SetRect(&activity_back.bounds,0,0,32,64);activity_rect=r;activity_copy(&win->portBits,&activity_back,&r,&source);}
 SetPort(&prompt_port);TextFont(font_id);TextSize(font_size);TextFace(0);TextMode(srcCopy);ForeColor(inverse?whiteColor:blackColor);BackColor(inverse?blackColor:whiteColor);
 EraseRect(&source);MoveTo(0,font_ascent);DrawChar("|/-\\"[phase]);InvertRect(&source);
 SetPort(win);activity_copy(&prompt_port.portBits,&win->portBits,&source,&r);SetPort(old);
 activity_visible=1;activity_phase=phase;activity_presented(&activity_clock);
}
