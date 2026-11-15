/* Extended opt-in UX probe: real Toolbox layout/resources and event dispatch. */
static void ux_event(unsigned code,unsigned char c,unsigned mods){EventRecord ev;memset(&ev,0,sizeof ev);ev.what=keyDown;ev.message=(code<<8)|c;ev.modifiers=mods;handle(&ev);}
static void ux_probe(void){
 ClariceHardware h;long heap=FreeMem();unsigned long t=TickCount();int caps=1;
 for(int i=0;i<20;i++){if(cli_hardware(&h,sizeof h)||h.version!=1||h.cpu!=1||h.ram!=4194304||h.width!=512||h.height!=342||h.depth!=1)caps=0;}
 usability_record("hardware snapshot stock mono 4 MB and no retained allocation",caps&&FreeMem()==heap);
 char report[180];snprintf(report,sizeof report,"hardware 20 calls=%lu ticks model=%s cpu=%u fpu=%d system=%x\n",TickCount()-t,h.model_name,h.cpu,h.fpu,h.system);long n=strlen(report);FSWrite(usability_ref,&n,report);
 int saved_mode=control_mode;usability_reset();
 for(control_mode=0;control_mode<3;control_mode++){
  busy=1;int e=app_screen_open();unsigned char byte=0;
  ux_event(7,'x',ControlKey);long got=(app_screen_pending()?app_screen_read(&byte,1):0);usability_record("physical Ctrl-X editor",!e&&got==1&&byte==24);
  if(control_mode==1){ux_event(13,0xb7,optionKey);got=(app_screen_pending()?app_screen_read(&byte,1):0);usability_record("Option-W uses keyboard map not accented character",got==1&&byte==23);}
  if(control_mode==2){ux_event(7,'x',cmdKey);got=(app_screen_pending()?app_screen_read(&byte,1):0);usability_record("Command-X maps to Ctrl-X",got==1&&byte==24);ux_event(8,0x8d,cmdKey|optionKey);got=(app_screen_pending()?app_screen_read(&byte,1):0);usability_record("Command-Option-C editor control not local abort",got==1&&byte==3&&!cancelled);}
  ux_event(27,'_',ControlKey|shiftKey);got=(app_screen_pending()?app_screen_read(&byte,1):0);usability_record("Ctrl-underscore is a control byte, not an arrow",got==1&&byte==31&&!app_screen_pending());
  app_screen_close();usability_reset();
  memcpy(last_screen[0],"copy",4);select_start=0;select_end=4;ux_event(8,'c',cmdKey);Handle scrap=NewHandle(0);long off,count=GetScrap(scrap,'TEXT',&off);usability_record("Command-C keeps Macintosh copy",count==4&&!memcmp(*scrap,"copy",4));DisposeHandle(scrap);select_start=select_end=-1;
  busy=1;app_serial_tty(1);ux_event(8,'c',ControlKey);usability_record("serial Ctrl-C delivered remotely",app_serial_key()==3&&!cancelled);ux_event(47,'.',cmdKey);usability_record("serial Command-period local cancellation",cancelled);app_serial_tty(0);usability_reset();
 }
 usability_reset();busy=1;ux_event(27,'_',ControlKey|shiftKey);
 int queued_open=app_screen_open();unsigned char queued=0;long queued_n=app_screen_pending()?app_screen_read(&queued,1):0;
 usability_record("queued Ctrl-underscore keeps its control modifier",!queued_open&&queued_n==1&&queued==31&&!app_screen_pending());app_screen_close();usability_reset();
 strcpy(input,"unchanged");cursor=3;int previous_history=history_back;ux_event(27,'_',ControlKey|shiftKey);
 usability_record("unsupported shell control is not an arrow",!strcmp(input,"unchanged")&&cursor==3&&history_back==previous_history);usability_reset();
 control_mode=saved_mode;
 app_clear();current_line=100;oldest_line=0;for(int r=0;r<=100;r++)snprintf(text[r],COLS,"History row %03d",r);strcpy(input,"keep");cursor=4;scrollback=3;draw();
 char screen[40][COLS];long first;int prompt;compose_input(screen,&first,&prompt);int clean=prompt==visible+1;
 for(int r=0;r<=visible;r++)if(memcmp(screen[r],text[(first+r)%ROWS],columns)||screen[r][COLS-1])clean=0;
 usability_record("scrollback fills last row, no pinned prompt/cursor",clean);
 select_start=visible*columns;select_end=select_start+7;copy_selection();Handle scrap=NewHandle(0);long off,count=GetScrap(scrap,'TEXT',&off);usability_record("copy includes last history row",count==7&&!memcmp(*scrap,"History",7));DisposeHandle(scrap);select_start=select_end=-1;
 ux_event(1,'S',shiftKey);usability_record("shifted typing returns live and retains key",scrollback==0&&!strcmp(input,"keepS"));
 scrollback=3;ZeroScrap();PutScrap(2,'TEXT',"XY");ux_event(9,'v',cmdKey);usability_record("paste returns live and retains contents",!scrollback&&!strcmp(input,"keepSXY"));
 busy=1;scrollback=3;key('z',0);usability_record("foreground typeahead returns to live view",!scrollback&&ahead_count==1);busy=0;ahead_count=ahead_head=0;
 int saved_theme=inverse;long line=current_line;char oldrow[COLS];memcpy(oldrow,text[0],COLS);
 for(int dark=0;dark<2;dark++){char *args[]={"prefs","colours",dark?"dark":"light"};Rect tiny;SetRect(&tiny,4,0,8,4);InvalRect(&tiny);app_preferences(3,args);EventRecord ev;while(GetNextEvent(updateMask,&ev))handle(&ev);draw();usability_record("theme immediately fills margins, preserves input and history",GetPixel(TEXT_LEFT-1,line_h*3)==dark&&!strcmp(input,"keepSXY")&&line==current_line&&!memcmp(oldrow,text[0],COLS));}
 inverse=saved_theme;font_layout();SetPort(win);InvalRect(&win->portRect);save_preferences();usability_reset();
}
