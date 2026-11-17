/* Opt-in native viewport regression; all fixtures are disposable. */
static void viewport_probe(void){
 char path[PATH_CAP],report[2048];unsigned char pn[256];short ref;int used=0;
 shell_temp_path(path,"viewport-v2.txt");pstr(pn,path);HCreate(0,0,pn,'CLRC','TEXT');OSErr open_error=HOpen(0,0,pn,fsWrPerm,&ref);if(open_error){char err[80];snprintf(err,sizeof err,"VIEWPORT open failed %d\n",open_error);term_puts(err);return;}SetEOF(ref,0);
 int oldh=win->portRect.bottom,oldw=win->portRect.right,saved_inverse=inverse;long heap=FreeMem();
 input[0]=0;cursor=0;busy=0;page_mode=0;app_clear();
 for(int i=0;i<110;i++){char l[64];snprintf(l,sizeof l,"row %03d abcdefghijklmnopqrstuvwxyz\n",i);term_puts(l);}scrollback=10;force_draw=dirty=1;draw();
 char model[40][COLS];long before,after;int pr;compose_input(model,&before,&pr);
 SizeWindow(win,oldw,oldh-55,1);layout();compose_input(model,&after,&pr);
 used+=snprintf(report+used,sizeof report-used,"height anchor before=%ld after=%ld %s\n",before,after,before==after?"PASS":"FAIL");
 SizeWindow(win,oldw,oldh,1);layout();force_draw=dirty=1;draw();
 /* A narrow update outside the bar must not poison its staging bitmap. */
 RgnHandle original=NewRgn();GetClip(original);Rect tiny;SetRect(&tiny,TEXT_LEFT,TEXT_TOP,TEXT_LEFT+8,TEXT_TOP+8);ClipRect(&tiny);sync_history_bar(1);SetClip(original);
 scrollback++;sync_history_bar(0);unsigned short snapshot[342];bar_snapshot(snapshot,0);sync_history_bar(1);int mismatch=bar_snapshot(snapshot,1);
 used+=snprintf(report+used,sizeof report-used,"bar clipped-staging vs full rebuild %s\n",mismatch?"FAIL":"PASS");DisposeRgn(original);
 for(int theme=0;theme<2;theme++){
  char *args[]={"prefs","colours",theme?"dark":"light"};int rc=app_preferences(3,args);SetPort(win);int pixel=GetPixel(TEXT_LEFT-1,line_h*3);
  used+=snprintf(report+used,sizeof report-used,"theme requested=%d actual=%d margin=%d status=%d\n",theme,inverse,pixel,rc);
 }
 inverse=saved_inverse;scrollback=0;force_draw=dirty=1;draw();
 used+=snprintf(report+used,sizeof report-used,"heap delta=%ld\n",FreeMem()-heap);long n=used;FSWrite(ref,&n,report);FSClose(ref);unsigned char vol[]={7,'R','C','G','u','e','s','t'};FlushVol(vol,0);term_puts(report);
}
