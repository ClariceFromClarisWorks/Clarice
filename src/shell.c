#include "sys6.h"
#include <Files.h>
#include <Serial.h>
#include <Devices.h>
#include <Errors.h>
#include <MacMemory.h>
#include <Resources.h>
#include <OSUtils.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <setjmp.h>
#include <wctype.h>
#include <wchar.h>
#include <locale.h>
#include <langinfo.h>
#include <strings.h>

char shell_cwd[PATH_CAP];
int shell_status;
unsigned long shell_commands;
static short output_ref=-1, input_ref=-1, error_ref=-3, history_ref=-1;
static int script_depth;
static int output_error, history_enabled=1;
static char home_path[PATH_CAP];
static char root_path[PATH_CAP],unix_cwd[PATH_CAP]="/";
static char env_names[16][32],env_values[16][128];
static unsigned long history_count;
static const char *commands[]={"help","appinstall","scrollbench","prefs","sh","test","printf","true","false","pwd","cd","echo","export","history","clear","status","selftest","checks","exit",0};
static void pstr(unsigned char *p,const char *s) {size_t n=strlen(s); if(n>255)n=255;p[0]=(unsigned char)n;memcpy(p+1,s,n);}
#include "streams.inc"
static void say(const char *s){emit(s,(long)asm_length(s));}
static const char *env_get(const char *key){int i;if(!strcmp(key,"HOME"))return "/";if(!strcmp(key,"PWD"))return unix_cwd;for(i=0;i<16;i++)if(!strcmp(env_names[i],key))return env_values[i];return "";}
static int env_set(const char *s){const char *eq=strchr(s,'=');int i;size_t n;if(!eq)return -1;n=(size_t)(eq-s);if(!n||n>=32||strlen(eq+1)>=128)return -1;for(i=0;i<(int)n;i++)if(!(isalnum((unsigned char)s[i])||s[i]=='_')||(i==0&&isdigit((unsigned char)s[i])))return -1;for(i=0;i<16;i++)if(!env_names[i][0]||(strlen(env_names[i])==n&&!strncmp(env_names[i],s,n))){memcpy(env_names[i],s,n);env_names[i][n]=0;strcpy(env_values[i],eq+1);return 0;}return -1;}
#include "namespace.inc"
/* GetCatInfo resolves a full pathname but ioVRefNum is an input, not a
   resolved-volume output. Bind it before callers reuse the directory ID. */
static int info(const char *path,CInfoPBRec *pb){
 unsigned char n[256];short vol;memset(pb,0,sizeof *pb);
 int e=cli_native_volume(path,&vol);if(e)return e;
 pstr(n,path);pb->hFileInfo.ioNamePtr=n;pb->hFileInfo.ioVRefNum=vol;
 return PBGetCatInfoSync(pb);
}
static int directory(const char *s,short *vol,long *id){char p[PATH_CAP];CInfoPBRec pb;int e=resolve(s,p);if(e)return e;e=info(p,&pb);if(e)return e;if(!(pb.hFileInfo.ioFlAttrib&16))return dirNFErr;*id=pb.dirInfo.ioDrDirID;*vol=pb.dirInfo.ioVRefNum;return 0;}
static int open_path(const char *s,short perm,short *ref){char p[PATH_CAP];unsigned char n[256];int e=resolve(s,p);if(e)return e;pstr(n,p);return HOpen(0,0,n,perm,ref);}
/* Accept CLX1 native commands and explicitly supported shell scripts. */
static int program_kind(const char *path){short ref;char head[96];long n=sizeof head-1;int e=open_path(path,fsRdPerm,&ref);if(e)return 0;e=FSRead(ref,&n,head);FSClose(ref);if(e&&e!=eofErr)return 0;head[n]=0;if(n>=4&&!memcmp(head,"CLX1",4))return 2;if(memchr(head,0,n))return 0;if(n>=2&&head[0]=='#'&&head[1]=='!'){char *end=strpbrk(head,"\r\n");if(!end)return 0;*end=0;return !strcmp(head+2,"/bin/sh")||!strcmp(head+2,"/usr/bin/sh");}size_t len=strlen(path);return len>=3&&!strcmp(path+len-3,".sh");}
static int path_entry(const char **cursor,char *out){const char *p=*cursor;if(!p)return 0;const char *end=strchr(p,':');size_t n=end?(size_t)(end-p):strlen(p);if(n>=PATH_CAP)return 0;memcpy(out,p,n);out[n]=0;if(!n)strcpy(out,".");*cursor=end?end+1:0;return 1;}
static int command_path(const char *name,char *out){char native[PATH_CAP];CInfoPBRec pb;int rejected=0;
 if(strchr(name,'/')||!strncmp(name,"native:",7)){if(strlen(name)>=PATH_CAP)return -1;strcpy(out,name);if(resolve(out,native)||info(native,&pb))return 0;if(!(pb.hFileInfo.ioFlAttrib&16)&&program_kind(out))return 1;errsay("command is not an executable command\n");return -1;}
 const char *p=env_get("PATH");while(path_entry(&p,out)){size_t n=strlen(out);if(n+strlen(name)+2>=PATH_CAP)continue;strcat(out,"/");strcat(out,name);if(resolve(out,native)||info(native,&pb))continue;if(!(pb.hFileInfo.ioFlAttrib&16)&&program_kind(out))return 1;rejected=1;}
 if(rejected)errsay("PATH command found, but it is not an executable command\n");return rejected?-1:0;
}
static int redirect(const char *s,int append){char p[PATH_CAP];unsigned char n[256];int e=resolve(s,p);if(e)return e;pstr(n,p);e=HOpen(0,0,n,fsWrPerm,&output_ref);if(e==fnfErr){e=HCreate(0,0,n,'S6UX','TEXT');if(!e)e=HOpen(0,0,n,fsWrPerm,&output_ref);}if(e){output_ref=-1;return e;}e=append?SetFPos(output_ref,fsFromLEOF,0):SetEOF(output_ref,0);if(e){FSClose(output_ref);output_ref=-1;}return e;}
/* Fixed-slot disk ring: 128 x 1028 byte records + header. A record is committed by writing its sequence last. */
#define HISTORY_SLOTS 128
#define HISTORY_SLOT (LINE_CAP+4)
static int hist_record(unsigned long seq,char *out,int writing){unsigned long stored=0;long n;OSErr e;long pos=8+(long)((seq-1)%HISTORY_SLOTS)*HISTORY_SLOT;e=writing?0:SetFPos(history_ref,fsFromStart,pos);if(e)return e;if(writing){long size=0;GetEOF(history_ref,&size);if(size<pos+HISTORY_SLOT){e=SetEOF(history_ref,pos+HISTORY_SLOT);if(e)return e;}e=SetFPos(history_ref,fsFromStart,pos);if(e)return e;n=4;e=FSWrite(history_ref,&n,&stored);if(e)return e;n=LINE_CAP;e=FSWrite(history_ref,&n,out);if(e)return e;SetFPos(history_ref,fsFromStart,pos);n=4;return FSWrite(history_ref,&n,&seq);}n=4;e=FSRead(history_ref,&n,&stored);if(e||stored!=seq)return -1;n=LINE_CAP;e=FSRead(history_ref,&n,out);out[LINE_CAP-1]=0;return e;}
static void history_add(const char *s){char record[LINE_CAP];long n;unsigned long head[2];if(history_ref<0||!history_enabled||!s[0]||s[0]==' ')return;memset(record,0,sizeof record);strncpy(record,s,sizeof(record)-1);if(hist_record(history_count+1,record,1)){term_puts("history: write failed; history disabled\n");history_enabled=0;return;}history_count++;head[0]=0x53364831;head[1]=history_count;SetFPos(history_ref,fsFromStart,0);n=8;if(FSWrite(history_ref,&n,head)){history_enabled=0;term_puts("history: header write failed\n");}}
int history_get(int back,char *out){if(history_ref<0||back<1||back>HISTORY_SLOTS||(unsigned long)back>history_count)return 0;return hist_record(history_count-(unsigned long)back+1,out,0)==0;}
int history_search(const char *needle,int start,char *out){int i;for(i=start;i<=HISTORY_SLOTS&&(unsigned long)i<=history_count;i++){if(history_get(i,out)&&asm_contains(out,needle))return i;if(app_yield())break;}return 0;}
void history_shutdown(void){if(history_ref>=0){FSClose(history_ref);history_ref=-1;FlushVol(0,0);}}
static int install_commands(void);
void shell_init(void){unsigned char vol[256];short vr;WDPBRec pb;unsigned long head[2];long n=8;char path[PATH_CAP];unsigned char pn[256];
 GetVol(vol,&vr);memcpy(shell_cwd,vol+1,vol[0]);shell_cwd[vol[0]]=':';shell_cwd[vol[0]+1]=0;
 /* GetVol may return a working-directory reference; resolve its actual volume name. */
 memset(&pb,0,sizeof pb);pb.ioVRefNum=vr;pb.ioWDIndex=0;if(!PBGetWDInfoSync(&pb))vr=pb.ioWDVRefNum;
 {HParamBlockRec vp;memset(&vp,0,sizeof vp);vp.volumeParam.ioNamePtr=vol;vp.volumeParam.ioVRefNum=vr;if(!PBHGetVInfoSync(&vp)){memcpy(shell_cwd,vol+1,vol[0]);shell_cwd[vol[0]]=':';shell_cwd[vol[0]+1]=0;}}
 {int e=namespace_install();if(e)error("System Folder installation failed",e);}strcpy(shell_cwd,root_path);strcpy(home_path,root_path);migrate_reports();{int e=install_commands();if(e)error("Command installation",e);}env_set("PATH=/bin:/usr/bin");shell_state_path(path,"clarice.history","Clarice History");pstr(pn,path);
 {int e=HOpen(0,0,pn,fsRdWrPerm,&history_ref);if(e==fnfErr){e=HCreate(0,0,pn,'S6UX','DATA');if(!e)e=HOpen(0,0,pn,fsRdWrPerm,&history_ref);}if(e)history_ref=-1;}
 if(history_ref>=0){if(FSRead(history_ref,&n,head)||n!=8||head[0]!=0x53364831){SetEOF(history_ref,0);history_count=0;}else history_count=head[1];}
 else term_puts("History disk unavailable; session remains usable.\n");
}
/* Parser storage has a hard cap; expansion never executes commands. */
static int parse(const char *s,char *buf,char **argv,char **target,int *append){int argc=0,used=0,q=0,active=0,redirect_pending=0;*target=0;*append=0;
 while(*s){unsigned char c=(unsigned char)*s++;
 if(!q&&(c==' '||c=='\t')){if(active){buf[used++]=0;active=0;}continue;}
 if(!q&&(c=='|'||c=='&'||c==';'||c=='<'||c=='`'))return -2;
 if(!q&&c=='>'){if(active){buf[used++]=0;active=0;}if(*target||redirect_pending)return -1;redirect_pending=1;if(*s=='>'){*append=1;s++;}continue;}
 if(!active){if(argc>=MAX_ARGS||used>2040)return -1;if(redirect_pending){*target=buf+used;redirect_pending=0;}else argv[argc++]=buf+used;active=1;}
 if(c=='\''&&q!=2){q=q==1?0:1;continue;}if(c=='"'&&q!=1){q=q==2?0:2;continue;}
 if(c=='\\'&&q!=1){if(!*s)return -1;c=(unsigned char)*s++;}
 else if(c=='$'&&q!=1){char key[32],num[20];const char *value;int k=0,brace=0;
 if(*s=='(')return -2;if(*s=='?'){s++;snprintf(num,sizeof num,"%d",shell_status);value=num;}
 else{if(*s=='{'){brace=1;s++;}while(isalnum((unsigned char)*s)||*s=='_'){if(k>=31)return -1;key[k++]=*s++;}key[k]=0;if(brace){if(*s!='}')return -1;s++;}if(!k){c='$';goto ordinary;}value=env_get(key);}
 if(used+(int)strlen(value)>=2040)return -1;strcpy(buf+used,value);used+=(int)strlen(value);continue;}
 ordinary:if(used>=2040)return -1;buf[used++]=(char)c;
 }
 if(q||redirect_pending)return -1;if(active)buf[used++]=0;argv[argc]=0;return argc;
}
static int selftest(void){unsigned long t=TickCount(),n;int fail=0;char b[2048],*av[MAX_ARGS+1],*dst;int ap;
 {static const char *samples[]={"","a","abc","abababa","0123456789abcdefghijk","odd address","xxxxxxxxxxxxxxxxxxxxxxxx"};unsigned int x,y;for(x=0;x<sizeof samples/sizeof samples[0];x++)for(y=0;y<sizeof samples/sizeof samples[0];y++){if(asm_length(samples[x])!=strlen(samples[x]))fail++;if(asm_contains(samples[x],samples[y])!=(strstr(samples[x],samples[y])!=0))fail++;}for(x=0;x<sizeof samples/sizeof samples[0];x++){char odd[80];strcpy(odd+1,samples[x]);if(asm_length(odd+1)!=strlen(samples[x]))fail++;}}
 {unsigned char src[14],got[56],ref[56];int x;for(x=0;x<14;x++)src[x]=(unsigned char)(x*19);memset(got,0x55,sizeof got);memset(ref,0x55,sizeof ref);for(x=0;x<14;x++)ref[1+x*4]=src[x];asm_glyph(got+1,src,4,14);if(memcmp(got,ref,sizeof got))fail++;}
 if(!asm_contains("abc","")||!asm_contains("abc","bc")||asm_contains("abc","abcd")||asm_contains("abc","z"))fail++;
 if(parse("echo 'a b' \"\" x\\ y",b,av,&dst,&ap)!=4||strcmp(av[1],"a b")||strcmp(av[2],"")||strcmp(av[3],"x y"))fail++;
 if(parse("echo 'bad",b,av,&dst,&ap)>=0||parse("cat a | cat",b,av,&dst,&ap)!=-2)fail++;
 {char line[LINE_CAP]="ec";int pos=2;if(shell_complete(line,&pos)!=1||strcmp(line,"echo ")||pos!=5)fail++;}
 for(n=0;n<10000;n++){if(!asm_contains("0123456789abcdefghijk","efgh"))fail++;if((n&255)==0)app_yield();}
 {char text[128];snprintf(text,sizeof text,"selftest: %s; 10000 scans, %lu ticks\n",fail?"FAIL":"PASS",(unsigned long)(TickCount()-t));say(text);}return fail?1:0;}
#include "port-files.inc"
#include "module.inc"
#include "help.inc"
static int run_args(int argc,char **av){int i,status=0;if(!argc)return 0;
 shell_commands++;
 if(!strcmp(av[0],"help"))status=help_command(argc,av);
 else if(!strcmp(av[0],"appinstall"))status=appinstall_command(argc,av);
 else if(!strcmp(av[0],"pwd")){say(unix_cwd);say("\n");}
 else if(!strcmp(av[0],"echo")){for(i=1;i<argc;i++){if(i>1)say(" ");say(av[i]);}say("\n");}
 else if(!strcmp(av[0],"cd")){char path[PATH_CAP];short v;long id;const char *to=argc>1?av[1]:"~";int e=directory(to,&v,&id);char canonical[PATH_CAP];if(!e)e=resolve_ex(to,path,canonical,1);if(e){error("cd",e);status=1;}else {strcpy(unix_cwd,canonical);strcpy(shell_cwd,path);size_t len=strlen(shell_cwd);if(shell_cwd[len-1]!=':'){shell_cwd[len]=':';shell_cwd[len+1]=0;}}}

 else if(!strcmp(av[0],"export")){if(argc==1){for(i=0;i<16;i++)if(env_names[i][0]){say(env_names[i]);say("=");say(env_values[i]);say("\n");}}for(i=1;i<argc;i++)if(env_set(av[i])){term_puts("export: invalid name/value or environment full\n");status=2;}}
 else if(!strcmp(av[0],"history")){if(argc>1&&!strcmp(av[1],"off"))history_enabled=0;else if(argc>1&&!strcmp(av[1],"on"))history_enabled=1;else if(argc>1&&!strcmp(av[1],"clear")){if(history_ref>=0&&!SetEOF(history_ref,0))history_count=0;else status=1;}else{char line[LINE_CAP];for(i=HISTORY_SLOTS;i>=1;i--)if(history_get(i,line)){say(line);say("\n");if(app_yield()){status=130;break;}}}}
 else if(!strcmp(av[0],"clear"))app_clear();
 else if(!strcmp(av[0],"status")&&argc==2&&!strcmp(av[1],"runtime")){char b[240];snprintf(b,sizeof b,"Runtime: heap=%ld largest=%ld image=%lu ticks=%lu load=%lu owned-peak=%lu remaining=%lu\n",FreeMem(),MaxBlock(),module_bytes,module_ticks,module_load_ticks,port_peak,port_remaining);say(b);snprintf(b,sizeof b,"Loader ticks: read=%lu validate=%lu imports=%lu relocations=%lu prepare=%lu (successful execution only)\n",module_read_ticks,module_validate_ticks,module_import_ticks,module_reloc_ticks,module_prepare_ticks);say(b);}
 else if(!strcmp(av[0],"status")){if(argc>2||(argc==2&&strcmp(av[1],"-h"))){say("usage: status [-h]\n");status=2;}else app_metrics(argc==2);}
 else if(!strcmp(av[0],"scrollbench"))status=app_scroll_bench(argc>1&&!strcmp(av[1],"cached")?3:argc>1&&!strcmp(av[1],"native")?2:argc>1&&!strcmp(av[1],"quick"));
 else if(!strcmp(av[0],"prefs"))status=app_preferences(argc,av);
 else if(!strcmp(av[0],"checks"))app_run_checks();
 else if(!strcmp(av[0],"selftest"))status=selftest();
 else if(!strcmp(av[0],"exit"))app_quit();
 else {say("command not found: ");say(av[0]);say("\n");status=127;}
 if(output_error)status=1;shell_status=status;return status;
}
/* Completion scans the directory without retaining its entries. */
static void match_candidate(const char *s,const char *prefix,char *common,int *count){size_t n=strlen(prefix),i;if(strncmp(s,prefix,n))return;if(!*count)strcpy(common,s);else for(i=0;common[i];i++)if(common[i]!=s[i]){common[i]=0;break;}(*count)++;}
static void completion_item(const char *candidate,const char *prefix,char *common,int *count,int show,int *shown){match_candidate(candidate,prefix,common,count);if(show&&*shown<50&&!strncmp(candidate,prefix,strlen(prefix))){term_puts(candidate);term_puts("  ");(*shown)++;}}
static void complete_functions(const char *,char *,int *,int,int *);
static int builtin_name(const char *s){int i;for(i=0;commands[i];i++)if(!strcmp(commands[i],s))return 1;return 0;}
static void complete_directory(const char *dir,const char *prefix,char *common,int *count,int show,int *shown,int command){char native[PATH_CAP],canonical[PATH_CAP],candidate[100],full[PATH_CAP];short vol;long id;int index=1,e;
 if(resolve_ex(dir,native,canonical,1))return;
 if(!strcmp(canonical,"/mnt")){char name[32];while(!volume_name(index++,name)){if(encode_name(name,candidate,sizeof candidate))continue;strcat(candidate,"/");completion_item(candidate,prefix,common,count,show,shown);if(app_yield())break;}return;}
 if(directory(dir,&vol,&id))return;
 for(;;){CInfoPBRec pb;unsigned char name[256];memset(&pb,0,sizeof pb);pb.hFileInfo.ioNamePtr=name;pb.hFileInfo.ioVRefNum=vol;pb.hFileInfo.ioDirID=id;pb.hFileInfo.ioFDirIndex=index++;e=PBGetCatInfoSync(&pb);if(e)break;name[name[0]+1]=0;if(encode_name((char*)name+1,candidate,sizeof candidate))continue;if(strncmp(candidate,prefix,strlen(prefix)))continue;
 if(strlen(dir)+strlen(candidate)+2>=sizeof full)continue;strcpy(full,dir);strcat(full,"/");strcat(full,candidate);
 if(command){char selected[PATH_CAP];if(builtin_name(candidate)||(pb.hFileInfo.ioFlAttrib&16)||!program_kind(full))continue;if(command_path(candidate,selected)!=1||strcmp(selected,full))continue;}
 else {int folder=pb.hFileInfo.ioFlAttrib&16;if(!folder&&is_link(&pb)){short v;long d;folder=!directory(full,&v,&d);}if(folder)strcat(candidate,"/");}
 completion_item(candidate,prefix,common,count,show,shown);if((index&15)==0&&app_yield())break;
 }
}
int shell_complete(char *line,int *cursor){static char previous[LINE_CAP];static int previous_cursor=-1;int show=previous_cursor==*cursor&&!strcmp(previous,line);int shown=0;char token[PATH_CAP],common[PATH_CAP],dir[PATH_CAP],encoded[PATH_CAP];int start=0,q=0,j=0,i,count=0,cmd=1;const char *base;
 for(i=0;i<*cursor;i++){char c=line[i];if(c=='\\'&&q!=1){i++;continue;}if(c=='\''&&q!=2){q=q==1?0:1;continue;}if(c=='"'&&q!=1){q=q==2?0:2;continue;}if(!q&&(c==' '||c=='\t')){start=i+1;cmd=0;}}
 q=0;for(i=start;i<*cursor&&j<PATH_CAP-2;i++){char c=line[i];if(c=='\''&&q!=2){q=q==1?0:1;continue;}if(c=='"'&&q!=1){q=q==2?0:2;continue;}if(c=='\\'&&q!=1&&i+1<*cursor)c=line[++i];token[j++]=c;}token[j]=0;
 if(cmd&&!strchr(token,'/')){for(i=0;commands[i];i++)completion_item(commands[i],token,common,&count,show,&shown);complete_functions(token,common,&count,show,&shown);const char *path=env_get("PATH");while(path_entry(&path,dir))complete_directory(dir,token,common,&count,show,&shown,1);}
 else{char *slash=strrchr(token,'/');if(slash){size_t n=(size_t)(slash-token)+1;memcpy(dir,token,n);dir[n]=0;base=slash+1;}else{strcpy(dir,".");base=token;}complete_directory(dir,base,common,&count,show,&shown,0);if(count&&slash){char tmp[PATH_CAP];size_t n=(size_t)(slash-token)+1;if(n+strlen(common)>=sizeof tmp)return 0;memcpy(tmp,token,n);strcpy(tmp+n,common);strcpy(common,tmp);}}
 if(shown){term_puts("\n");if(count>shown)term_puts("More matches; type a longer prefix.\n");}if(!count){previous_cursor=-1;return 0;}
 j=0;for(i=0;common[i]&&j<PATH_CAP-3;i++){char c=common[i];if(c==' '||c=='\t'||c=='\\'||c=='\''||c=='"'||c=='$'||c=='>'||c=='&'||c==';'||c=='|'||c=='('||c==')')encoded[j++]='\\';encoded[j++]=c;}if(count==1&&j&&encoded[j-1]!='/')encoded[j++]=' ';encoded[j]=0;
 if(start+j+(int)strlen(line+*cursor)>=LINE_CAP)return 0;memmove(line+start+j,line+*cursor,strlen(line+*cursor)+1);memcpy(line+start,encoded,j);*cursor=start+j;strcpy(previous,line);previous_cursor=*cursor;return count;
}

#include "sh.inc"

static void complete_functions(const char *p,char *c,int *n,int show,int *shown){for(int i=0;i<16;i++)if(sh_functions[i].name[0]&&!builtin_name(sh_functions[i].name))completion_item(sh_functions[i].name,p,c,n,show,shown);}
