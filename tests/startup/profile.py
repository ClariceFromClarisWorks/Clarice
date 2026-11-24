"""Instrument a disposable source copy, never the production build directory."""
from pathlib import Path
import shutil
r=Path(__file__).resolve().parents[2];w=r/'work/startup-q1';s=w/'src'
if s.exists():shutil.rmtree(s)
shutil.copytree(r/'src',s)
link=w/'work'
if not link.exists():link.symlink_to('..',target_is_directory=True)
p=s/'sys6.h';p.write_text(p.read_text()+'\nvoid startup_mark(int);\n')
p=s/'app.c';head,main=p.read_text().split('int main(void)',1)
probe=r'''
static unsigned long startup_times[24];
void startup_mark(int n){startup_times[n]=TickCount();}
static void startup_report(void){char b[600];int used=0;
 for(int i=0;i<24;i++)used+=snprintf(b+used,sizeof b-used,"%d=%lu ",i,startup_times[i]?startup_times[i]-startup_times[0]:0);
 used+=snprintf(b+used,sizeof b-used,"heap=%ld largest=%ld paint=%lu\n",FreeMem(),MaxBlock(),paint_ticks);
 unsigned long stamp=0;short sr;unsigned char sn[]="\pSys6Unix:launch-q1";
 if(!HOpen(0,0,sn,fsRdPerm,&sr)){long count=4;FSRead(sr,&count,&stamp);FSClose(sr);used+=snprintf(b+used,sizeof b-used,"native-entry=%lu\n",startup_times[0]-stamp);}
 unsigned char path[]="\pSys6Unix:startup-q1";short ref;HCreate(0,0,path,'CLRC','TEXT');
 if(!HOpen(0,0,path,fsWrPerm,&ref)){SetFPos(ref,fsFromLEOF,0);long n=used;FSWrite(ref,&n,b);FSClose(ref);}}
'''
for text,n in [('unsigned long launch=TickCount();',0),('SysEnvirons(1,&system_info);',1),('DrawMenuBar();',2),('if(!win)return 1;SetPort(win);',23),('shell_init();',10),('shell_state_path(prefs_path,"clarice.conf","Clarice Preferences");',5),('load_preferences();',6),('font_layout();',7),('welcome();busy=1;',8),('shell_profile();',9)]:
 assert text in main,text
 main=main.replace(text,text+f'startup_mark({n});',1)
for text,n in [('scroll_region=NewRgn();',3),('shell_init();',4),('shell_profile();',19)]:main=main.replace(text,f'startup_mark({n});'+text,1)
main=main.replace(' while(!done){',' startup_mark(11);draw();startup_mark(12);startup_report();\n while(!done){',1)
p.write_text(head+probe+'int main(void)'+main)
p=s/'shell.c';t=p.read_text().replace('{int e=namespace_install();','startup_mark(13);{int e=namespace_install();').replace('strcpy(shell_cwd,root_path);','startup_mark(14);strcpy(shell_cwd,root_path);',1).replace('migrate_reports();','migrate_reports();startup_mark(15);',1).replace('if(history_ref>=0){if(FSRead','startup_mark(16);if(history_ref>=0){if(FSRead',1);p.write_text(t)
p=s/'namespace.inc';t=p.read_text().replace('static const char *dirs[]={','startup_mark(17);static const char *dirs[]={',1).replace(' snprintf(part,sizeof part,"%.240setc:profile"',' startup_mark(18);snprintf(part,sizeof part,"%.240setc:profile"',1);p.write_text(t)
p=s/'sh.inc';t=p.read_text().replace('char *args[]={"/etc/profile"};','startup_mark(20);char *args[]={"/etc/profile"};').replace('if(!rc){script_depth++;sh_flow=0;rc=sh_source(text);script_depth--;}','if(!rc){startup_mark(21);script_depth++;sh_flow=0;rc=sh_source(text);script_depth--;startup_mark(22);}',1);p.write_text(t)
print(s)
