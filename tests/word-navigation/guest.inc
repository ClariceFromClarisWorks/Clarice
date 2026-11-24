/* Included only in a disposable diagnostic copy of app.c. */
static short word_ref;
static int word_fail;
static void word_record(const char *name,int pass){char b[160];snprintf(b,sizeof b,"%s %s\n",pass?"PASS":"FAIL",name);long n=strlen(b);FSWrite(word_ref,&n,b);if(!pass)word_fail++;}
static void word_event(int c,int mods){EventRecord e;memset(&e,0,sizeof e);e.what=keyDown;e.message=c;e.modifiers=mods;e.when=TickCount();handle(&e);}
static void word_probe(void){unsigned char pn[]="\pSys6Unix:word-navigation";HCreate(0,0,pn,'CLRC','TEXT');if(HOpen(0,0,pn,fsWrPerm,&word_ref))return;SetEOF(word_ref,0);
 busy=searching=paging=tty_reading=scrollback=0;select_start=select_end=-1;
 strcpy(input,"  one\t two   three ");cursor=strlen(input);word_event(28,cmdKey);word_record("previous word/trailing space",cursor==13);word_event(28,cmdKey);word_record("previous word/repeated space",cursor==7);word_event(29,cmdKey);word_record("next word start",cursor==13);word_event(29,cmdKey);word_record("right end",cursor==19);word_event(29,cmdKey);word_record("right boundary",cursor==19);
 word_event(28,0);word_record("plain left",cursor==18);word_event(29,0);word_record("plain right",cursor==19);
 for(int m=0;m<3;m++){control_mode=m;cursor=19;word_event(28,cmdKey);word_record("Command-arrow precedes control substitution",cursor==13);}
 control_mode=1;
 input[0]=0;cursor=0;word_event(28,cmdKey);word_event(29,cmdKey);word_record("empty boundary",!cursor&&!input[0]);
 strcpy(input,"echo history-value");cursor=strlen(input);execute();history_move(-1);word_event(28,cmdKey);key('X',0);word_record("history recall then word/edit",!strcmp(input,"echo Xhistory-value"));
 input[0]=0;cursor=0;int e=app_screen_open();word_event(28,0);unsigned char bytes[3]={0};long got=app_screen_read(bytes,3);word_record("external screen plain arrow unchanged",!e&&got==3&&!memcmp(bytes,"\033[D",3));word_event(28,cmdKey);word_record("external Command-arrow retains menu handling",!app_screen_pending());app_screen_close();
 memset(text,0,sizeof text);current_line=oldest_line=out_col=0;grid_window(60,20,cell_w,line_h);layout();
 for(int i=0;i<180;i++)input[i]=(i%12==11)?' ':'x';input[180]=0;cursor=180;
 for(int theme=0;theme<2;theme++){inverse=theme;force_draw=dirty=1;draw();int width=columns*cell_w,height=(visible+1)*line_h;Ptr pixels=NewPtr(width*height);if(!pixels){word_record("pixel fixture allocation",0);break;}
 SetPort(win);for(int y=0;y<height;y++)for(int x=0;x<width;x++)pixels[y*width+x]=GetPixel(TEXT_LEFT+x,TEXT_TOP+y);
 int ar,ac,br,bc;input_position(4,cursor,&ar,&ac);word_event(28,cmdKey);input_position(4,cursor,&br,&bc);draw();int good=1;SetPort(win);
 for(int y=0;y<height;y++)for(int x=0;x<width;x++){int old=y/line_h==ar&&x/cell_w==ac,newc=y/line_h==br&&x/cell_w==bc;int expected=pixels[y*width+x]^(old!=newc);if(GetPixel(TEXT_LEFT+x,TEXT_TOP+y)!=expected)good=0;}
 word_record("wrapped movement changes only old/new caret pixels",good);DisposePtr(pixels);cursor=180;
 }
 /* Same existing draw path: compare plain/word event-to-draw cost. */
 for(int mode=0;mode<2;mode++){cursor=180;dirty=1;draw();long free0=FreeMem(),largest=MaxBlock();unsigned long total=0,worst=0,logic=0;
 for(int i=0;i<120;i++){unsigned long t=TickCount();word_event(i&1?29:28,mode?cmdKey:0);logic+=TickCount()-t;draw();unsigned long d=TickCount()-t;total+=d;if(d>worst)worst=d;}
 char b[200];snprintf(b,sizeof b,"mode=%s events=120 logic_ticks=%lu draw_inclusive=%lu worst=%lu heap_delta=%ld largest_delta=%ld\n",mode?"word":"plain",logic,total,worst,FreeMem()-free0,MaxBlock()-largest);long n=strlen(b);FSWrite(word_ref,&n,b);word_record("navigation heap stable",FreeMem()==free0);
 }
 char b[60];snprintf(b,sizeof b,"failures=%d\n",word_fail);long n=strlen(b);FSWrite(word_ref,&n,b);FSClose(word_ref);FlushVol(0,0);input[0]=0;cursor=0;force_draw=dirty=1;term_puts("WORD-PROBE-DONE\n");
}
