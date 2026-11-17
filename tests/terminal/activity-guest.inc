/* Opt-in actual QuickDraw checks; only the disposable Audit volume is touched. */
static void activity_probe(void){
 unsigned char name[]="\pAudit:activity.txt";short ref;HCreate(0,0,name,'CLRC','TEXT');if(HOpen(0,0,name,fsWrPerm,&ref))return;SetEOF(ref,0);
 int saved=inverse,failures=0;unsigned char pixels[32*64];long heap=FreeMem();
 for(int theme=0;theme<2;theme++)for(int polarity=0;polarity<2;polarity++){
  inverse=theme;force_draw=dirty=1;draw();SetPort(win);ForeColor(polarity?whiteColor:blackColor);BackColor(polarity?blackColor:whiteColor);
  Rect r;SetRect(&r,TEXT_LEFT+(columns-1)*cell_w,TEXT_TOP,TEXT_LEFT+columns*cell_w,TEXT_TOP+line_h);int n=0;
  for(int y=r.top;y<r.bottom;y++)for(int x=r.left;x<r.right;x++)pixels[n++]=GetPixel(x,y);
  memset(&activity_clock,0,sizeof activity_clock);busy=activity_command=1;activity_interactive=cancelled=0;activity_output_tick=TickCount()-301;unsigned long now=TickCount();
  int ok=1;for(int phase=0;phase<4;phase++){activity_tick(now+30*phase);if(!activity_visible||GetPixel(r.right-1,r.top)!=!inverse||win->fgColor!=(polarity?whiteColor:blackColor))ok=0;}
  cancelled=1;activity_tick(now+120);cancelled=0;n=0;
  if(activity_visible)ok=0;for(int y=r.top;y<r.bottom;y++)for(int x=r.left;x<r.right;x++)if(pixels[n++]!=GetPixel(x,y))ok=0;
  busy=activity_command=0;char b[100];snprintf(b,sizeof b,"%s theme=%d port-polarity=%d frames/inverse/restoration\n",ok?"PASS":"FAIL",theme,polarity);long len=strlen(b);FSWrite(ref,&len,b);if(!ok)failures++;
 }
 inverse=saved;force_draw=dirty=1;draw();char b[100];snprintf(b,sizeof b,"failures=%d heap_before=%ld heap_after=%ld\n",failures,heap,FreeMem());long len=strlen(b);FSWrite(ref,&len,b);FSClose(ref);
 /* Explicitly unmount only our disposable results volume before host export. */
 for(int i=1;;i++){HParamBlockRec pb;unsigned char vol[256];memset(&pb,0,sizeof pb);pb.volumeParam.ioNamePtr=vol;pb.volumeParam.ioVolIndex=i;if(PBHGetVInfoSync(&pb))break;if(vol[0]==5&&!memcmp(vol+1,"Audit",5)){short v=pb.volumeParam.ioVRefNum;FlushVol(0,v);ParamBlockRec u;memset(&u,0,sizeof u);u.volumeParam.ioVRefNum=v;PBUnmountVol(&u);break;}}
 term_puts("ACTIVITY-PROBE-DONE\n");
}
