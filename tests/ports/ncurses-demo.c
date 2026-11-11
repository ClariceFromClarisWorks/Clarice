/* Exercises real upstream ncurses windows, attributes and keyboard decoding. */
#include <curses.h>
#include <stdio.h>
extern int app_screen_open(void);
extern void app_screen_close(void);
static volatile sig_atomic_t signals;
static void caught(int sig){if(sig==SIGINT)signals++;}
int upstream_main(int argc,char **argv){
 if(argc>1&&!strcmp(argv[1],"--check-runtime")){
  if(strcmp(getenv("TERM"),"clarice")||!getenv("HOME"))return 2;
  if(setenv("CLARICE_PROBE","private",1)||strcmp(getenv("CLARICE_PROBE"),"private"))return 3;
  if(setenv("CLARICE_PROBE","ignored",0)||strcmp(getenv("CLARICE_PROBE"),"private"))return 4;
  if(unsetenv("CLARICE_PROBE")||getenv("CLARICE_PROBE"))return 5;
  if(signal(SIGINT,caught)==SIG_ERR||raise(SIGINT)||signals!=1)return 6;
  signal(SIGINT,SIG_IGN);if(raise(SIGINT)||signals!=1)return 7;
  puts("PASS: private environment and cooperative signal handlers");return 0;
 }
 if(!cli_isatty(0)||!cli_isatty(1)){fprintf(stderr,"cursesdemo: interactive terminal required\n");return 2;}if(app_screen_open())return 1;
 WINDOW *s=initscr();if(!s){app_screen_close();return 2;}
 int sigmode=argc>1&&!strcmp(argv[1],"--signals");if(sigmode){cbreak();signal(SIGINT,caught);}else raw();noecho();keypad(stdscr,TRUE);
 int key=0,count=0;while(key!='q'){erase();attron(A_REVERSE);mvaddstr(0,0,"Clarice: upstream ncurses 6.5");attroff(A_REVERSE);mvprintw(2,0,"Key %d; events %d; signals %d",key,count++,(int)signals);mvaddstr(4,0,sigmode?"Ctrl+C calls handler; Q exits.":"Arrows decode; Ctrl+C is input; Q exits.");move(6,0);refresh();key=getch();if(key==ERR&&!sigmode)break;}
 endwin();app_screen_close();printf("ncurses demo: %d events; exit key %d; signals %d\n",count,key,(int)signals);return key=='q'?0:1;
}
