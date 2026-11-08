/* Clarice shell execution adapter. The parser/AST in vendor/mrsh is MIT
 * licensed, copyright 2018 emersion. This adapter is original project code.
 * Fixed limits are deliberate on the 4 MB baseline. See SHELL-COMPATIBILITY. */
#include <mrsh/parser.h>
#include <mrsh/arithm.h>
#define SH_TEXT 8192
#define SH_DEPTH 16
static int sh_source_depth,sh_depth,sh_flow,sh_flow_status,sh_loop,sh_functions_active;
static int sh_argc=1;
static char *sh_owned_args;
static char *sh_argv[MAX_ARGS+1]={"sh"};
static struct {char name[32];struct mrsh_command *body;} sh_functions[16];
static int sh_array(struct mrsh_array *a);
static int sh_command(struct mrsh_command *cmd);
static int sh_source(const char *source);
static int sh_fail(const char *s){errsay("sh: ");errsay(s);errsay("\n");return 2;}
static int sh_set(const char *name,const char *value){char s[164];if(strlen(name)>31||strlen(value)>127)return sh_fail("variable exceeds limit");snprintf(s,sizeof s,"%s=%s",name,value);return env_set(s)?sh_fail("variable table full or invalid name"):0;}
static const char *sh_get(const char *name,char *num){if(!strcmp(name,"?")){snprintf(num,32,"%d",shell_status);return num;}if(!strcmp(name,"#")){snprintf(num,32,"%d",sh_argc-1);return num;}if(isdigit((unsigned char)name[0])){int i=atoi(name);return i<sh_argc?sh_argv[i]:"";}return env_get(name);}
static int sh_isset(const char *name){int i;if(!strcmp(name,"HOME")||!strcmp(name,"PWD")||!strcmp(name,"?")||!strcmp(name,"#"))return 1;if(isdigit((unsigned char)*name))return atoi(name)<sh_argc;for(i=0;i<16;i++)if(!strcmp(env_names[i],name))return 1;return 0;}
static int sh_pattern(const char *p,const char *s){while(*p){if(*p=='*'){while(*p=='*')p++;if(!*p)return 1;do{if(sh_pattern(p,s))return 1;}while(*s++);return 0;}if(*p=='?'){if(!*s)return 0;p++;s++;continue;}if(*p=='['){int yes=0,neg=0;unsigned char c=*s++;if(!c)return 0;p++;if(*p=='!'||*p=='^'){neg=1;p++;}while(*p&&*p!=']'){unsigned char lo=*p++,hi=lo;if(*p=='-'&&p[1]&&p[1]!=']'){p++;hi=*p++;}if(c>=lo&&c<=hi)yes=1;}if(*p!=']'||yes==neg)return 0;p++;continue;}if(*p=='\\'&&p[1])p++;if(*p++!=*s++)return 0;}return !*s;}
struct sh_saved {char names[16][32],values[16][128],cwd[PATH_CAP];
#ifndef SH_HOST_TEST
char unixdir[PATH_CAP];
#endif
char function_names[16][32];struct mrsh_command *functions[16];};
static struct sh_saved *sh_save(void){
#ifndef SH_HOST_TEST
 if(FreeMem()<65536)return 0;
#endif
 struct sh_saved *s=malloc(sizeof *s);if(!s)return 0;memcpy(s->names,env_names,sizeof env_names);memcpy(s->values,env_values,sizeof env_values);strcpy(s->cwd,shell_cwd);
#ifndef SH_HOST_TEST
strcpy(s->unixdir,unix_cwd);
#endif
for(int i=0;i<16;i++){strcpy(s->function_names[i],sh_functions[i].name);s->functions[i]=sh_functions[i].body?mrsh_command_copy(sh_functions[i].body):0;}return s;}
static void sh_restore(struct sh_saved *s){memcpy(env_names,s->names,sizeof env_names);memcpy(env_values,s->values,sizeof env_values);strcpy(shell_cwd,s->cwd);
#ifndef SH_HOST_TEST
strcpy(unix_cwd,s->unixdir);
#endif
for(int i=0;i<16;i++){mrsh_command_destroy(sh_functions[i].body);strcpy(sh_functions[i].name,s->function_names[i]);sh_functions[i].body=s->functions[i];}free(s);}
static unsigned long sh_temp_serial;
static int sh_temp(short *ref,unsigned char *name){char path[PATH_CAP];int e,i;for(i=0;i<100;i++){snprintf(path,sizeof path,"%.470sClarice temp %lu",home_path,++sh_temp_serial);pstr(name,path);e=HCreate(0,0,name,'S6UX','TEXT');if(!e){e=HOpen(0,0,name,fsRdWrPerm,ref);if(e)HDelete(0,0,name);return e;}if(e!=dupFNErr)return e;}return dupFNErr;}
static void sh_temp_close(short ref,unsigned char *name){if(ref>=0)FSClose(ref);HDelete(0,0,name);}
static int sh_arith(struct mrsh_arithm_expr *e,long *v){long a,b;char num[32];if(!e)return 0;switch(e->type){
case MRSH_ARITHM_LITERAL:*v=((struct mrsh_arithm_literal*)e)->value;return 1;
case MRSH_ARITHM_VARIABLE:*v=strtol(env_get(((struct mrsh_arithm_variable*)e)->name),0,0);return 1;
case MRSH_ARITHM_UNOP:{struct mrsh_arithm_unop *u=(void*)e;if(!sh_arith(u->body,&a))return 0;*v=u->type==MRSH_ARITHM_UNOP_PLUS?a:u->type==MRSH_ARITHM_UNOP_MINUS?(long)(0UL-(unsigned long)a):u->type==MRSH_ARITHM_UNOP_TILDE?~a:!a;return 1;}
case MRSH_ARITHM_COND:{struct mrsh_arithm_cond *c=(void*)e;return sh_arith(c->condition,&a)&&sh_arith(a?c->body:c->else_part,v);}
case MRSH_ARITHM_ASSIGN:{struct mrsh_arithm_assign *x=(void*)e;if(x->op!=MRSH_ARITHM_ASSIGN_NONE)return 0;if(!sh_arith(x->value,v))return 0;snprintf(num,sizeof num,"%ld",*v);return !sh_set(x->name,num);}
case MRSH_ARITHM_BINOP:{struct mrsh_arithm_binop *x=(void*)e;if(!sh_arith(x->left,&a))return 0;if(x->type==MRSH_ARITHM_BINOP_DAND&&!a){*v=0;return 1;}if(x->type==MRSH_ARITHM_BINOP_DOR&&a){*v=1;return 1;}if(!sh_arith(x->right,&b))return 0;switch(x->type){
case MRSH_ARITHM_BINOP_ASTERISK:*v=(unsigned long)a*(unsigned long)b;break;
case MRSH_ARITHM_BINOP_SLASH:if(!b||(a==(-2147483647L-1)&&b==-1))return 0;*v=a/b;break;
case MRSH_ARITHM_BINOP_PERCENT:if(!b||(a==(-2147483647L-1)&&b==-1))return 0;*v=a%b;break;
case MRSH_ARITHM_BINOP_PLUS:*v=(unsigned long)a+(unsigned long)b;break;
case MRSH_ARITHM_BINOP_MINUS:*v=(unsigned long)a-(unsigned long)b;break;
case MRSH_ARITHM_BINOP_DLESS:if(b<0||b>31)return 0;*v=(unsigned long)a<<b;break;
case MRSH_ARITHM_BINOP_DGREAT:if(b<0||b>31)return 0;*v=a>>b;break;
case MRSH_ARITHM_BINOP_LESS:*v=a<b;break;case MRSH_ARITHM_BINOP_LESSEQ:*v=a<=b;break;
case MRSH_ARITHM_BINOP_GREAT:*v=a>b;break;case MRSH_ARITHM_BINOP_GREATEQ:*v=a>=b;break;
case MRSH_ARITHM_BINOP_DEQ:*v=a==b;break;case MRSH_ARITHM_BINOP_BANGEQ:*v=a!=b;break;
case MRSH_ARITHM_BINOP_AND:*v=a&b;break;case MRSH_ARITHM_BINOP_OR:*v=a|b;break;case MRSH_ARITHM_BINOP_CIRC:*v=a^b;break;
case MRSH_ARITHM_BINOP_DAND:*v=!!b;break;case MRSH_ARITHM_BINOP_DOR:*v=!!b;break;}return 1;}}
return 0;}
/* Each byte carries quoting/field-splitting information until arguments form. */
struct sh_word {char text[2048];unsigned char mode[2048];int used,quoted;};
static int sh_append(struct sh_word *w,const char *s,int mode){int n=strlen(s);if(n+w->used>2047)return sh_fail("expanded word exceeds 2047 bytes");memcpy(w->text+w->used,s,n);memset(w->mode+w->used,mode,n);w->used+=n;w->text[w->used]=0;if(mode==1)w->quoted=1;return 0;}
static int sh_word(struct mrsh_word *node,struct sh_word *w,int quoted){size_t i;char num[32];if(!node)return 0;switch(node->type){
case MRSH_WORD_STRING:{struct mrsh_word_string *s=(void*)node;return sh_append(w,s->str,quoted||s->single_quoted?1:0);}
case MRSH_WORD_LIST:{struct mrsh_word_list *l=(void*)node;if(l->double_quoted)w->quoted=1;for(i=0;i<l->children.len;i++)if(sh_word(l->children.data[i],w,quoted||l->double_quoted))return 2;return 0;}
case MRSH_WORD_PARAMETER:{struct mrsh_word_parameter *p=(void*)node;const char *value=sh_get(p->name,num);int unset=!sh_isset(p->name)||(p->colon&&!*value);struct sh_word *arg;
 if(quoted&&!strcmp(p->name,"@"))return sh_fail("embedded quoted $@ is not implemented; use a separate \"$@\" argument");
 if(!strcmp(p->name,"@")||!strcmp(p->name,"*")){for(i=1;i<(size_t)sh_argc;i++){if(i>1&&sh_append(w," ",quoted?1:2))return 2;if(sh_append(w,sh_argv[i],quoted?1:2))return 2;}return 0;}
 if(p->op==MRSH_PARAM_LEADING_HASH){snprintf(num,sizeof num,"%lu",(unsigned long)strlen(value));value=num;}
 if(p->op==MRSH_PARAM_NONE||p->op==MRSH_PARAM_LEADING_HASH)return sh_append(w,value,quoted?1:2);
 if(p->op==MRSH_PARAM_MINUS&&!unset)return sh_append(w,value,quoted?1:2);
 if(p->op==MRSH_PARAM_EQUAL&&!unset)return sh_append(w,value,quoted?1:2);
 if(p->op==MRSH_PARAM_QMARK&&!unset)return sh_append(w,value,quoted?1:2);
 if(p->op==MRSH_PARAM_PLUS&&unset)return 0;
 arg=calloc(1,sizeof *arg);if(!arg)return sh_fail("out of memory");int rc=sh_word(p->arg,arg,quoted);
 if(!rc){if(p->op==MRSH_PARAM_QMARK)rc=sh_fail(arg->used?arg->text:"unset parameter");
 else if(p->op==MRSH_PARAM_EQUAL){rc=sh_set(p->name,arg->text);if(!rc)rc=sh_append(w,arg->text,quoted?1:2);}
 else if(p->op==MRSH_PARAM_MINUS||p->op==MRSH_PARAM_PLUS)rc=sh_append(w,arg->text,quoted?1:2);
 else {char copy[2048];size_t n=strlen(value),j;int match=-1;if(n>=sizeof copy)rc=2;else{strcpy(copy,value);for(j=0;j<=n;j++){char save=copy[j];int yes;if(p->op==MRSH_PARAM_HASH||p->op==MRSH_PARAM_DHASH){copy[j]=0;yes=sh_pattern(arg->text,copy);copy[j]=save;}else yes=sh_pattern(arg->text,copy+j);if(yes){match=j;if(p->op==MRSH_PARAM_HASH||p->op==MRSH_PARAM_DPERCENT)break;}}if(match>=0){if(p->op==MRSH_PARAM_HASH||p->op==MRSH_PARAM_DHASH)memmove(copy,copy+match,n-match+1);else copy[match]=0;}rc=sh_append(w,copy,quoted?1:2);}}}free(arg);return rc;}
case MRSH_WORD_ARITHMETIC:{struct sh_word *arg=calloc(1,sizeof *arg);long value;int rc;if(!arg)return sh_fail("out of memory");rc=sh_word(((struct mrsh_word_arithmetic*)node)->body,arg,1);if(!rc){struct mrsh_parser *p=mrsh_parser_with_data(arg->text,arg->used);struct mrsh_arithm_expr *e=mrsh_parse_arithm_expr(p);if(!e||mrsh_parser_error(p,0)||!sh_arith(e,&value))rc=sh_fail(mrsh_parser_error(p,0)?mrsh_parser_error(p,0):"unsupported or invalid arithmetic");else{snprintf(num,sizeof num,"%ld",value);rc=sh_append(w,num,quoted?1:2);}mrsh_arithm_expr_destroy(e);mrsh_parser_destroy(p);}free(arg);return rc;}
case MRSH_WORD_COMMAND:{struct mrsh_word_command *c=(void*)node;short ref,old=output_ref;unsigned char name[256];struct sh_saved *saved=sh_save();char value[2048];long n=sizeof value-1;int rc,flow=sh_flow;if(!saved)return sh_fail("out of memory");if(sh_temp(&ref,name)){sh_restore(saved);return sh_fail("cannot create substitution spool");}output_ref=ref;script_depth++;rc=c->program?sh_array(&c->program->body):0;script_depth--;output_ref=old;sh_restore(saved);sh_flow=flow;SetFPos(ref,fsFromStart,0);int e=FSRead(ref,&n,value);long size=0;GetEOF(ref,&size);sh_temp_close(ref,name);if((e&&e!=eofErr)||size>2047)return sh_fail("command substitution exceeds 2047 bytes");while(n&&(value[n-1]=='\n'||value[n-1]=='\r'))n--;value[n]=0;shell_status=rc;return sh_append(w,value,quoted?1:2);}}
return sh_fail("unknown word");}
struct sh_args {char data[4096];char *v[MAX_ARGS+1];int used,n;};
static int sh_arg(struct sh_args *a,const char *s){int n=strlen(s)+1;if(a->n>=MAX_ARGS||a->used+n>(int)sizeof a->data)return sh_fail("argument limit exceeded");a->v[a->n++]=a->data+a->used;memcpy(a->data+a->used,s,n);a->used+=n;a->v[a->n]=0;return 0;}
static int sh_glob(struct sh_args *a,char *s,unsigned char *mode){int i,has=0,start=a->n;for(i=0;s[i];i++)if(mode[i]!=1&&strchr("*?[",s[i]))has=1;if(!has)return sh_arg(a,s);
#ifdef SH_HOST_TEST
 return sh_arg(a,s);
#else
 char path[PATH_CAP],pattern[PATH_CAP];char *slash=strrchr(s,'/');short vol;long id;int index=1,e;size_t prefix=slash?(size_t)(slash-s)+1:0;
 if(prefix>=sizeof path)return 2;if(prefix){memcpy(path,s,prefix);path[prefix]=0;for(i=0;i<(int)prefix;i++)if(strchr("*?[",path[i]))return sh_fail("wildcards in parent directories are not implemented");}else strcpy(path,".");
 int used=0;for(i=(int)prefix;s[i];i++){if(used>PATH_CAP-3)return 2;if(mode[i]==1&&strchr("*?[\\",s[i]))pattern[used++]='\\';pattern[used++]=s[i];}pattern[used]=0;
 e=directory(path,&vol,&id);if(e)return sh_arg(a,s);
 for(;;){CInfoPBRec pb;unsigned char name[256];char candidate[PATH_CAP];memset(&pb,0,sizeof pb);pb.hFileInfo.ioNamePtr=name;pb.hFileInfo.ioVRefNum=vol;pb.hFileInfo.ioDirID=id;pb.hFileInfo.ioFDirIndex=index++;e=PBGetCatInfoSync(&pb);if(e==fnfErr)break;if(e)return sh_fail("glob directory read failed");name[name[0]+1]=0;if(name[1]=='.'&&pattern[0]!='.')continue;if(!sh_pattern(pattern,(char*)name+1))continue;memcpy(candidate,s,prefix);int n=prefix,j;for(j=1;j<=name[0];j++){unsigned char c=name[j];if(c=='/'||c=='%'){candidate[n++]='%';candidate[n++]='2';candidate[n++]=c=='/'?'F':'5';}else candidate[n++]=c;}candidate[n]=0;if(sh_arg(a,candidate))return 2;if(app_yield())return 130;}
 if(a->n==start)return sh_arg(a,s);for(i=start+1;i<a->n;i++){int j=i;char *value=a->v[i];while(j>start&&strcmp(a->v[j-1],value)>0){a->v[j]=a->v[j-1];j--;}a->v[j]=value;}return 0;
#endif
}
static int sh_expand(struct mrsh_word *node,struct sh_args *a){struct sh_word *w=calloc(1,sizeof *w);int rc,i,start;const char *ifs=sh_isset("IFS")?env_get("IFS"):" \t\n";if(!w)return sh_fail("out of memory");
 /* Preserve separate positional arguments for the common quoted "$@" form. */
 if(node&&node->type==MRSH_WORD_LIST){struct mrsh_word_list *l=(void*)node;if(l->double_quoted&&l->children.len==1){struct mrsh_word *c=l->children.data[0];if(c->type==MRSH_WORD_PARAMETER&&!strcmp(((struct mrsh_word_parameter*)c)->name,"@")){free(w);for(i=1;i<sh_argc;i++)if(sh_arg(a,sh_argv[i]))return 2;return 0;}}}
 rc=sh_word(node,w,0);if(!rc&&w->used&&w->text[0]=='~'&&w->mode[0]!=1&&(w->text[1]=='/'||!w->text[1])){char expanded[2048];const char *tail=w->text+1;if(*tail=='/')tail++;
#ifdef SH_HOST_TEST
 snprintf(expanded,sizeof expanded,"native:%s%s",home_path,tail);
#else
 snprintf(expanded,sizeof expanded,"/%s",tail);
#endif
if(strlen(expanded)>=sizeof w->text)rc=2;else{strcpy(w->text,expanded);w->used=strlen(expanded);memset(w->mode,1,w->used);}}
 if(!rc){if(!w->used&&w->quoted)rc=sh_arg(a,"");for(i=0;i<w->used&&!rc;){while(i<w->used&&w->mode[i]==2&&strchr(ifs,w->text[i]))i++;start=i;while(i<w->used&&!(w->mode[i]==2&&strchr(ifs,w->text[i])))i++;if(i>start){char save=w->text[i];w->text[i]=0;rc=sh_glob(a,w->text+start,w->mode+start);w->text[i]=save;}}}free(w);return rc;}
static int sh_test(int n,char **v){
#ifndef SH_HOST_TEST
 if(n==2&&(!strcmp(v[0],"-L")||!strcmp(v[0],"-h"))){char p[PATH_CAP];CInfoPBRec pb;return resolve_ex(v[1],p,0,0)||info(p,&pb)||!is_link(&pb);}
#endif
if(n==0)return 1;if(n==1)return !*v[0];if(n==2){if(!strcmp(v[0],"!"))return !sh_test(1,v+1);if(!strcmp(v[0],"-n"))return !*v[1];if(!strcmp(v[0],"-z"))return !!*v[1];if(!strcmp(v[0],"-e")||!strcmp(v[0],"-f")||!strcmp(v[0],"-d")||!strcmp(v[0],"-s")){char p[PATH_CAP];CInfoPBRec pb;if(resolve(v[1],p)||info(p,&pb))return 1;int dir=!!(pb.hFileInfo.ioFlAttrib&16);return v[0][1]=='e'?0:v[0][1]=='d'?!dir:v[0][1]=='f'?dir:!pb.hFileInfo.ioFlLgLen;}}
 if(n==3){if(!strcmp(v[1],"="))return !!strcmp(v[0],v[2]);if(!strcmp(v[1],"!="))return !strcmp(v[0],v[2]);char *end1,*end2;long a=strtol(v[0],&end1,10),b=strtol(v[2],&end2,10);if(*end1||*end2)return 2;if(!strcmp(v[1],"-eq"))return a!=b;if(!strcmp(v[1],"-ne"))return a==b;if(!strcmp(v[1],"-lt"))return a>=b;if(!strcmp(v[1],"-le"))return a>b;if(!strcmp(v[1],"-gt"))return a<=b;if(!strcmp(v[1],"-ge"))return a<b;}return sh_fail("unsupported test expression");}
static int sh_file(const char *path,int argc,char **argv,int dot){short ref;long n,size;char *text;int rc,i,oldargc=sh_argc,oldflow=sh_flow;char *owned=sh_owned_args;char *oldargv[MAX_ARGS+1];struct sh_saved *saved=dot?0:sh_save();if(!dot&&!saved)return sh_fail("out of memory");if(open_path(path,fsRdPerm,&ref)){if(saved)sh_restore(saved);return sh_fail("cannot open script");}GetEOF(ref,&size);if(size>SH_TEXT||size<0){FSClose(ref);if(saved)sh_restore(saved);return sh_fail("script exceeds 8 KiB limit");}text=malloc(size+1);if(!text){FSClose(ref);if(saved)sh_restore(saved);return sh_fail("out of memory");}n=size;rc=FSRead(ref,&n,text);FSClose(ref);if(rc&&rc!=eofErr){free(text);if(saved)sh_restore(saved);return sh_fail("script read failed");}text[n]=0;for(i=0;i<n;i++)if(text[i]=='\r')text[i]='\n';memcpy(oldargv,sh_argv,sizeof oldargv);if(!dot||argc>1){sh_argc=argc;memcpy(sh_argv,argv,argc*sizeof *argv);}sh_owned_args=0;script_depth++;sh_flow=0;rc=sh_source(text);script_depth--;free(sh_owned_args);sh_owned_args=owned;sh_flow=oldflow;sh_argc=oldargc;memcpy(sh_argv,oldargv,sizeof oldargv);free(text);if(saved)sh_restore(saved);return rc;}
static int sh_dispatch(struct sh_args *a){int i,n=a->n;char **v=a->v;if(!n)return 0;
 if(!strcmp(v[0],":")||!strcmp(v[0],"true"))return 0;if(!strcmp(v[0],"false"))return 1;
 if(!strcmp(v[0],"test")||!strcmp(v[0],"[")){if(v[0][0]=='['){if(n<2||strcmp(v[n-1],"]"))return sh_fail("missing ]");n--;}return sh_test(n-1,v+1);}
 if(!strcmp(v[0],"break")||!strcmp(v[0],"continue")){if(!sh_loop||n>1)return sh_fail("break/continue requires a loop; numeric levels unsupported");sh_flow=v[0][0]=='b'?1:2;return 0;}
 if(!strcmp(v[0],"return")||!strcmp(v[0],"exit")){int rc=n>1?atoi(v[1]):shell_status;if(!script_depth&&!sh_functions_active&&v[0][0]=='r')return sh_fail("return outside script/function");if(!script_depth&&!sh_functions_active&&v[0][0]=='e'){app_quit();return rc;}sh_flow=v[0][0]=='r'?3:4;sh_flow_status=rc;return rc;}
 if(!strcmp(v[0],"unset")){for(i=1;i<n;i++){int j;for(j=0;j<16;j++)if(!strcmp(env_names[j],v[i]))env_names[j][0]=env_values[j][0]=0;}return 0;}
 if(!strcmp(v[0],"shift")){int count=n>1?atoi(v[1]):1;if(count<0||count>=sh_argc)return 1;memmove(sh_argv+1,sh_argv+1+count,(sh_argc-count-1)*sizeof(char*));sh_argc-=count;return 0;}
 if(!strcmp(v[0],"set")){if(n==1){for(i=0;i<16;i++)if(env_names[i][0]){say(env_names[i]);say("=");say(env_values[i]);say("\n");}return 0;}if(n>1&&strcmp(v[1],"--"))return sh_fail("only set -- is implemented");char *storage=malloc(4096);int used=0;if(!storage)return sh_fail("out of memory");for(i=2;i<n;i++){int len=strlen(v[i])+1;if(used+len>4096){free(storage);return 2;}memcpy(storage+used,v[i],len);sh_argv[i-1]=storage+used;used+=len;}free(sh_owned_args);sh_owned_args=storage;sh_argc=n>1?n-1:1;return 0;}
 if(!strcmp(v[0],"printf")){if(n<2)return 0;const char *f=v[1];int arg=2;do{int before=arg;const char *p=f;while(*p){char c=*p++;if(c=='\\'){c=*p++;if(!c)break;c=c=='n'?'\n':c=='t'?'\t':c=='r'?'\r':c;emit(&c,1);}else if(c=='%'){c=*p++;if(c=='%')say("%");else if(c=='s')say(arg<n?v[arg++]:"");else if(c=='d'){char b[32];snprintf(b,sizeof b,"%ld",arg<n?strtol(v[arg++],0,0):0L);say(b);}else return sh_fail("printf supports %s, %d and %%");}else emit(&c,1);}if(arg==before)break;}while(arg<n);return output_error?1:0;}
 if(!strcmp(v[0],"read")){char line[128];int used=0;long count;char c;int e;if(n!=2)return sh_fail("read requires one variable");while(used<127){count=cli_read(0,&c,1);e=count<0;if(count<=0)break;if(c=='\n'||c=='\r')break;line[used++]=c;if(e)break;}line[used]=0;sh_set(v[1],line);return count>0?0:1;}
 if(!strcmp(v[0],"sh")){if(n<2)return sh_fail("usage: sh file [args] or sh -c command");if(!strcmp(v[1],"-c")){if(n<3)return 2;struct sh_saved *saved=sh_save();int oldflow=sh_flow;if(!saved)return 2;script_depth++;sh_flow=0;int rc=sh_source(v[2]);script_depth--;sh_restore(saved);sh_flow=oldflow;return rc;}return sh_file(v[1],n-1,v+1,0);}
 if(!strcmp(v[0],".")){if(n<2)return 2;return sh_file(v[1],n-1,v+1,1);}
 for(i=0;i<16;i++)if(!strcmp(sh_functions[i].name,v[0])){char *saved[MAX_ARGS+1];int count=sh_argc,rc;char *owned=sh_owned_args;sh_owned_args=0;memcpy(saved,sh_argv,sizeof saved);sh_argc=n;memcpy(sh_argv,v,n*sizeof(char*));struct mrsh_command *body=mrsh_command_copy(sh_functions[i].body);sh_functions_active++;rc=body?sh_command(body):2;sh_functions_active--;mrsh_command_destroy(body);free(sh_owned_args);sh_owned_args=owned;sh_argc=count;memcpy(sh_argv,saved,sizeof saved);if(sh_flow==3)sh_flow=0;return rc;}

#ifdef SH_HOST_TEST
 size_t len=strlen(v[0]);if((len>3&&!strcmp(v[0]+len-3,".sh"))||strchr(v[0],'/'))return sh_file(v[0],n,v,0);
 return run_args(n,v);}
#else
 for(i=0;commands[i];i++)if(!strcmp(commands[i],v[0]))return run_args(n,v);
 char candidate[PATH_CAP];int found=command_path(v[0],candidate);
 if(found<0)return 126;if(found){shell_commands++;return program_kind(candidate)==2?module_run(candidate,n,v):sh_file(candidate,n,v,0);}
 errsay("command not found: ");errsay(v[0]);errsay("\n");return 127;}
#endif

static int sh_simple(struct mrsh_simple_command *c){struct sh_args *a=calloc(1,sizeof *a);size_t i;short oldin=input_ref,oldout=output_ref,olderr=error_ref;short opened[16];unsigned char temp_names[8][256];short temp_refs[8];int temp_count=0,opened_count=0,rc=0;struct sh_saved *saved=0;if(!a)return sh_fail("out of memory");
 output_error=0;
 if(c->name)rc=sh_expand(c->name,a);for(i=0;!rc&&i<c->arguments.len;i++)rc=sh_expand(c->arguments.data[i],a);
 if(a->n&&c->assignments.len){saved=sh_save();if(!saved)rc=sh_fail("out of memory");}
 for(i=0;!rc&&i<c->assignments.len;i++){struct mrsh_assignment *x=c->assignments.data[i];struct sh_word *w=calloc(1,sizeof *w);if(!w){rc=2;break;}rc=sh_word(x->value,w,1);if(!rc)rc=sh_set(x->name,w->text);free(w);}
 for(i=0;!rc&&i<c->io_redirects.len;i++){struct mrsh_io_redirect *r=c->io_redirects.data[i];struct sh_word *w=calloc(1,sizeof *w);if(!w){rc=2;break;}rc=sh_word(r->name,w,1);if(!rc){if(opened_count>=16)rc=sh_fail("too many redirections");else if(r->op==MRSH_IO_LESS&&(r->io_number<0||r->io_number==0)){short ref;if(open_path(w->text,fsRdPerm,&ref))rc=sh_fail("input redirection failed");else{input_ref=ref;opened[opened_count++]=ref;}}
 else if((r->op==MRSH_IO_GREAT||r->op==MRSH_IO_DGREAT||r->op==MRSH_IO_CLOBBER)&&(r->io_number<0||r->io_number==1||r->io_number==2)){short prior=output_ref;int e=redirect(w->text,r->op==MRSH_IO_DGREAT);short ref=output_ref;output_ref=prior;if(e)rc=sh_fail("output redirection failed");else{opened[opened_count++]=ref;if(r->io_number==2)error_ref=ref;else output_ref=ref;}}
 else if((r->op==MRSH_IO_DLESS||r->op==MRSH_IO_DLESSDASH)&&(r->io_number<0||r->io_number==0)){
 if(temp_count==8)rc=sh_fail("too many here-documents");else{short ref;if(sh_temp(&ref,temp_names[temp_count]))rc=sh_fail("cannot create here-document");else{size_t j;temp_refs[temp_count++]=ref;short prior=output_ref;output_ref=ref;for(j=0;j<r->here_document.len&&!rc;j++){memset(w,0,sizeof *w);rc=sh_word(r->here_document.data[j],w,1);if(!rc){emit(w->text,w->used);say("\n");if(output_error)rc=1;}}output_ref=prior;SetFPos(ref,fsFromStart,0);input_ref=ref;}}}
 else if(r->op==MRSH_IO_GREATAND||r->op==MRSH_IO_LESSAND){int to=r->io_number<0?(r->op==MRSH_IO_GREATAND?1:0):r->io_number;short ref=-2;
 if(to<0||to>2)rc=sh_fail("descriptor must be 0, 1 or 2");
 else if(strcmp(w->text,"-")){if(w->text[1]||w->text[0]<'0'||w->text[0]>'2')rc=sh_fail("invalid descriptor duplication");else{int from=w->text[0]-'0';ref=from==0?input_ref:from==1?output_ref:error_ref;if(ref==-2){sh_fail("source descriptor is closed");rc=1;}}}
 if(!rc){if(to==0)input_ref=ref;else if(to==1)output_ref=ref;else error_ref=ref;}} 
 else rc=sh_fail("this redirection is not supported yet");}free(w);}
 if(!rc)rc=sh_dispatch(a);if(output_error&&!rc)rc=1;while(opened_count)if(FSClose(opened[--opened_count])&&!rc)rc=1;while(temp_count){temp_count--;sh_temp_close(temp_refs[temp_count],temp_names[temp_count]);}input_ref=oldin;output_ref=oldout;error_ref=olderr;if(saved){memcpy(saved->cwd,shell_cwd,PATH_CAP);
#ifndef SH_HOST_TEST
strcpy(saved->unixdir,unix_cwd);
#endif
sh_restore(saved);}free(a);return rc;}
static int sh_command(struct mrsh_command *cmd){int rc=0;size_t i;if(++sh_depth>SH_DEPTH){sh_depth--;return sh_fail("execution nesting limit");}if(app_yield()){sh_depth--;sh_flow=4;sh_flow_status=130;return 130;}switch(cmd->type){
case MRSH_SIMPLE_COMMAND:rc=sh_simple((void*)cmd);break;
case MRSH_BRACE_GROUP:rc=sh_array(&((struct mrsh_brace_group*)cmd)->body);break;
case MRSH_SUBSHELL:{struct sh_saved *s=sh_save();int flow=sh_flow;if(!s){rc=2;break;}script_depth++;rc=sh_array(&((struct mrsh_subshell*)cmd)->body);script_depth--;sh_restore(s);sh_flow=flow;break;}
case MRSH_IF_CLAUSE:{struct mrsh_if_clause *c=(void*)cmd;rc=sh_array(&c->condition);if(!sh_flow){if(!rc)rc=sh_array(&c->body);else rc=c->else_part?sh_command(c->else_part):0;}break;}
case MRSH_FOR_CLAUSE:{struct mrsh_for_clause *c=(void*)cmd;struct sh_args *a=calloc(1,sizeof *a);if(!a){rc=2;break;}if(c->in){for(i=0;!rc&&i<c->word_list.len;i++)rc=sh_expand(c->word_list.data[i],a);}else for(i=1;!rc&&i<(size_t)sh_argc;i++)rc=sh_arg(a,sh_argv[i]);if(!rc){sh_loop++;for(i=0;i<(size_t)a->n;i++){if(sh_set(c->name,a->v[i])){rc=2;break;}rc=sh_array(&c->body);if(sh_flow==2)sh_flow=0;else if(sh_flow)break;}sh_loop--;if(sh_flow==1)sh_flow=0;}free(a);break;}
case MRSH_LOOP_CLAUSE:{struct mrsh_loop_clause *c=(void*)cmd;sh_loop++;while(!sh_flow){int cond=sh_array(&c->condition);if(sh_flow||((cond==0)!=(c->type==MRSH_LOOP_WHILE)))break;rc=sh_array(&c->body);if(sh_flow==2)sh_flow=0;if(app_yield()){rc=130;sh_flow=4;sh_flow_status=130;break;}}sh_loop--;if(sh_flow==1)sh_flow=0;break;}
case MRSH_CASE_CLAUSE:{struct mrsh_case_clause *c=(void*)cmd;struct sh_word *word=calloc(1,sizeof *word),*pat=calloc(1,sizeof *pat);int matched=0;if(!word||!pat){free(word);free(pat);rc=2;break;}rc=sh_word(c->word,word,1);for(i=0;!rc&&!matched&&i<c->items.len;i++){struct mrsh_case_item *item=c->items.data[i];size_t j;for(j=0;!matched&&j<item->patterns.len;j++){memset(pat,0,sizeof *pat);rc=sh_word(item->patterns.data[j],pat,1);if(rc)break;matched=sh_pattern(pat->text,word->text);}if(matched)rc=sh_array(&item->body);}free(word);free(pat);break;}
case MRSH_FUNCTION_DEFINITION:{struct mrsh_function_definition *c=(void*)cmd;if(c->io_redirects.len||strlen(c->name)>31){rc=sh_fail("function redirections/name unsupported");break;}for(i=0;i<16;i++)if(!sh_functions[i].name[0]||!strcmp(sh_functions[i].name,c->name))break;if(i==16){rc=sh_fail("function table full");break;}if(sh_functions[i].body)mrsh_command_destroy(sh_functions[i].body);strcpy(sh_functions[i].name,c->name);sh_functions[i].body=mrsh_command_copy(c->body);if(!sh_functions[i].body)rc=2;break;}}
sh_depth--;shell_status=rc;return rc;}
static int sh_andor(struct mrsh_and_or_list *node){if(node->type==MRSH_AND_OR_LIST_BINOP){struct mrsh_binop *b=(void*)node;int rc=sh_andor(b->left);if(!sh_flow&&((b->type==MRSH_BINOP_AND&&!rc)||(b->type==MRSH_BINOP_OR&&rc)))rc=sh_andor(b->right);return rc;}
 struct mrsh_pipeline *p=(void*)node;size_t i;int rc=0;short oldin=input_ref,oldout=output_ref,previous=-1;unsigned char previous_name[256];for(i=0;i<p->commands.len&&!sh_flow;i++){short next=-1;unsigned char name[256];struct sh_saved *saved=p->commands.len>1?sh_save():0;if(p->commands.len>1&&!saved){rc=2;break;}if(i+1<p->commands.len&&sh_temp(&next,name)){if(saved)sh_restore(saved);rc=sh_fail("cannot create pipeline spool");break;}input_ref=previous>=0?previous:oldin;output_ref=next>=0?next:oldout;rc=sh_command(p->commands.data[i]);if(saved)sh_restore(saved);if(previous>=0)sh_temp_close(previous,previous_name);previous=next;if(next>=0){memcpy(previous_name,name,sizeof name);SetFPos(next,fsFromStart,0);}if(rc==130)break;}if(previous>=0)sh_temp_close(previous,previous_name);input_ref=oldin;output_ref=oldout;return rc==130?130:p->bang?!rc:rc;}
static int sh_array(struct mrsh_array *a){size_t i;int rc=0;for(i=0;i<a->len&&!sh_flow;i++){struct mrsh_command_list *c=a->data[i];if(c->ampersand)return sh_fail("background jobs are not implemented");rc=sh_andor(c->and_or_list);shell_status=rc;}return rc;}
static int sh_source(const char *source){struct mrsh_parser *parser;struct mrsh_program *program;int rc;size_t n=strlen(source);if(n>SH_TEXT)return sh_fail("source exceeds 8 KiB limit");
 {size_t k;int nest=0,tokens=0,space=1,q=0;for(k=0;k<n;k++){unsigned char c=source[k];if(c=='\\'&&q!=1){if(source[k+1])k++;continue;}if(c=='\''&&q!=2){q=q==1?0:1;continue;}if(c=='"'&&q!=1){q=q==2?0:2;continue;}if(!q){if(c=='('||c=='{'){if(++nest>16)return sh_fail("syntax nesting exceeds 16");}if((c==')'||c=='}')&&nest)nest--;if(isspace(c)||strchr(";|&<>(){}",c)){space=1;if(!isspace(c))tokens++;}else if(space){tokens++;space=0;}if(tokens>256)return sh_fail("source exceeds 256-token baseline limit");}}}
if(sh_depth>=SH_DEPTH||sh_source_depth>=8)return sh_fail("source nesting limit");
#ifndef SH_HOST_TEST
 if((unsigned long)FreeMem()<n*32+16384)return sh_fail("insufficient parser working memory");
#endif
 sh_source_depth++;parser=mrsh_parser_with_data(source,n);rc=0;
 /* mrsh may finish a program at a consumed here-document terminator. Keep
    parsing the remaining source, rather than silently dropping later commands. */
 for(int chunks=0;chunks<256&&!sh_flow;chunks++){program=mrsh_parse_program(parser);if(mrsh_parser_error(parser,0)){rc=sh_fail(mrsh_parser_error(parser,0));mrsh_program_destroy(program);break;}if(!program){rc=sh_fail("parser allocation failed");break;}int empty=!program->body.len;rc=sh_array(&program->body);mrsh_program_destroy(program);if(mrsh_parser_eof(parser))break;if(empty){rc=sh_fail("unconsumed source");break;}}
 mrsh_parser_destroy(parser);sh_source_depth--;return rc;}
int shell_run(const char *source){int rc;history_add(source);sh_flow=0;output_error=0;rc=sh_source(source);if(sh_flow==3||sh_flow==4)rc=sh_flow_status;sh_flow=0;shell_status=rc;return rc;}

#ifndef SH_HOST_TEST
void shell_profile(void){char native[PATH_CAP];CInfoPBRec pb;if(resolve("/etc/profile",native)||info(native,&pb)){term_puts("Profile unavailable; using built-in defaults.\n");return;}char *args[]={"/etc/profile"};int rc=sh_file(args[0],1,args,1);if(rc)term_puts("Profile returned an error; built-ins remain available.\n");sh_flow=0;shell_status=0;}
#endif
