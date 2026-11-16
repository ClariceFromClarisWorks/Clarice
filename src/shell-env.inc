/* Invocation-visible shell variables. One allocation per value, no spare slots.
 * Replacement is staged before unlinking the old value; restoration transfers
 * owned nodes and cannot fail while unwinding an execution scope. */
enum { SH_ENV_SET=1, SH_ENV_EXPORT=2 };
struct sh_env {struct sh_env *next;char *value;unsigned char flags;char name[];};
static struct sh_env *sh_env;
static struct sh_env *env_find(struct sh_env *list,const char *name){for(;list;list=list->next)if(!strcmp(list->name,name))return list;return 0;}
static int env_name(const char *s,size_t n){if(!n)return 0;for(size_t i=0;i<n;i++)if(!(isalnum((unsigned char)s[i])||s[i]=='_')||(!i&&isdigit((unsigned char)s[i])))return 0;return 1;}
static struct sh_env *env_new(const char *name,size_t n,const char *value){size_t v=strlen(value);struct sh_env *e;if(!env_name(name,n)||n>(size_t)-1-sizeof *e-2||v>(size_t)-1-sizeof *e-n-2)return 0;e=malloc(sizeof *e+n+v+2);if(!e)return 0;e->next=0;e->flags=SH_ENV_SET;memcpy(e->name,name,n);e->name[n]=0;e->value=e->name+n+1;memcpy(e->value,value,v+1);return e;}
static struct sh_env *env_take(struct sh_env **list,const char *name){while(*list){struct sh_env *e=*list;if(!strcmp(e->name,name)){*list=e->next;e->next=0;return e;}list=&e->next;}return 0;}
static void env_put(struct sh_env **list,struct sh_env *e){struct sh_env *old=env_take(list,e->name);if(old)e->flags|=old->flags&SH_ENV_EXPORT;free(old);e->next=*list;*list=e;}
static void env_free(struct sh_env *list){while(list){struct sh_env *next=list->next;free(list);list=next;}}
static int env_clone(struct sh_env *list,struct sh_env **out){struct sh_env **tail=out;*out=0;for(;list;list=list->next){*tail=env_new(list->name,strlen(list->name),list->value);if(!*tail){env_free(*out);*out=0;return -1;}(*tail)->flags=list->flags;tail=&(*tail)->next;}return 0;}
static int env_assign(const char *name,const char *value){struct sh_env *e=env_new(name,strlen(name),value);if(!e)return -1;env_put(&sh_env,e);return 0;}
static int env_set(const char *s){const char *eq=strchr(s,'=');size_t n=eq?(size_t)(eq-s):strlen(s);if(!env_name(s,n))return -1;struct sh_env *e;if(!eq&&(e=env_find(sh_env,s))){e->flags|=SH_ENV_EXPORT;return 0;}e=env_new(s,n,eq?eq+1:"");if(!e)return -1;e->flags=SH_ENV_EXPORT|(eq?SH_ENV_SET:0);env_put(&sh_env,e);return 0;}
