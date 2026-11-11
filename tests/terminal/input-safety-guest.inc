/* Uses the actual input, Scrap Manager and screen paths in the diagnostic guest. */
static int probe_failures;
static void probe_check(int ok,const char *name){char b[160];snprintf(b,sizeof b,"%s %s\n",ok?"PASS":"FAIL",name);probe_write(b);if(!ok)probe_failures++;}
static void probe_reset(void){busy=tty_reading=cancelled=interrupted=0;key(3,0);input[0]=0;cursor=0;}
static void probe_scrap(const char *p,long n){ZeroScrap();PutScrap(n,'TEXT',(Ptr)p);paste();}
static void input_safety_probe(void){char b[1100];unsigned long commands;long heap;
 probe_reset();strcpy(input,"abcd");cursor=2;probe_scrap("XY",2);probe_check(!strcmp(input,"abXYcd")&&cursor==4,"paste inserts at cursor");
 probe_reset();for(int i=0;i<900;i++)b[i]='a'+i%26;commands=shell_commands;probe_scrap(b,900);probe_check(strlen(input)==900&&!memcmp(input,b,900)&&shell_commands==commands,"900 byte paste intact, not executed");
 heap=FreeMem();for(int i=0;i<8;i++){input[0]=0;cursor=0;probe_scrap(b,900);}probe_check(FreeMem()==heap,"repeated paste releases temporary heap");
 probe_reset();const char *multi="echo first > /tmp/qol2-first\r\necho second > /tmp/qol2-second\r";commands=shell_commands;probe_scrap(multi,strlen(multi));probe_check(!strcmp(input,"echo first > /tmp/qol2-first\necho second > /tmp/qol2-second\n")&&shell_commands==commands,"multiline CRLF preserved without automatic execution");key(13,0);probe_check(shell_status==0,"explicit Return executes multiline script");
 probe_reset();commands=shell_commands;memset(b,'x',sizeof b);probe_scrap(b,sizeof b);key(13,0);probe_check(input_blocked&&!input[0]&&shell_commands==commands,"oversized paste blocks submission");
 for(int i=0;i<10;i++)key('x',0);key(13,0);probe_check(shell_commands==commands,"overflow suffix cannot execute");probe_reset();probe_check(!input_blocked,"Ctrl+C explicitly resets blocked input");
 commands=shell_commands;for(int i=0;i<1024;i++)key('x',0);key(13,0);probe_check(input_blocked&&!input[0]&&shell_commands==commands,"typed overflow cannot execute truncated line");
 probe_reset();probe_scrap("echo safe\003echo unsafe",21);probe_check(input_blocked&&!input[0],"embedded control byte rejects whole paste");
 probe_reset();memset(continued,'x',900);continued[900]=0;memset(input,'y',200);input[200]=0;cursor=200;commands=shell_commands;key(13,0);probe_check(input_blocked&&shell_commands==commands,"continued command overflow blocked");
 probe_reset();busy=1;commands=shell_commands;for(int i=0;i<LINE_CAP+1;i++)key('x',0);busy=0;key(13,0);probe_check(input_blocked&&!ahead_count&&shell_commands==commands,"pending queue overflow fails closed");
 probe_reset();strcpy(input,"abc");cursor=3;key(28,0);key('X',0);key(8,0);probe_check(!strcmp(input,"abc")&&cursor==2,"left, insert, backspace editing");
 probe_reset();shell_run("echo qol2-history");history_move(-1);probe_check(!strcmp(input,"echo qol2-history"),"history Up");history_move(1);probe_check(!input[0],"history Down restores draft");
 strcpy(input,"pw");cursor=2;key(9,0);probe_check(!strcmp(input,"pwd "),"Tab completion");probe_reset();
 probe_check(!app_screen_open(),"ncurses screen ownership");if(editor_screen){unsigned char got[8];key(30,0);key('z',0);key(3,0);long n=app_screen_read(got,sizeof got);probe_check(n==5&&!memcmp(got,"\033[Az\003",5)&&!cancelled,"ncurses arrow, text, Ctrl+C bytes preserved");busy=1;for(int i=0;i<129;i++)key('x',0);probe_check(input_blocked&&cancelled,"ncurses queue overflow reported and cancelled");app_screen_close();busy=0;}
 probe_reset();probe_curses=1;busy=1;int result=shell_run("cursesdemo");busy=0;probe_check(!result&&!editor_screen&&!probe_curses,"upstream ncurses demo consumes arrows/text/Ctrl+C/q");
 probe_reset();snprintf(b,sizeof b,"SAFETY-DONE failures=%d heap=%ld\n",probe_failures,FreeMem());probe_write(b);
}
