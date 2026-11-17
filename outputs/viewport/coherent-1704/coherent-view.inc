/* Isolated presentation experiment, not a production renderer replacement.
   Keep all existing composition, clipping, selection and scroll decisions.
   For large changes only, redirect their writes to a discardable monochrome
   snapshot, then present the completed result through one clipped CopyBits.
   No callbacks/event dispatch occur while the window points at this buffer. */
#define COHERENT_BYTES (64L*342L)
static Handle coherent_handle;
static int coherent_enabled=1,coherent_fail_alloc; /* local diagnostic fault only */
static unsigned long coherent_frames,coherent_fallbacks,coherent_snapshot_ticks,
 coherent_compose_ticks,coherent_present_ticks,coherent_allocate_ticks;
static void coherent_dispose(void){if(coherent_handle){DisposeHandle(coherent_handle);coherent_handle=0;}}
static int coherent_supported(void){
 BitMap *b=&win->portBits;
 return !(b->rowBytes&0xc000)&&(b->rowBytes&0x3fff)==64&&
  b->bounds.right-b->bounds.left==512&&b->bounds.bottom-b->bounds.top==342&&
  win->portRect.left>=b->bounds.left&&win->portRect.top>=b->bounds.top&&
  win->portRect.right<=b->bounds.right&&win->portRect.bottom<=b->bounds.bottom;
}
static void draw_native_coherent(void){
 /* No staging for ordinary typing, selection changes or one-row scrolling. */
 long first=viewport_live_first()-scrollback;if(first<oldest_line)first=oldest_line;
 int substantial=force_draw||(drawn_first>=0&&labs(first-drawn_first)>=4);
 if(!coherent_enabled||!substantial||!coherent_supported()){draw_native();return;}
 unsigned long t;
 if(coherent_handle&&!*coherent_handle){DisposeHandle(coherent_handle);coherent_handle=0;}
 if(!coherent_handle){t=TickCount();coherent_handle=coherent_fail_alloc?0:NewHandle(COHERENT_BYTES);coherent_allocate_ticks+=TickCount()-t;}
 if(!coherent_handle){coherent_fallbacks++;draw_native();return;}
 HNoPurge(coherent_handle);HLock(coherent_handle);
 if(!*coherent_handle){HUnlock(coherent_handle);HPurge(coherent_handle);coherent_fallbacks++;draw_native();return;}
 BitMap screen=win->portBits,stage=screen;stage.baseAddr=*coherent_handle;
 Rect area=win->portRect;
 SetPort(win);ForeColor(blackColor);BackColor(whiteColor);
 /* Identical screen-coordinate bounds preserve the accepted bit alignment.
    Snapshot the bounded screen bitmap; unchanged native clip/vis and scroll
    visibility guards prevent obscured pixels from entering visible output. */
 t=TickCount();HideCursor();BlockMove(screen.baseAddr,stage.baseAddr,COHERENT_BYTES);ShowCursor();coherent_snapshot_ticks+=TickCount()-t;
 SetPortBits(&stage);t=TickCount();draw_native();coherent_compose_ticks+=TickCount()-t;
 /* Restore the real port before unlocking memory or exposing any events. */
 SetPort(win);SetPortBits(&screen);long fg=win->fgColor,bg=win->bkColor;ForeColor(blackColor);BackColor(whiteColor);
 t=TickCount();CopyBits(&stage,&screen,&area,&area,srcCopy,0);coherent_present_ticks+=TickCount()-t;coherent_frames++;ForeColor(fg);BackColor(bg);
 HUnlock(coherent_handle);HPurge(coherent_handle);
}
