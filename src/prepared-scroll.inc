/* Prepare the exposed top row before an upward history scroll moves pixels.
   Reuses the existing 704-byte row buffer and clipped QuickDraw presentation.
   Other shifts/geometry remain on their existing paths. */
static int prepared_scroll_enabled=1;
static unsigned long prepared_scroll_frames,prepared_scroll_ticks;
#ifdef CLARICE_DRAW_DIAGNOSTICS
static void (*prepared_scroll_observer)(void);
#endif
static int prepared_scroll_draw(void){
 if(!prepared_scroll_enabled||force_draw||select_start>=0||!scrollback||
    FrontWindow()!=win||!direct_stage_supported()||!mono||font_id!=4||
    font_size!=9||cell_w!=6||line_h!=11||font_ascent!=9||columns<1||columns>85)return 0;
 long first=direct_stage_live_first()-scrollback;if(first<oldest_line)first=oldest_line;
 if(first!=drawn_first-1||first+visible>=current_line)return 0;
 Rect area;SetRect(&area,TEXT_LEFT,TEXT_TOP,TEXT_LEFT+columns*6,TEXT_TOP+(visible+1)*11);
 if(!region_contains(win->visRgn,&area)||!region_contains(win->clipRgn,&area))return 0;
 char screen[40][COLS] __attribute__((aligned(2)));int prompt;
 compose_input(screen,&first,&prompt);if(first!=drawn_first-1||prompt<=visible)return 0;
 for(int row=0;row<=visible;row++)if(screen[row][COLS-1])return 0;
 /* Only pixels whose corresponding model rows match are reused. The old
    bottom prompt is discarded, so its caret cannot enter the moved region. */
 for(int row=0;row<visible;row++)if(last_screen[row][COLS-1]||memcmp(screen[row+1],last_screen[row],columns))return 0;
 char rendered[COLS];for(int x=0;x<columns;x++){unsigned char c=screen[0][x];if(c&&(c<32||c>126))return 0;rendered[x]=c?c:' ';}
 int px=TEXT_LEFT-win->portBits.bounds.left,py=TEXT_TOP-win->portBits.bounds.top,phase=px&15;
 if(px<0||py<0||px+columns*6>512||py+(visible+1)*11>342||phase+columns*6>512)return 0;
 unsigned long start=TickCount();
 SetPort(&prompt_port);TextFont(font_id);TextSize(font_size);TextFace(0);TextMode(srcCopy);
 ForeColor(inverse?whiteColor:blackColor);BackColor(inverse?blackColor:whiteColor);
 int ready=native_glyph_row(rendered,columns,phase);glyph_planes_end();SetPort(win);
 if(!ready)return 0;
 prepared_scroll_ticks+=TickCount()-start;
#ifdef CLARICE_DRAW_DIAGNOSTICS
 if(prepared_scroll_observer)prepared_scroll_observer();
#endif
 Rect source=area,destination=area,top=area,row_source;
 source.bottom-=11;destination.top+=11;top.bottom=top.top+11;
 SetRect(&row_source,phase,0,phase+columns*6,11);
 ForeColor(blackColor);BackColor(whiteColor);HideCursor();
 CopyBits(&win->portBits,&win->portBits,&source,&destination,srcCopy,0);
 unsigned long moved=TickCount();
 present_native_row(&row_source,&top);
 unsigned long dt=TickCount()-moved;scroll_exposed_ticks+=dt;if(dt>scroll_exposed_worst)scroll_exposed_worst=dt;
 ShowCursor();ForeColor(inverse?whiteColor:blackColor);BackColor(inverse?blackColor:whiteColor);
 memcpy(last_screen,screen,(visible+1)*COLS);drawn_first=first;drawn_prompt=prompt;
 dirty=force_draw=0;last_paint=TickCount();prepared_scroll_frames++;return 1;
}
