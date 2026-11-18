/* Clarice shell execution adapter. The parser/AST in vendor/mrsh is MIT
 * licensed, copyright 2018 emersion. This adapter is original project code.
 * Adapter storage grows on demand; parser/stack safeguards remain explicit. */
#include <mrsh/parser.h>
#include <mrsh/arithm.h>
#include <limits.h>
#define SH_DEPTH 16
static int sh_source_depth,sh_depth,sh_flow,sh_flow_status,sh_loop,sh_functions_active;
static int sh_argc=1;
static unsigned long sh_substitutions;
static int sh_substatus;
static char *sh_owned_args;
static char *sh_initial_argv[]={"sh",0};
static char **sh_argv=sh_initial_argv;
struct sh_function {struct sh_function *next;struct mrsh_command *body;char name[1];};
static struct sh_function *sh_functions;
static void sh_functions_free(struct sh_function *f){while(f){struct sh_function *next=f->next;mrsh_command_destroy(f->body);free(f);f=next;}}
static struct sh_function *sh_function_new(const char *name,const struct mrsh_command *body){size_t len=strlen(name);if(len>(size_t)-1-sizeof(struct sh_function))return 0;struct sh_function *f=malloc(sizeof *f+len);if(!f)return 0;f->body=mrsh_command_copy(body);if(!f->body){free(f);return 0;}f->next=0;strcpy(f->name,name);return f;}
static int sh_functions_clone(struct sh_function *src,struct sh_function **out){struct sh_function **tail=out;*out=0;for(;src;src=src->next){struct sh_function *f=sh_function_new(src->name,src->body);if(!f){sh_functions_free(*out);*out=0;return -1;}*tail=f;tail=&f->next;}return 0;}
static int sh_array(struct mrsh_array *a);
static int sh_command(struct mrsh_command *cmd);
static int sh_command_body(struct mrsh_command *cmd);
static int sh_source(const char *source);
static int sh_fail(const char *s){errsay("sh: ");errsay(s);errsay("\n");return 2;}
static int sh_set(const char *name,const char *value){return env_assign(name,value)?sh_fail("invalid variable name or out of memory"):0;}
static int sh_position_index(const char *name){unsigned int n=0;if(!*name)return -1;for(;*name;name++){unsigned char c=*name;if(c<'0'||c>'9'||n>((unsigned int)INT_MAX-(c-'0'))/10)return -1;n=n*10+c-'0';}return (int)n;}
static const char *sh_get(const char *name,char *num){if(!strcmp(name,"?")){snprintf(num,32,"%d",shell_status);return num;}if(!strcmp(name,"#")){snprintf(num,32,"%d",sh_argc-1);return num;}if(isdigit((unsigned char)name[0])){int i=sh_position_index(name);return i>=0&&i<sh_argc?sh_argv[i]:"";}return env_get(name);}
static int sh_isset(const char *name){if(!strcmp(name,"HOME")||!strcmp(name,"PWD")||!strcmp(name,"?")||!strcmp(name,"#"))return 1;if(isdigit((unsigned char)*name))return sh_position_index(name)>=0&&sh_position_index(name)<sh_argc;struct sh_env *e=env_find(sh_env,name);return e&&(e->flags&SH_ENV_SET);}
/* A single owned block stores positional pointers and strings. */
static char *sh_positions_copy(int n,char **v,char ***out){size_t bytes;if(n<1||(size_t)n>((size_t)-1/sizeof(char*))-1)return 0;bytes=((size_t)n+1)*sizeof(char*);for(int i=0;i<n;i++){size_t len=strlen(v[i])+1;if(len>(size_t)-1-bytes)return 0;bytes+=len;}char *storage=malloc(bytes);if(!storage)return 0;char **args=(char**)storage;char *p=storage+((size_t)n+1)*sizeof(char*);for(int i=0;i<n;i++){size_t len=strlen(v[i])+1;args[i]=p;memcpy(p,v[i],len);p+=len;}args[n]=0;*out=args;return storage;}
static int sh_positions(int n,char **v){char **args;char *storage=sh_positions_copy(n,v,&args);if(!storage)return sh_fail("out of memory storing positional parameters");free(sh_owned_args);sh_owned_args=storage;sh_argv=args;sh_argc=n;return 0;}
static int sh_pattern(const char *p,const char *s){while(*p){if(*p=='*'){while(*p=='*')p++;if(!*p)return 1;do{if(sh_pattern(p,s))return 1;}while(*s++);return 0;}if(*p=='?'){if(!*s)return 0;p++;s++;continue;}if(*p=='['){int yes=0,neg=0;unsigned char c=*s++;if(!c)return 0;p++;if(*p=='!'||*p=='^'){neg=1;p++;}while(*p&&*p!=']'){unsigned char lo=*p++,hi=lo;if(*p=='-'&&p[1]&&p[1]!=']'){p++;hi=*p++;}if(c>=lo&&c<=hi)yes=1;}if(*p!=']'||yes==neg)return 0;p++;continue;}if(*p=='\\'&&p[1])p++;if(*p++!=*s++)return 0;}return !*s;}
struct sh_saved {
struct sh_saved *saved_next;
#if !defined(SH_HOST_TEST)||defined(SH_SESSION_HOST_TEST)
int session_exit;
#endif
struct sh_env *env;char *args;char **argv;int argc,env_defaults;char cwd[PATH_CAP];
#ifndef SH_HOST_TEST
char unixdir[PATH_CAP];
#endif
struct sh_function *functions;};
static struct sh_saved *sh_saved_frames;
static struct sh_saved *sh_save(void){
#ifndef SH_HOST_TEST
 if(!native_heap_room(0,65536L,1))return 0;
#endif
 struct sh_saved *s=malloc(sizeof *s);if(!s)return 0;if(env_clone(sh_env,&s->env)){free(s);return 0;}s->env_defaults=sh_env_defaults;s->argc=sh_argc;s->args=sh_positions_copy(sh_argc,sh_argv,&s->argv);if(!s->args){env_free(s->env);free(s);return 0;}strcpy(s->cwd,shell_cwd);
#ifndef SH_HOST_TEST
strcpy(s->unixdir,unix_cwd);
if(!native_heap_room(0,65536L,1)){env_free(s->env);free(s->args);free(s);return 0;}
#endif
#if !defined(SH_HOST_TEST)||defined(SH_SESSION_HOST_TEST)
s->session_exit=session_exit_requested;
#endif
if(sh_functions_clone(sh_functions,&s->functions)){env_free(s->env);free(s->args);free(s);return 0;}s->saved_next=sh_saved_frames;sh_saved_frames=s;return s;}
static void sh_restore(struct sh_saved *s){
sh_saved_frames=s->saved_next;
#if !defined(SH_HOST_TEST)||defined(SH_SESSION_HOST_TEST)
session_exit_requested=s->session_exit;
#endif
sh_env_defaults=s->env_defaults;env_free(sh_env);sh_env=s->env;free(sh_owned_args);sh_owned_args=s->args;sh_argv=s->argv;sh_argc=s->argc;strcpy(shell_cwd,s->cwd);
#ifndef SH_HOST_TEST
strcpy(unix_cwd,s->unixdir);
#endif
sh_functions_free(sh_functions);sh_functions=s->functions;free(s);}
static unsigned long sh_temp_serial;
static int sh_temp(short *ref,unsigned char *name){char path[PATH_CAP];int e,i;for(i=0;i<100;i++){snprintf(path,sizeof path,"%.220stmp:clarice-pipe-%lu",home_path,++sh_temp_serial);pstr(name,path);e=HCreate(0,0,name,'S6UX','TEXT');if(!e){e=HOpen(0,0,name,fsRdWrPerm,ref);if(e)HDelete(0,0,name);return e;}if(e!=dupFNErr)return e;}return dupFNErr;}
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
struct sh_word {char *text;unsigned char *mode;int used,quoted,quote_end;size_t cap;char small[64];unsigned char flags[64];};
static struct sh_word *sh_word_new(void){struct sh_word *w=calloc(1,sizeof *w);if(w){w->quote_end=-1;w->text=w->small;w->mode=w->flags;w->cap=sizeof w->small;}return w;}
static void sh_word_reset(struct sh_word *w){w->used=w->quoted=0;w->quote_end=-1;w->text[0]=0;}
static void sh_word_free(struct sh_word *w){if(w){if(w->text!=w->small)free(w->text);free(w);}}
static int sh_append(struct sh_word *w,const char *s,int mode){size_t n=strlen(s),need;if(n>(size_t)INT_MAX-1-(size_t)w->used)return sh_fail("expanded word too large");need=(size_t)w->used+n+1;
 if(need>w->cap){size_t cap=w->cap+w->cap/2;if(cap<w->cap||cap<need)cap=need;if(cap>(size_t)-1/2)return sh_fail("expanded word too large");char *p=malloc(cap*2);if(!p)return sh_fail("out of memory expanding word");memcpy(p,w->text,w->used);memcpy(p+cap,w->mode,w->used);if(w->text!=w->small)free(w->text);w->text=p;w->mode=(unsigned char*)p+cap;w->cap=cap;}
 memcpy(w->text+w->used,s,n);memset(w->mode+w->used,mode,n);w->used+=(int)n;w->text[w->used]=0;if(mode==1){w->quoted=1;w->quote_end=w->used;}return 0;}
static int sh_word(struct mrsh_word *node,struct sh_word *w,int quoted){size_t i;char num[32];if(!node)return 0;switch(node->type){
case MRSH_WORD_STRING:{struct mrsh_word_string *s=(void*)node;return sh_append(w,s->str,quoted||s->single_quoted?1:0);}
case MRSH_WORD_LIST:{struct mrsh_word_list *l=(void*)node;if(l->double_quoted)w->quoted=1;for(i=0;i<l->children.len;i++){int rc=sh_word(l->children.data[i],w,quoted||l->double_quoted);if(rc)return rc;}return 0;}
case MRSH_WORD_PARAMETER:{struct mrsh_word_parameter *p=(void*)node;const char *value=sh_get(p->name,num);char *stable=0;int unset=!sh_isset(p->name)||(p->colon&&!*value);struct sh_word *arg;
 if(!strcmp(p->name,"@")||!strcmp(p->name,"*")){if(p->op!=MRSH_PARAM_NONE)return sh_fail("parameter operation on $@/$* unsupported");for(i=1;i<(size_t)sh_argc;i++){char sep[2]={' ',0};if(quoted)sep[0]=sh_isset("IFS")?env_get("IFS")[0]:' ';if(i>1&&sh_append(w,sep,quoted?1:2))return 2;if(sh_append(w,sh_argv[i],quoted?1:2))return 2;}return 0;}
 if(p->op==MRSH_PARAM_LEADING_HASH){snprintf(num,sizeof num,"%lu",(unsigned long)strlen(value));value=num;}
 if(p->op==MRSH_PARAM_NONE||p->op==MRSH_PARAM_LEADING_HASH)return sh_append(w,value,quoted?1:2);
 if(p->op==MRSH_PARAM_MINUS&&!unset)return sh_append(w,value,quoted?1:2);
 if(p->op==MRSH_PARAM_EQUAL&&!unset)return sh_append(w,value,quoted?1:2);
 if(p->op==MRSH_PARAM_QMARK&&!unset)return sh_append(w,value,quoted?1:2);
 if(p->op==MRSH_PARAM_PLUS&&unset)return 0;
 stable=malloc(strlen(value)+1);if(!stable)return sh_fail("out of memory");strcpy(stable,value);value=stable;arg=sh_word_new();if(!arg){free(stable);return sh_fail("out of memory");}int rc=sh_word(p->arg,arg,quoted);
 if(!rc){if(p->op==MRSH_PARAM_QMARK)rc=sh_fail(arg->used?arg->text:"unset parameter");
 else if(p->op==MRSH_PARAM_EQUAL){rc=sh_set(p->name,arg->text);if(!rc)rc=sh_append(w,arg->text,quoted?1:2);}
 else if(p->op==MRSH_PARAM_MINUS||p->op==MRSH_PARAM_PLUS)rc=sh_append(w,arg->text,quoted?1:2);
 else {char *copy=stable;size_t n=strlen(value),j;int match=-1;{for(j=0;j<=n;j++){char save=copy[j];int yes;if(p->op==MRSH_PARAM_HASH||p->op==MRSH_PARAM_DHASH){copy[j]=0;yes=sh_pattern(arg->text,copy);copy[j]=save;}else yes=sh_pattern(arg->text,copy+j);if(yes){match=j;if(p->op==MRSH_PARAM_HASH||p->op==MRSH_PARAM_DPERCENT)break;}}if(match>=0){if(p->op==MRSH_PARAM_HASH||p->op==MRSH_PARAM_DHASH)memmove(copy,copy+match,n-match+1);else copy[match]=0;}rc=sh_append(w,copy,quoted?1:2);}}}free(stable);sh_word_free(arg);return rc;}
case MRSH_WORD_ARITHMETIC:{struct sh_word *arg=sh_word_new();long value;int rc;if(!arg)return sh_fail("out of memory");rc=sh_word(((struct mrsh_word_arithmetic*)node)->body,arg,1);if(!rc){struct mrsh_parser *p=mrsh_parser_with_data(arg->text,arg->used);struct mrsh_arithm_expr *e=mrsh_parse_arithm_expr(p);if(!e||mrsh_parser_error(p,0)||!sh_arith(e,&value))rc=sh_fail(mrsh_parser_error(p,0)?mrsh_parser_error(p,0):"unsupported or invalid arithmetic");else{snprintf(num,sizeof num,"%ld",value);rc=sh_append(w,num,quoted?1:2);}mrsh_arithm_expr_destroy(e);mrsh_parser_destroy(p);}sh_word_free(arg);return rc;}
case MRSH_WORD_COMMAND:{struct mrsh_word_command *c=(void*)node;short ref,old=output_ref;unsigned char name[256];struct sh_saved *saved=sh_save();char *value=0;long n=0,size=0;int rc,flow=sh_flow;if(!saved)return sh_fail("out of memory");if(sh_temp(&ref,name)){sh_restore(saved);return sh_fail("cannot create substitution spool");}output_ref=ref;script_depth++;rc=c->program?sh_array(&c->program->body):0;script_depth--;output_ref=old;sh_restore(saved);sh_flow=flow;
 int e=GetEOF(ref,&size);if(!e&&(size<0||size>=INT_MAX))e=-1;if(!e){value=malloc((size_t)size+1);if(!value)e=-1;}if(!e)e=SetFPos(ref,fsFromStart,0);if(!e){n=size;e=FSRead(ref,&n,value);if(e==eofErr&&n==size)e=0;if(n!=size)e=-1;}sh_temp_close(ref,name);
 if(!e&&memchr(value,0,(size_t)n))e=-1;if(e){free(value);return sh_fail("cannot read command substitution (I/O, binary data or memory)");}while(n&&(value[n-1]=='\n'||value[n-1]=='\r'))n--;value[n]=0;shell_status=rc;sh_substatus=rc;sh_substitutions++;int result=rc==130?130:sh_append(w,value,quoted?1:2);free(value);return result;}}

return sh_fail("unknown word");}
struct sh_args {char **v;size_t cap;int n;};
static void sh_args_free(struct sh_args *a){if(a){for(int i=0;i<a->n;i++)free(a->v[i]);free(a->v);free(a);}}
static int sh_arg(struct sh_args *a,const char *s){if(a->n==INT_MAX)return sh_fail("too many arguments");if((size_t)a->n+2>a->cap){size_t cap=a->cap?a->cap*2:8;if(cap<a->cap||cap>(size_t)-1/sizeof(char*))return sh_fail("argument storage overflow");char **v=realloc(a->v,cap*sizeof *v);if(!v)return sh_fail("out of memory storing arguments");a->v=v;a->cap=cap;}size_t len=strlen(s)+1;char *copy=malloc(len);if(!copy)return sh_fail("out of memory storing argument");memcpy(copy,s,len);a->v[a->n++]=copy;a->v[a->n]=0;return 0;}
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
static int sh_fields(struct sh_word *w,struct sh_args *a){int rc=0,i,start,prior=a->n;const char *ifs;
 ifs=sh_isset("IFS")?env_get("IFS"):" \t\n";if(!rc&&w->used&&w->text[0]=='~'&&w->mode[0]!=1&&(w->text[1]=='/'||!w->text[1])){const char *tail=w->text+1;if(*tail=='/')tail++;
#ifdef SH_HOST_TEST
 const char *prefix="native:";size_t len=strlen(prefix)+strlen(home_path)+strlen(tail)+1;char *expanded=malloc(len);if(expanded)snprintf(expanded,len,"%s%s%s",prefix,home_path,tail);
#else
 size_t len=strlen(tail)+2;char *expanded=malloc(len);if(expanded)snprintf(expanded,len,"/%s",tail);
#endif
 if(!expanded)rc=sh_fail("out of memory");else{sh_word_reset(w);rc=sh_append(w,expanded,1);free(expanded);}}

 if(!rc){if(!w->used&&w->quoted)rc=sh_arg(a,"");i=0;
  while(i<w->used&&w->mode[i]==2&&strchr(ifs,w->text[i])&&strchr(" \t\n",w->text[i]))i++;
  while(i<w->used&&!rc){start=i;while(i<w->used&&!(w->mode[i]==2&&strchr(ifs,w->text[i])))i++;
   int end=i,nonwhite=i<w->used&&!strchr(" \t\n",w->text[i]);
   if(end>start||nonwhite){char save=w->text[end];w->text[end]=0;rc=sh_glob(a,w->text+start,w->mode+start);w->text[end]=save;}
   if(i<w->used){if(nonwhite)i++;else{while(i<w->used&&w->mode[i]==2&&strchr(ifs,w->text[i])&&strchr(" \t\n",w->text[i]))i++;if(i<w->used&&w->mode[i]==2&&strchr(ifs,w->text[i]))i++;}
    while(i<w->used&&w->mode[i]==2&&strchr(ifs,w->text[i])&&strchr(" \t\n",w->text[i]))i++;}
  }
 }if(!rc&&w->quoted&&(a->n==prior||(w->used&&w->quote_end==w->used&&w->mode[w->used-1]==2&&strchr(ifs,w->text[w->used-1]))))rc=sh_arg(a,"");return rc;}
/* Quoted $@ creates field boundaries, not a joined string. Flush only at
 * those boundaries so an adjacent prefix/suffix stays on the first/last field. */
static int sh_argument_word(struct mrsh_word *node,struct sh_word *w,struct sh_args *a,int quoted){
 if(!node)return 0;
 if(node->type==MRSH_WORD_LIST){struct mrsh_word_list *l=(void*)node;
  /* A quoted group retains an empty field, except a sole quoted $@,
   * which expands to no fields when there are no positional parameters.
   * Keep the group boundary (as dash does), not just individual expansions. */
  struct mrsh_word *only=l->children.len==1?l->children.data[0]:0;
  int sole_at=only&&only->type==MRSH_WORD_PARAMETER&&
   !strcmp(((struct mrsh_word_parameter*)only)->name,"@")&&
   ((struct mrsh_word_parameter*)only)->op==MRSH_PARAM_NONE;
  if(l->double_quoted&&!sole_at){w->quoted=1;w->quote_end=w->used;}
  for(size_t i=0;i<l->children.len;i++){int rc=sh_argument_word(l->children.data[i],w,a,quoted||l->double_quoted);if(rc)return rc;}return 0;
 }
 if(node->type==MRSH_WORD_PARAMETER){struct mrsh_word_parameter *p=(void*)node;
  if(p->op==MRSH_PARAM_MINUS||p->op==MRSH_PARAM_PLUS){char num[32];const char *value=sh_get(p->name,num);int unset=!sh_isset(p->name)||(p->colon&&!*value);
   if(p->op==MRSH_PARAM_MINUS?unset:!unset)return sh_argument_word(p->arg,w,a,quoted);
   if(p->op==MRSH_PARAM_PLUS)return sh_append(w,"",quoted?1:2);
  }
 }
 if(quoted&&node->type==MRSH_WORD_PARAMETER){struct mrsh_word_parameter *p=(void*)node;
  if(!strcmp(p->name,"@")&&p->op==MRSH_PARAM_NONE){
   for(int i=1;i<sh_argc;i++){int rc=sh_append(w,sh_argv[i],1);if(rc)return rc;if(i+1<sh_argc){rc=sh_fields(w,a);if(rc)return rc;sh_word_reset(w);}}return 0;
  }
 }
 return sh_word(node,w,quoted);
}
static int sh_expand(struct mrsh_word *node,struct sh_args *a){struct sh_word *w=sh_word_new();if(!w)return sh_fail("out of memory");int rc=sh_argument_word(node,w,a,0);if(!rc)rc=sh_fields(w,a);sh_word_free(w);return rc;}
/* Arity-sensitive POSIX test forms. Peel negation/grouping iteratively;
 * binary operators take precedence over treating an operand as syntax. */
static int sh_test(int n,char **v){int negate=0,result;
 for(;;){
  if(n==0){result=1;break;}if(n==1){result=!*v[0];break;}
  if(n==3&&(!strcmp(v[1],"=")||!strcmp(v[1],"!="))){result=!!strcmp(v[0],v[2]);if(v[1][0]=='!')result=!result;break;}
  if(n==3&&(!strcmp(v[1],"-eq")||!strcmp(v[1],"-ne")||!strcmp(v[1],"-lt")||!strcmp(v[1],"-le")||!strcmp(v[1],"-gt")||!strcmp(v[1],"-ge"))){
   long values[2];for(int i=0;i<2;i++){const char *s=v[i*2];char *end;errno=0;values[i]=strtol(s,&end,10);if(end==s||errno==ERANGE)return sh_fail("test: integer expected");while(isspace((unsigned char)*end))end++;if(*end)return sh_fail("test: integer expected");}
   long a=values[0],b=values[1];const char *op=v[1];result=!strcmp(op,"-eq")?a!=b:!strcmp(op,"-ne")?a==b:!strcmp(op,"-lt")?a>=b:!strcmp(op,"-le")?a>b:!strcmp(op,"-gt")?a<=b:a<b;break;
  }
  if(!strcmp(v[0],"!")){negate=!negate;v++;n--;continue;}
  if(n>=3&&!strcmp(v[0],"(")&&!strcmp(v[n-1],")")){v++;n-=2;continue;}
  if(n!=2)return sh_fail("unsupported test expression; use shell && or ||");
  if(!strcmp(v[0],"-n")){result=!*v[1];break;}if(!strcmp(v[0],"-z")){result=!!*v[1];break;}
#ifndef SH_HOST_TEST
  if(!strcmp(v[0],"-L")||!strcmp(v[0],"-h")){char p[PATH_CAP];CInfoPBRec pb;result=resolve_ex(v[1],p,0,0)||info(p,&pb)||!is_link(&pb);break;}
#else
  if(!strcmp(v[0],"-L")||!strcmp(v[0],"-h")){struct stat st;result=lstat(v[1],&st)||!S_ISLNK(st.st_mode);break;}
#endif
  if(!strcmp(v[0],"-e")||!strcmp(v[0],"-f")||!strcmp(v[0],"-d")||!strcmp(v[0],"-s")){char p[PATH_CAP];CInfoPBRec pb;if(resolve(v[1],p)||info(p,&pb)){result=1;break;}int dir=!!(pb.hFileInfo.ioFlAttrib&16);result=v[0][1]=='e'?0:v[0][1]=='d'?!dir:v[0][1]=='f'?dir:!pb.hFileInfo.ioFlLgLen;break;}
  return sh_fail("unsupported test operator");
 }return negate?!result:result;
}
/* A new sh invocation inherits exported values, not the parent's local
 * variables or function table. Subshell groups use the existing full snapshot. */
static void sh_child_scope(void){struct sh_env **slot=&sh_env;while(*slot){struct sh_env *e=*slot;if((e->flags&(SH_ENV_SET|SH_ENV_EXPORT))!=(SH_ENV_SET|SH_ENV_EXPORT)){*slot=e->next;free(e);}else slot=&e->next;}sh_functions_free(sh_functions);sh_functions=0;}
static void sh_quote_value(const char *s){say("'");for(;*s;s++){if(*s=='\'')say("'\\''");else emit(s,1);}say("'");}
static int sh_file(const char *path,int argc,char **argv,int dot){short ref;long n,size;char *text;int rc,i,oldargc=sh_argc,oldflow=sh_flow;char *owned=sh_owned_args;char **oldargv=sh_argv;struct sh_saved *saved=dot?0:sh_save();if(!dot&&!saved)return sh_fail("out of memory");if(open_path(path,fsRdPerm,&ref)){if(saved)sh_restore(saved);return sh_fail("cannot open script");}rc=GetEOF(ref,&size);if(rc||size<0||(unsigned long)size>=(size_t)-1){FSClose(ref);if(saved)sh_restore(saved);return sh_fail("cannot read script size");}text=malloc(size+1);if(!text){FSClose(ref);if(saved)sh_restore(saved);return sh_fail("out of memory");}n=size;rc=FSRead(ref,&n,text);FSClose(ref);if((rc&&rc!=eofErr)||n!=size||memchr(text,0,(size_t)n)){free(text);if(saved)sh_restore(saved);return sh_fail("script read failed or contains NUL");}text[n]=0;for(i=0;i<n;i++)if(text[i]=='\r')text[i]='\n';if(dot){if(argc>1)sh_owned_args=0;}else sh_child_scope();if(!dot||argc>1)rc=sh_positions(argc,argv);else{sh_argv=oldargv;rc=0;}if(!rc){script_depth++;sh_flow=0;rc=sh_source(text);script_depth--;}sh_flow=oldflow;free(text);if(saved)sh_restore(saved);else if(argc>1){free(sh_owned_args);sh_owned_args=owned;sh_argc=oldargc;sh_argv=oldargv;}return rc;}

#include "shell-read.inc"
#include "shell-printf.inc"
/* Query the same native PATH resolver used for execution; no shell cache. */
static int sh_command_v(int n,char **v){
 if(n<2||strcmp(v[1],"-v"))return sh_fail("usage: command -v [--] name");
 int at=2;if(at<n&&!strcmp(v[at],"--"))at++;
 if(at==n)return 0;if(n!=at+1)return sh_fail("command -v: one name required");
 const char *name=v[at];
 const char *special[]={":","[","break","continue","return","set","shift","unset","read",".","command", "if","then","else","elif","fi","do","done","while","until","for","case","esac","in","!","{","}",0};
 for(int i=0;special[i];i++)if(!strcmp(name,special[i])){say(name);say("\n");return output_error?1:0;}
 for(struct sh_function *f=sh_functions;f;f=f->next)if(!strcmp(name,f->name)){say(name);say("\n");return output_error?1:0;}
 for(int i=0;commands[i];i++)if(!strcmp(name,commands[i])){say(name);say("\n");return output_error?1:0;}
 char path[PATH_CAP];int kind;if(command_path(name,path,&kind)>0){say(path);say("\n");return output_error?1:0;}return 127;
}
static int sh_dispatch(struct sh_args *a);
/* Utility wrappers live here to reuse scoped execution, not nest CLX runtimes. */
static int sh_scoped_utility(struct sh_args *a,int timed){
 int n=a->n,at=1,rc=0,oldflow=sh_flow,oldstatus=sh_flow_status;char **v=a->v;
 if(n==2&&!strcmp(v[1],"--help"))return builtin_help(timed?"time":"env");
 if(sh_depth>=SH_DEPTH)return sh_fail("execution nesting limit");
 struct sh_saved *saved=sh_save();if(!saved)return sh_fail("out of memory");
 if(!timed){sh_child_scope();
  while(at<n&&v[at][0]=='-'){
   if(!strcmp(v[at],"--")){at++;break;}
   if(!strcmp(v[at],"-i")||!strcmp(v[at],"-")){env_free(sh_env);sh_env=0;sh_env_defaults=0;at++;continue;}
   if(!strcmp(v[at],"-u")){if(++at>=n||!env_name(v[at],strlen(v[at]))){rc=sh_fail("env: -u needs a variable name");goto done;}if(!strcmp(v[at],"HOME"))sh_env_defaults&=~1;if(!strcmp(v[at],"PWD"))sh_env_defaults&=~2;free(env_take(&sh_env,v[at++]));continue;}
   rc=sh_fail("env: unsupported option; use help env");goto done;
  }
  while(at<n&&strchr(v[at],'=')){if(env_set(v[at++])){rc=sh_fail("env: invalid name or out of memory");goto done;}}
 }else if(at<n&&!strcmp(v[at],"--"))at++;
 else if(at<n&&v[at][0]=='-'){rc=sh_fail("time: only elapsed time is available; use help time");goto done;}
 if(at==n){if(timed){rc=sh_fail("time: command required");goto done;}
  if(sh_env_defaults){struct sh_env *e=env_find(sh_env,"HOME");if(!e&&(sh_env_defaults&1)){say("HOME=/\n");}e=env_find(sh_env,"PWD");if(!e&&(sh_env_defaults&2)){say("PWD=");say(env_get("PWD"));say("\n");}}
  for(struct sh_env *e=sh_env;e;e=e->next)if((e->flags&(SH_ENV_SET|SH_ENV_EXPORT))==(SH_ENV_SET|SH_ENV_EXPORT)){say(e->name);say("=");say(e->value);say("\n");if(app_yield()){rc=130;goto done;}}
  rc=output_error?1:0;goto done;
 }
 {struct sh_args child=*a;child.n=n-at;child.v=v+at;unsigned long started=TickCount();
  sh_depth++;script_depth++;sh_flow=0;rc=sh_dispatch(&child);script_depth--;sh_depth--;
  if(timed){unsigned long ticks=TickCount()-started;char b[64];snprintf(b,sizeof b,"real %lu.%02lu s\n",ticks/60,(ticks%60)*100/60);errsay(b);}
 }
done:sh_restore(saved);sh_flow=oldflow;sh_flow_status=oldstatus;return rc;
}
static int sh_dispatch(struct sh_args *a){int i,n=a->n;char **v=a->v;if(!n)return 0;

#ifdef CLARICE_PARSER_SAFETY_TEST
 if(!strcmp(v[0],"parsercheck")){
  extern void parser_test_begin(unsigned long,int);
  extern void parser_test_end(unsigned long*,unsigned long*,int*);
  unsigned long count,before,after;int hit,rc;char text[128];
  if(n!=4)return 2;
  parser_test_end(&count,&before,&hit);
  parser_test_begin(strtoul(v[1],0,10),atoi(v[2]));
  rc=sh_source(v[3]);parser_test_end(&count,&after,&hit);
  snprintf(text,sizeof text,"PARSER calls=%lu hit=%d status=%d delta=%ld\n",count,hit,rc,(long)after-(long)before);say(text);
  return rc;
 }
#endif
 if(!strcmp(v[0],"env"))return sh_scoped_utility(a,0);
 if(!strcmp(v[0],"time"))return sh_scoped_utility(a,1);
 if(!strcmp(v[0],"command"))return sh_command_v(n,v);
 if(!strcmp(v[0],":")||!strcmp(v[0],"true"))return 0;if(!strcmp(v[0],"false"))return 1;
 if(!strcmp(v[0],"test")||!strcmp(v[0],"[")){if(v[0][0]=='['){if(n<2||strcmp(v[n-1],"]"))return sh_fail("missing ]");n--;}return sh_test(n-1,v+1);}
 if(!strcmp(v[0],"break")||!strcmp(v[0],"continue")){if(!sh_loop||n>1)return sh_fail("break/continue requires a loop; numeric levels unsupported");sh_flow=v[0][0]=='b'?1:2;return 0;}
 if(!strcmp(v[0],"return")||!strcmp(v[0],"exit")){int rc=n>1?atoi(v[1]):shell_status;
#if !defined(SH_HOST_TEST) || defined(SH_SESSION_HOST_TEST)
 if(session_io&&v[0][0]=='e'){session_exit_requested=1;sh_flow=4;sh_flow_status=rc;return rc;}
#endif
 if(!script_depth&&!sh_functions_active&&v[0][0]=='r')return sh_fail("return outside script/function");if(!script_depth&&!sh_functions_active&&v[0][0]=='e'){app_quit();return rc;}sh_flow=v[0][0]=='r'?3:4;sh_flow_status=rc;return rc;}
 if(!strcmp(v[0],"export")){if(n==1||(n==2&&!strcmp(v[1],"-p"))){for(struct sh_env *e=sh_env;e;e=e->next)if(e->flags&SH_ENV_EXPORT){say("export ");say(e->name);if(e->flags&SH_ENV_SET){say("=");sh_quote_value(e->value);}say("\n");}return output_error?1:0;}for(i=1;i<n;i++)if(env_set(v[i]))return sh_fail("export: invalid name or out of memory");return 0;}
 if(!strcmp(v[0],"unset")){int funcs=n>1&&!strcmp(v[1],"-f");i=n>1&&(!strcmp(v[1],"-v")||funcs)?2:1;for(;i<n;i++){if(!env_name(v[i],strlen(v[i])))return sh_fail("unset: invalid name");if(funcs){struct sh_function **slot=&sh_functions;while(*slot&&strcmp((*slot)->name,v[i]))slot=&(*slot)->next;if(*slot){struct sh_function *old=*slot;*slot=old->next;old->next=0;sh_functions_free(old);}}else free(env_take(&sh_env,v[i]));}return 0;}
 if(!strcmp(v[0],"shift")){int count=n>1?atoi(v[1]):1;if(count<0||count>=sh_argc)return 1;memmove(sh_argv+1,sh_argv+1+count,(sh_argc-count-1)*sizeof(char*));sh_argc-=count;return 0;}
 if(!strcmp(v[0],"set")){if(n==1){for(struct sh_env *e=sh_env;e;e=e->next)if(e->flags&SH_ENV_SET){say(e->name);say("=");say(e->value);say("\n");}return 0;}int first=1;if(!strcmp(v[1],"--")||!strcmp(v[1],"-"))first=2;else if(v[1][0]=='-'||v[1][0]=='+')return sh_fail("set: unsupported shell option");if(first==2&&n==2&&v[1][1]==0)return 0;char *prior=v[first-1];v[first-1]=sh_argv[0];int rc=sh_positions(n-first+1,v+first-1);v[first-1]=prior;return rc;}

 if(!strcmp(v[0],"printf"))return sh_printf(n,v);
 if(!strcmp(v[0],"read"))return sh_read(n,v);
 if(!strcmp(v[0],"sh")){if(n<2)return sh_fail("usage: sh file [args] or sh -c command");if(!strcmp(v[1],"-c")){if(n<3)return 2;struct sh_saved *saved=sh_save();int oldflow=sh_flow;if(!saved)return 2;
 sh_child_scope();int rc=sh_positions(n>3?n-3:1,n>3?v+3:v);if(!rc){script_depth++;sh_flow=0;rc=sh_source(v[2]);script_depth--;}sh_restore(saved);sh_flow=oldflow;return rc;}return sh_file(v[1],n-1,v+1,0);}
 if(!strcmp(v[0],".")){if(n<2)return 2;return sh_file(v[1],n-1,v+1,1);}
 for(struct sh_function *f=sh_functions;f;f=f->next)if(!strcmp(f->name,v[0])){char **saved=sh_argv;int count=sh_argc,rc;char *owned=sh_owned_args;sh_owned_args=0;char *name=v[0];v[0]=saved[0];rc=sh_positions(n,v);v[0]=name;struct mrsh_command *body=rc?0:mrsh_command_copy(f->body);sh_functions_active++;rc=rc?rc:body?sh_command(body):2;sh_functions_active--;mrsh_command_destroy(body);free(sh_owned_args);sh_owned_args=owned;sh_argc=count;sh_argv=saved;if(sh_flow==3)sh_flow=0;return rc;}


#ifdef SH_HOST_TEST
 size_t len=strlen(v[0]);if((len>3&&!strcmp(v[0]+len-3,".sh"))||strchr(v[0],'/'))return sh_file(v[0],n,v,0);
 return run_args(n,v);}
#else
 for(i=0;commands[i];i++)if(!strcmp(commands[i],v[0]))return run_args(n,v);
 char candidate[PATH_CAP];int kind=0;
#ifdef CLARICE_DRAW_DIAGNOSTICS
 unsigned long dispatch_start=TickCount();
#endif
 int found=command_path(v[0],candidate,&kind);
#ifdef CLARICE_DRAW_DIAGNOSTICS
 dispatch_path_ticks=TickCount()-dispatch_start;dispatch_start=TickCount();
#endif
 if(found>0&&dispatch_legacy)kind=program_kind(candidate);
#ifdef CLARICE_DRAW_DIAGNOSTICS
 dispatch_kind_ticks=TickCount()-dispatch_start;
#endif
 if(found<0)return 126;if(found){if(session_io&&!(n==2&&!strcmp(v[1],"--help"))){const char *base=strrchr(candidate,'/');base=base?base+1:candidate;if(!strcmp(base,"clarice")||!strcmp(base,"serial")||!strcmp(base,"telnet")||!strcmp(base,"kermit")||!strcmp(base,"nano")||!strcmp(base,"nano-profile")){errsay("command currently requires local input ownership\n");return 2;}}shell_commands++;return kind==2?module_run(candidate,n,v):sh_file(candidate,n,v,0);}
 if(!strcmp(v[0],"chmod")){errsay("chmod: command not found; did you mean hfsattr?\n");return 127;}
 errsay("command not found: ");errsay(v[0]);errsay("\n");command_install_hint(v[0]);return 127;}
#endif

static int sh_special(const char *name){const char *names[]={":",".","break","continue","exit","export","return","set","shift","unset",0};for(int i=0;names[i];i++)if(!strcmp(name,names[i]))return 1;return 0;}
static int sh_assignment_argument(struct mrsh_word *w){if(w->type==MRSH_WORD_LIST){struct mrsh_word_list *l=(void*)w;return l->children.len?sh_assignment_argument(l->children.data[0]):0;}if(w->type!=MRSH_WORD_STRING)return 0;const char *s=((struct mrsh_word_string*)w)->str,*eq=strchr(s,'=');return eq&&env_name(s,(size_t)(eq-s));}
static int sh_simple(struct mrsh_simple_command *c,struct mrsh_command *compound){struct sh_args *a=calloc(1,sizeof *a);size_t i;short oldin=input_ref,oldout=output_ref,olderr=error_ref;short opened[16];unsigned char temp_names[8][256];short temp_refs[8];int temp_count=0,opened_count=0,rc=0;struct sh_saved *saved=0;if(!a)return sh_fail("out of memory");
 unsigned long substitutions_before=sh_substitutions;output_error=0;
 if(c->name)rc=sh_expand(c->name,a);for(i=0;!rc&&i<c->arguments.len;i++){struct mrsh_word *arg=c->arguments.data[i];if(a->n&&!strcmp(a->v[0],"export")&&sh_assignment_argument(arg)){struct sh_word *w=sh_word_new();if(!w)rc=sh_fail("out of memory");else{rc=sh_word(arg,w,1);if(!rc)rc=sh_arg(a,w->text);sh_word_free(w);}}else rc=sh_expand(arg,a);}
 if(a->n&&c->assignments.len&&!sh_special(a->v[0])){saved=sh_save();if(!saved)rc=sh_fail("out of memory");}
 for(i=0;!rc&&i<c->assignments.len;i++){struct mrsh_assignment *x=c->assignments.data[i];struct sh_word *w=sh_word_new();if(!w){rc=2;break;}rc=sh_word(x->value,w,1);if(!rc)rc=sh_set(x->name,w->text);if(!rc&&saved)env_find(sh_env,x->name)->flags|=SH_ENV_EXPORT;sh_word_free(w);}
 for(i=0;!rc&&i<c->io_redirects.len;i++){struct mrsh_io_redirect *r=c->io_redirects.data[i];struct sh_word *w=sh_word_new();if(!w){rc=2;break;}rc=sh_word(r->name,w,1);if(!rc){if(opened_count>=16)rc=sh_fail("too many redirections");else if(r->op==MRSH_IO_LESS&&(r->io_number<0||r->io_number==0)){short ref;if(stream_input_open(w->text,&ref))rc=sh_fail("input redirection failed");else{input_ref=ref;opened[opened_count++]=ref;}}
 else if((r->op==MRSH_IO_GREAT||r->op==MRSH_IO_DGREAT||r->op==MRSH_IO_CLOBBER)&&(r->io_number<0||r->io_number==1||r->io_number==2)){short prior=output_ref;int e=redirect(w->text,r->op==MRSH_IO_DGREAT);short ref=output_ref;output_ref=prior;if(e)rc=sh_fail("output redirection failed");else{opened[opened_count++]=ref;if(r->io_number==2)error_ref=ref;else output_ref=ref;}}
 else if((r->op==MRSH_IO_DLESS||r->op==MRSH_IO_DLESSDASH)&&(r->io_number<0||r->io_number==0)){
 if(temp_count==8)rc=sh_fail("too many here-documents");else{short ref;if(sh_temp(&ref,temp_names[temp_count]))rc=sh_fail("cannot create here-document");else{size_t j;temp_refs[temp_count++]=ref;short prior=output_ref;output_ref=ref;for(j=0;j<r->here_document.len&&!rc;j++){sh_word_reset(w);rc=sh_word(r->here_document.data[j],w,1);if(!rc){emit(w->text,w->used);say("\n");if(output_error)rc=1;}}output_ref=prior;SetFPos(ref,fsFromStart,0);input_ref=ref;}}}
 else if(r->op==MRSH_IO_GREATAND||r->op==MRSH_IO_LESSAND){int to=r->io_number<0?(r->op==MRSH_IO_GREATAND?1:0):r->io_number;short ref=-2;
 if(to<0||to>2)rc=sh_fail("descriptor must be 0, 1 or 2");
 else if(strcmp(w->text,"-")){if(w->text[1]||w->text[0]<'0'||w->text[0]>'2')rc=sh_fail("invalid descriptor duplication");else{int from=w->text[0]-'0';ref=from==0?input_ref:from==1?output_ref:error_ref;if(ref==-2){sh_fail("source descriptor is closed");rc=1;}}}
 if(!rc){if(to==0)input_ref=ref;else if(to==1)output_ref=ref;else error_ref=ref;}} 
 else rc=sh_fail("this redirection is not supported yet");}sh_word_free(w);}
 if(!rc)rc=compound?sh_command_body(compound):a->n?sh_dispatch(a):sh_substitutions!=substitutions_before?sh_substatus:0;if(output_error&&!rc)rc=1;while(opened_count){short ref=opened[--opened_count];if(!vstream_code(ref)&&FSClose(ref)&&!rc)rc=1;}while(temp_count){temp_count--;sh_temp_close(temp_refs[temp_count],temp_names[temp_count]);}input_ref=oldin;output_ref=oldout;error_ref=olderr;if(saved){
 /* Temporary prefix assignments must not undo unrelated builtin effects
    (notably IFS= read -r value). Restore only the assigned names. */
 for(i=0;i<c->assignments.len;i++){struct mrsh_assignment *x=c->assignments.data[i];free(env_take(&sh_env,x->name));}
 for(i=0;i<c->assignments.len;i++){struct mrsh_assignment *x=c->assignments.data[i];struct sh_env *e=env_take(&saved->env,x->name);if(e)env_put(&sh_env,e);}
 env_free(saved->env);free(saved->args);sh_functions_free(saved->functions);free(saved);
 }sh_args_free(a);return rc;}
/* The existing simple-command redirect owner also scopes compound execution.
   This borrows the AST list; only its normal file/temp cleanup runs here. */
static int sh_command(struct mrsh_command *cmd){
 if(cmd->redirects){struct mrsh_simple_command scope={0};scope.io_redirects=*cmd->redirects;return sh_simple(&scope,cmd);}
 return sh_command_body(cmd);
}
static int sh_command_body(struct mrsh_command *cmd){int rc=0;size_t i;if(++sh_depth>SH_DEPTH){sh_depth--;return sh_fail("execution nesting limit");}if(app_yield()){sh_depth--;sh_flow=4;sh_flow_status=130;return 130;}switch(cmd->type){
case MRSH_SIMPLE_COMMAND:rc=sh_simple((void*)cmd,0);break;
case MRSH_BRACE_GROUP:rc=sh_array(&((struct mrsh_brace_group*)cmd)->body);break;
case MRSH_SUBSHELL:{struct sh_saved *s=sh_save();int flow=sh_flow;if(!s){rc=2;break;}script_depth++;rc=sh_array(&((struct mrsh_subshell*)cmd)->body);script_depth--;sh_restore(s);sh_flow=flow;break;}
case MRSH_IF_CLAUSE:{struct mrsh_if_clause *c=(void*)cmd;rc=sh_array(&c->condition);if(!sh_flow){if(!rc)rc=sh_array(&c->body);else rc=c->else_part?sh_command(c->else_part):0;}break;}
case MRSH_FOR_CLAUSE:{struct mrsh_for_clause *c=(void*)cmd;struct sh_args *a=calloc(1,sizeof *a);if(!a){rc=2;break;}if(c->in){for(i=0;!rc&&i<c->word_list.len;i++)rc=sh_expand(c->word_list.data[i],a);}else for(i=1;!rc&&i<(size_t)sh_argc;i++)rc=sh_arg(a,sh_argv[i]);if(!rc){sh_loop++;for(i=0;i<(size_t)a->n;i++){if(sh_set(c->name,a->v[i])){rc=2;break;}rc=sh_array(&c->body);if(sh_flow==2)sh_flow=0;else if(sh_flow)break;}sh_loop--;if(sh_flow==1)sh_flow=0;}sh_args_free(a);break;}
case MRSH_LOOP_CLAUSE:{struct mrsh_loop_clause *c=(void*)cmd;sh_loop++;while(!sh_flow){int cond=sh_array(&c->condition);if(sh_flow||((cond==0)!=(c->type==MRSH_LOOP_WHILE)))break;rc=sh_array(&c->body);if(sh_flow==2)sh_flow=0;if(app_yield()){rc=130;sh_flow=4;sh_flow_status=130;break;}}sh_loop--;if(sh_flow==1)sh_flow=0;break;}
case MRSH_CASE_CLAUSE:{struct mrsh_case_clause *c=(void*)cmd;struct sh_word *word=sh_word_new(),*pat=sh_word_new();int matched=0;if(!word||!pat){sh_word_free(word);sh_word_free(pat);rc=2;break;}rc=sh_word(c->word,word,1);for(i=0;!rc&&!matched&&i<c->items.len;i++){struct mrsh_case_item *item=c->items.data[i];size_t j;for(j=0;!matched&&j<item->patterns.len;j++){sh_word_reset(pat);rc=sh_word(item->patterns.data[j],pat,1);if(rc)break;matched=sh_pattern(pat->text,word->text);}if(matched)rc=sh_array(&item->body);}sh_word_free(word);sh_word_free(pat);break;}
case MRSH_FUNCTION_DEFINITION:{struct mrsh_function_definition *c=(void*)cmd;if(c->io_redirects.len){rc=sh_fail("function redirections unsupported");break;}struct sh_function *replacement=sh_function_new(c->name,c->body);if(!replacement){rc=sh_fail("out of memory copying function");break;}struct sh_function **slot=&sh_functions;while(*slot&&strcmp((*slot)->name,c->name))slot=&(*slot)->next;struct sh_function *old=*slot;replacement->next=old?old->next:0;*slot=replacement;if(old){old->next=0;sh_functions_free(old);}break;}}

sh_depth--;shell_status=rc;return rc;}
static int sh_andor(struct mrsh_and_or_list *node){if(node->type==MRSH_AND_OR_LIST_BINOP){struct mrsh_binop *b=(void*)node;int rc=sh_andor(b->left);if(!sh_flow&&((b->type==MRSH_BINOP_AND&&!rc)||(b->type==MRSH_BINOP_OR&&rc)))rc=sh_andor(b->right);return rc;}
 struct mrsh_pipeline *p=(void*)node;size_t i;int rc=0;short oldin=input_ref,oldout=output_ref,previous=-1;unsigned char previous_name[256];for(i=0;i<p->commands.len&&!sh_flow;i++){short next=-1;unsigned char name[256];struct sh_saved *saved=p->commands.len>1?sh_save():0;if(p->commands.len>1&&!saved){rc=2;break;}if(i+1<p->commands.len&&sh_temp(&next,name)){if(saved)sh_restore(saved);rc=sh_fail("cannot create pipeline spool");break;}input_ref=previous>=0?previous:oldin;output_ref=next>=0?next:oldout;rc=sh_command(p->commands.data[i]);if(saved)sh_restore(saved);if(previous>=0)sh_temp_close(previous,previous_name);previous=next;if(next>=0){memcpy(previous_name,name,sizeof name);SetFPos(next,fsFromStart,0);}if(rc==130)break;}if(previous>=0)sh_temp_close(previous,previous_name);input_ref=oldin;output_ref=oldout;return rc==130?130:p->bang?!rc:rc;}
static int sh_array(struct mrsh_array *a){size_t i;int rc=0;for(i=0;i<a->len&&!sh_flow;i++){struct mrsh_command_list *c=a->data[i];if(c->ampersand)return sh_fail("background jobs are not implemented");rc=sh_andor(c->and_or_list);shell_status=rc;if(rc==130){sh_flow=4;sh_flow_status=130;}}return rc;}
static int sh_source(const char *source){struct mrsh_parser *parser;struct mrsh_program *program;int rc;size_t n=strlen(source);
if(sh_depth>=SH_DEPTH||sh_source_depth>=8)return sh_fail("source nesting limit");
#ifndef SH_HOST_TEST
 if(!native_heap_room(0,16384L,1))return sh_fail("insufficient parser working memory");
#endif
 sh_source_depth++;parser=mrsh_parser_with_data(source,n);rc=0;
 /* mrsh may finish a program at a consumed here-document terminator. Keep
    parsing the remaining source, rather than silently dropping later commands. */
 while(!sh_flow){program=mrsh_parse_program(parser);if(mrsh_parser_error(parser,0)){rc=sh_fail(mrsh_parser_error(parser,0));mrsh_program_destroy(program);break;}if(!program){rc=sh_fail("parser allocation failed");break;}int empty=!program->body.len;rc=sh_array(&program->body);mrsh_program_destroy(program);if(mrsh_parser_eof(parser))break;if(empty){rc=sh_fail("unconsumed source");break;}}
 mrsh_parser_destroy(parser);sh_source_depth--;if(sh_flow==3||sh_flow==4)rc=sh_flow_status;return rc;}
int shell_run(const char *source){int rc;history_add(source);sh_flow=0;output_error=0;rc=sh_source(source);if(sh_flow==3||sh_flow==4)rc=sh_flow_status;sh_flow=0;shell_status=rc;return rc;}

#ifndef SH_HOST_TEST
void shell_profile(void){char native[PATH_CAP];CInfoPBRec pb;if(resolve("/etc/profile",native)||info(native,&pb)){term_puts("Profile unavailable; using built-in defaults.\n");return;}char *args[]={"/etc/profile"};int rc=sh_file(args[0],1,args,1);if(rc==130)term_puts("Profile interrupted; built-ins remain available.\n");else if(rc)term_puts("Profile returned an error; built-ins remain available.\n");sh_flow=0;shell_status=rc;}
#endif
