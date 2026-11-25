#include "sys6.h"
#include "session.h"
#include "service.h"
#include "remote-settings.h"
static const ClariceSessionIO *session_io;
static int session_exit_requested;
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
static short self_update_resources;
static short output_ref=-1, input_ref=-1, error_ref=-3, history_ref=-1;
static int script_depth;
static int output_error, history_enabled=1;
static char home_path[PATH_CAP];
static char root_path[PATH_CAP],unix_cwd[PATH_CAP]="/";
#include "shell-env.inc"
static unsigned long history_count;
static int builtin_help(const char *);
static const char *commands[]={"clarice","remote","env","time","command","help","appinstall","scrollbench","prefs","sh","test","printf","true","false","pwd","cd","echo","export","history","clear","status","selftest","checks","exit",0};
static void pstr(unsigned char *p,const char *s) {size_t n=strlen(s); if(n>255)n=255;p[0]=(unsigned char)n;memcpy(p+1,s,n);}
#include "streams.inc"
static void say(const char *s){emit(s,(long)asm_length(s));}
static const char *env_get(const char *key){struct sh_env *e=env_find(sh_env,key);if(e)return e->value;if(!strcmp(key,"HOME"))return "/home";if(!strcmp(key,"PWD"))return unix_cwd;return "";}
static int shell_relocate_cwd(const char *,const char *,int);
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
#ifdef CLARICE_DRAW_DIAGNOSTICS
static int dispatch_legacy;
static unsigned long dispatch_path_ticks,dispatch_kind_ticks;
void shell_dispatch_legacy(int old){dispatch_legacy=old;}
#else
#define dispatch_legacy 0
#endif
/* Accept CLX1 native commands and explicitly supported shell scripts. */
static int program_kind_native(const char *path,const char *native){short ref;unsigned char pn[256];char head[96];long n=sizeof head-1;pstr(pn,native);int e=HOpen(0,0,pn,fsRdPerm,&ref);if(e)return 0;e=FSRead(ref,&n,head);FSClose(ref);if(e&&e!=eofErr)return 0;head[n]=0;if(n>=4&&!memcmp(head,"CLX1",4))return 2;if(memchr(head,0,n))return 0;if(n>=2&&head[0]=='#'&&head[1]=='!'){char *end=strpbrk(head,"\r\n");if(!end)return 0;*end=0;return !strcmp(head+2,"/bin/sh")||!strcmp(head+2,"/usr/bin/sh");}size_t len=strlen(path);return len>=3&&!strcmp(path+len-3,".sh");}
/* Reuse the resolved path while choosing an executable; no resident cache. */
static int program_kind(const char *path){char native[PATH_CAP];if(resolve(path,native))return 0;return program_kind_native(path,native);}
static int path_entry(const char **cursor,char *out){while(*cursor){const char *p=*cursor,*end=strchr(p,':');size_t n=end?(size_t)(end-p):strlen(p);*cursor=end?end+1:0;if(n>=PATH_CAP)continue;memcpy(out,p,n);out[n]=0;if(!n)strcpy(out,".");return 1;}return 0;}
static int command_path(const char *name,char *out,int *kind){char native[PATH_CAP];CInfoPBRec pb;int rejected=0;
 if(strchr(name,'/')||!strncmp(name,"native:",7)){if(strlen(name)>=PATH_CAP)return -1;strcpy(out,name);if(resolve(out,native)||info(native,&pb))return 0;if(!(pb.hFileInfo.ioFlAttrib&16)){int k=dispatch_legacy?program_kind(out):program_kind_native(out,native);if(k){if(kind)*kind=k;return 1;}}errsay("command is not an executable command\n");return -1;}
 const char *p=env_get("PATH");while(path_entry(&p,out)){size_t n=strlen(out);if(n+strlen(name)+2>=PATH_CAP)continue;strcat(out,"/");strcat(out,name);if(resolve(out,native)||info(native,&pb))continue;if(!(pb.hFileInfo.ioFlAttrib&16)){int k=dispatch_legacy?program_kind(out):program_kind_native(out,native);if(k){if(kind)*kind=k;return 1;}}rejected=1;}
 if(rejected)errsay("PATH command found, but it is not an executable command\n");return rejected?-1:0;
}
static int stream_input_open(const char *path,short *ref){char native[PATH_CAP],canonical[PATH_CAP];unsigned char name[256];int e=resolve_ex(path,native,canonical,1);if(e)return e;int kind=vstream_kind(canonical);if(kind){*ref=-(kind*4+1);return 0;}pstr(name,native);return HOpen(0,0,name,fsRdPerm,ref);}
static int redirect(const char *s,int append){char p[PATH_CAP],canonical[PATH_CAP];unsigned char n[256];int e=resolve_ex(s,p,canonical,1);if(e)return e;int kind=vstream_kind(canonical);if(kind){output_ref=-(kind*4+2);return 0;}pstr(n,p);e=HOpen(0,0,n,fsWrPerm,&output_ref);if(e==fnfErr){e=HCreate(0,0,n,'S6UX','TEXT');if(!e)e=HOpen(0,0,n,fsWrPerm,&output_ref);}if(e){output_ref=-1;return e;}e=append?SetFPos(output_ref,fsFromLEOF,0):SetEOF(output_ref,0);if(e){FSClose(output_ref);output_ref=-1;}return e;}
/* Fixed-slot disk ring: 128 x 1028 byte records + header. A record is committed by writing its sequence last. */
#define HISTORY_SLOTS 128
#define HISTORY_SLOT (LINE_CAP+4)
static int hist_record(unsigned long seq,char *out,int writing){unsigned long stored=0;long n;OSErr e;long pos=8+(long)((seq-1)%HISTORY_SLOTS)*HISTORY_SLOT;e=writing?0:SetFPos(history_ref,fsFromStart,pos);if(e)return e;if(writing){long size=0;GetEOF(history_ref,&size);if(size<pos+HISTORY_SLOT){e=SetEOF(history_ref,pos+HISTORY_SLOT);if(e)return e;}e=SetFPos(history_ref,fsFromStart,pos);if(e)return e;n=4;e=FSWrite(history_ref,&n,&stored);if(e)return e;n=LINE_CAP;e=FSWrite(history_ref,&n,out);if(e)return e;SetFPos(history_ref,fsFromStart,pos);n=4;return FSWrite(history_ref,&n,&seq);}n=4;e=FSRead(history_ref,&n,&stored);if(e||stored!=seq)return -1;n=LINE_CAP;e=FSRead(history_ref,&n,out);out[LINE_CAP-1]=0;return e;}
static void history_add(const char *s){char record[LINE_CAP];long n;unsigned long head[2];if(history_ref<0||!history_enabled||!s[0]||s[0]==' ')return;memset(record,0,sizeof record);strncpy(record,s,sizeof(record)-1);if(hist_record(history_count+1,record,1)){term_puts("history: write failed; history disabled\n");history_enabled=0;return;}history_count++;head[0]=0x53364831;head[1]=history_count;SetFPos(history_ref,fsFromStart,0);n=8;if(FSWrite(history_ref,&n,head)){history_enabled=0;term_puts("history: header write failed\n");}}
int history_get(int back,char *out){if(history_ref<0||back<1||back>HISTORY_SLOTS||(unsigned long)back>history_count)return 0;return hist_record(history_count-(unsigned long)back+1,out,0)==0;}
int history_search(const char *needle,int start,char *out){int i;for(i=start;i<=HISTORY_SLOTS&&(unsigned long)i<=history_count;i++){if(history_get(i,out)&&asm_contains(out,needle))return i;if(app_yield())break;}return 0;}
void history_shutdown(void){if(history_ref>=0){FSClose(history_ref);history_ref=-1;FlushVol(0,0);}}
void shell_init(void){
 self_update_resources=CurResFile();unsigned char vol[256];short vr;WDPBRec pb;unsigned long head[2];long n=8;char path[PATH_CAP];unsigned char pn[256];
 GetVol(vol,&vr);memcpy(shell_cwd,vol+1,vol[0]);shell_cwd[vol[0]]=':';shell_cwd[vol[0]+1]=0;
 /* GetVol may return a working-directory reference; resolve its actual volume name. */
 memset(&pb,0,sizeof pb);pb.ioVRefNum=vr;pb.ioWDIndex=0;if(!PBGetWDInfoSync(&pb))vr=pb.ioWDVRefNum;
 {HParamBlockRec vp;memset(&vp,0,sizeof vp);vp.volumeParam.ioNamePtr=vol;vp.volumeParam.ioVRefNum=vr;if(!PBHGetVInfoSync(&vp)){memcpy(shell_cwd,vol+1,vol[0]);shell_cwd[vol[0]]=':';shell_cwd[vol[0]+1]=0;}}
 {int e=namespace_install();if(e)error("System Folder installation failed",e);}strcpy(shell_cwd,root_path);strcpy(home_path,root_path);migrate_reports();env_set("PATH=/bin:/usr/bin");shell_state_path(path,"clarice.history","Clarice History");pstr(pn,path);
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
#ifdef CLARICE_COMPLETION_DIAGNOSTICS
static int completion_probe(void);
#endif
static int selftest(void){
#ifdef CLARICE_COMPLETION_DIAGNOSTICS
return completion_probe();
#endif
unsigned long t=TickCount(),n;int fail=0;char b[2048],*av[MAX_ARGS+1],*dst;int ap;
 {static const char *samples[]={"","a","abc","abababa","0123456789abcdefghijk","odd address","xxxxxxxxxxxxxxxxxxxxxxxx"};unsigned int x,y;for(x=0;x<sizeof samples/sizeof samples[0];x++)for(y=0;y<sizeof samples/sizeof samples[0];y++){if(asm_length(samples[x])!=strlen(samples[x]))fail++;if(asm_contains(samples[x],samples[y])!=(strstr(samples[x],samples[y])!=0))fail++;}for(x=0;x<sizeof samples/sizeof samples[0];x++){char odd[80];strcpy(odd+1,samples[x]);if(asm_length(odd+1)!=strlen(samples[x]))fail++;}}
 {unsigned char src[14],got[56],ref[56];int x;for(x=0;x<14;x++)src[x]=(unsigned char)(x*19);memset(got,0x55,sizeof got);memset(ref,0x55,sizeof ref);for(x=0;x<14;x++)ref[1+x*4]=src[x];asm_glyph(got+1,src,4,14);if(memcmp(got,ref,sizeof got))fail++;}
 if(!asm_contains("abc","")||!asm_contains("abc","bc")||asm_contains("abc","abcd")||asm_contains("abc","z"))fail++;
 if(parse("echo 'a b' \"\" x\\ y",b,av,&dst,&ap)!=4||strcmp(av[1],"a b")||strcmp(av[2],"")||strcmp(av[3],"x y"))fail++;
 if(parse("echo 'bad",b,av,&dst,&ap)>=0||parse("cat a | cat",b,av,&dst,&ap)!=-2)fail++;
 {char line[LINE_CAP]="ec";int pos=2;if(shell_complete(line,&pos)!=1||strcmp(line,"echo ")||pos!=5)fail++;}
 for(n=0;n<10000;n++){if(!asm_contains("0123456789abcdefghijk","efgh"))fail++;if((n&255)==0)app_yield();}
 {char text[128];snprintf(text,sizeof text,"selftest: %s; 10000 scans, %lu ticks\n",fail?"FAIL":"PASS",(unsigned long)(TickCount()-t));say(text);}return fail?1:0;}
#include "port-files.inc"
#include "safe-save.inc"
#include "service.inc"
#include "module.inc"
#include "self-update.inc"
#include "help.inc"
static int run_args(int argc,char **av){int i,status=0;if(!argc)return 0;
 shell_commands++;
 if(session_io&&(!strcmp(av[0],"prefs")||!strcmp(av[0],"scrollbench")||!strcmp(av[0],"checks")||!strcmp(av[0],"selftest")||(!strcmp(av[0],"status")&&argc<2))){errsay("command requires the local graphical terminal\n");return 2;}
 if(!strcmp(av[0],"remote")){if(argc==2&&!strcmp(av[1],"off"))status=shell_service_stop()?1:0;else if(argc==2&&!strcmp(av[1],"status")){shell_service_status(0);shell_service_status(1);}else{say("remote status | off; start with telnetd --no-password --peer IPv4 [--port N]\n");status=argc==1?0:2;}}
 else if(!strcmp(av[0],"help"))status=help_command(argc,av);
 else if(!strcmp(av[0],"clarice"))status=self_update_command(argc,av);
 else if(!strcmp(av[0],"appinstall"))status=argc==2&&!strcmp(av[1],"--help")?builtin_help("appinstall"):appinstall_command(argc,av);
 else if(!strcmp(av[0],"pwd")){say(unix_cwd);say("\n");}
 else if(!strcmp(av[0],"echo")){for(i=1;i<argc;i++){if(i>1)say(" ");say(av[i]);}say("\n");}
 else if(!strcmp(av[0],"cd")){char path[PATH_CAP];short v;long id;const char *to=argc>1?av[1]:"~";int e=directory(to,&v,&id);char canonical[PATH_CAP];if(!e)e=resolve_ex(to,path,canonical,1);if(e){error("cd",e);status=1;}else {strcpy(unix_cwd,canonical);strcpy(shell_cwd,path);size_t len=strlen(shell_cwd);if(shell_cwd[len-1]!=':'){shell_cwd[len]=':';shell_cwd[len+1]=0;}}}

 else if(!strcmp(av[0],"export")){if(argc==1){for(struct sh_env *e=sh_env;e;e=e->next){say(e->name);say("=");say(e->value);say("\n");}}for(i=1;i<argc;i++)if(env_set(av[i])){errsay("export: invalid name/value or environment full\n");status=2;}}
 else if(!strcmp(av[0],"history")){if(argc>1&&!strcmp(av[1],"off"))history_enabled=0;else if(argc>1&&!strcmp(av[1],"on"))history_enabled=1;else if(argc>1&&!strcmp(av[1],"clear")){if(history_ref>=0&&!SetEOF(history_ref,0))history_count=0;else status=1;}else{char line[LINE_CAP];for(i=HISTORY_SLOTS;i>=1;i--)if(history_get(i,line)){say(line);say("\n");if(app_yield()){status=130;break;}}}}
 else if(!strcmp(av[0],"clear")){if(session_io)say("\033[2J\033[H");else app_clear();}
 else if(!strcmp(av[0],"status")&&argc==2&&!strcmp(av[1],"runtime")){char b[240];snprintf(b,sizeof b,"Runtime: heap=%ld largest=%ld image=%lu ticks=%lu load=%lu owned-peak=%lu remaining=%lu\n",FreeMem(),MaxBlock(),module_bytes,module_ticks,module_load_ticks,port_peak,port_remaining);say(b);snprintf(b,sizeof b,"Loader ticks: read=%lu validate=%lu imports=%lu relocations=%lu prepare=%lu events=%lu (events included above)\n",module_read_ticks,module_validate_ticks,module_import_ticks,module_reloc_ticks,module_prepare_ticks,module_poll_ticks);
#ifdef CLARICE_NANO_PERF
 {char perf[900];app_nano_stats(perf,sizeof perf);say(perf);snprintf(perf,sizeof perf,"Native ticks: allocs=%lu FreeMem=%lu NewPtr=%lu reads=%lu read=%lu stdio=%lu image-alloc=%lu meta-alloc=%lu\n",perf_allocs,perf_free_ticks,perf_new_ticks,perf_reads,perf_read_ticks,perf_stdio_ticks,perf_image_ticks,perf_meta_ticks);say(perf);}
#endif
say(b);snprintf(b,sizeof b,"Verification ticks: mode=%s header=%lu record=%lu payload=%lu metadata=%lu structure=%lu (structure included in relocations)\n",module_fast?"fast":"full",module_header_ticks,module_record_ticks,module_payload_ticks,module_metadata_ticks,module_structure_ticks);say(b);}
 else if(!strcmp(av[0],"status")){if(argc>2||(argc==2&&strcmp(av[1],"-h"))){say("usage: status [-h]\n");status=2;}else app_metrics(argc==2);}
 else if(!strcmp(av[0],"scrollbench"))status=app_scroll_bench(
#ifdef CLARICE_NANO_PERF
 argc>1&&!strcmp(av[1],"editor-old")?90:argc>1&&!strcmp(av[1],"editor-fast")?91:argc>1&&!strcmp(av[1],"editorstats")?92:argc>1&&!strcmp(av[1],"editorprobe")?93:argc>1&&!strcmp(av[1],"editor-rows")?94:argc>1&&!strcmp(av[1],"editor-native")?95:
#endif
#ifdef CLARICE_DRAW_DIAGNOSTICS
 argc>1&&!strcmp(av[1],"arrows")?20:argc>1&&!strcmp(av[1],"live")?19:argc>1&&!strcmp(av[1],"memory")?18:argc>1&&!strcmp(av[1],"stagepixels")?17:argc>1&&!strcmp(av[1],"directpixels")?16:argc>1&&!strcmp(av[1],"direct")?15:argc>1&&!strcmp(av[1],"alignpixels")?14:argc>1&&!strcmp(av[1],"alignment")?13:argc>1&&!strcmp(av[1],"present")?12:argc>1&&!strcmp(av[1],"clearing")?11:argc>1&&!strcmp(av[1],"glyphpixels")?10:argc>1&&!strcmp(av[1],"jumps")?9:argc>1&&!strcmp(av[1],"frames")?8:argc>1&&!strcmp(av[1],"listing")?7:argc>1&&!strcmp(av[1],"catalog")?6:argc>1&&!strcmp(av[1],"dispatch")?5:argc>1&&!strcmp(av[1],"profile")?4:
#endif
 argc>1&&!strcmp(av[1],"cached")?3:argc>1&&!strcmp(av[1],"native")?2:argc>1&&!strcmp(av[1],"quick"));
 else if(!strcmp(av[0],"prefs"))status=app_preferences(argc,av);
 else if(!strcmp(av[0],"checks"))app_run_checks();
 else if(!strcmp(av[0],"selftest"))status=selftest();
 else if(!strcmp(av[0],"exit"))app_quit();
 else {say("command not found: ");say(av[0]);say("\n");command_install_hint(av[0]);status=127;}
 if(output_error)status=1;shell_status=status;return status;
}
#include "completion.inc"

#ifdef CLARICE_DRAW_DIAGNOSTICS
void shell_dispatch_stats(unsigned long *out){out[0]=dispatch_path_ticks;out[1]=dispatch_kind_ticks;out[2]=module_load_ticks;out[3]=module_ticks;}
#endif
#include "sh.inc"
#include "session.inc"

static void complete_functions(Completion *c){int n=0;for(struct sh_function *f=sh_functions;f&&!c->stop;f=f->next){if(!(n++&7)&&app_completion_poll()){c->stop=2;break;}if(!builtin_name(f->name))completion_item(c,f->name);}}

#ifdef CLARICE_OFFLINE_INSTALLER
#include "offline-install.inc"
#endif

#ifdef CLARICE_COMPLETION_DIAGNOSTICS
#include "../tests/completion/guest.inc"
#endif
