/* Ownership transfer at command boundaries, using the existing shell state.
   No second parser, process model, or environment copy on each command. */
typedef struct {
 struct sh_saved shell;
 ClariceSessionIO io;
 int closed,status,flow,flow_status,depth,source_depth,loop,functions_active;
 int substatus,script_depth,output_error,history_enabled;
 unsigned long substitutions,commands,history_count;
 short input,output,error,history;
} ShellSession;
static ShellSession *remote_session,*local_session;
static int session_ui;
static void session_capture(ShellSession *s){
 s->shell.env=sh_env;s->shell.env_defaults=sh_env_defaults;s->shell.args=sh_owned_args;s->shell.argv=sh_argv;s->shell.argc=sh_argc;s->shell.functions=sh_functions;
 strcpy(s->shell.cwd,shell_cwd);
#ifndef SH_HOST_TEST
 strcpy(s->shell.unixdir,unix_cwd);
#endif
 s->status=shell_status;s->flow=sh_flow;s->flow_status=sh_flow_status;s->depth=sh_depth;s->source_depth=sh_source_depth;s->loop=sh_loop;s->functions_active=sh_functions_active;s->substatus=sh_substatus;s->substitutions=sh_substitutions;
 s->script_depth=script_depth;s->output_error=output_error;
 s->input=input_ref;s->output=output_ref;s->error=error_ref;
#ifndef SH_HOST_TEST
 s->commands=shell_commands;s->history=history_ref;s->history_count=history_count;s->history_enabled=history_enabled;
#endif
}
static void session_activate(const ShellSession *s){
 sh_env=s->shell.env;sh_env_defaults=s->shell.env_defaults;sh_owned_args=s->shell.args;sh_argv=s->shell.argv;sh_argc=s->shell.argc;sh_functions=s->shell.functions;
 strcpy(shell_cwd,s->shell.cwd);
#ifndef SH_HOST_TEST
 strcpy(unix_cwd,s->shell.unixdir);
#endif
 shell_status=s->status;sh_flow=s->flow;sh_flow_status=s->flow_status;sh_depth=s->depth;sh_source_depth=s->source_depth;sh_loop=s->loop;sh_functions_active=s->functions_active;sh_substatus=s->substatus;sh_substitutions=s->substitutions;
 script_depth=s->script_depth;output_error=s->output_error;
 input_ref=s->input;output_ref=s->output;error_ref=s->error;
#ifndef SH_HOST_TEST
 shell_commands=s->commands;history_ref=s->history;history_count=s->history_count;history_enabled=s->history_enabled;
#endif
}
void *cli_session_open(const ClariceSessionIO *io){
 if(remote_session||!io||!io->read||!io->write||!io->cancelled)return 0;
 ShellSession *s=calloc(1,sizeof *s);if(!s)return 0;
 s->io=*io;s->input=s->output=s->history=-1;s->error=-3;s->shell.env_defaults=sh_env_defaults;
 /* A fresh supplementary shell inherits exported values, not local variables
    or function definitions. Its directory starts at Clarice's root. */
 struct sh_env **tail=&s->shell.env;
 for(struct sh_env *e=sh_env;e;e=e->next)if((e->flags&(SH_ENV_SET|SH_ENV_EXPORT))==(SH_ENV_SET|SH_ENV_EXPORT)){
  *tail=env_new(e->name,strlen(e->name),e->value);if(!*tail)goto fail;(*tail)->flags=e->flags;tail=&(*tail)->next;
 }
 s->shell.argc=1;s->shell.args=sh_positions_copy(1,sh_initial_argv,&s->shell.argv);if(!s->shell.args)goto fail;
#ifndef SH_HOST_TEST
 strcpy(s->shell.cwd,root_path);strcpy(s->shell.unixdir,"/");
#else
 strcpy(s->shell.cwd,home_path);
#endif
 remote_session=s;return s;
fail:env_free(s->shell.env);free(s);return 0;
}
int cli_session_run(void *opaque,const char *command){
 ShellSession *s=opaque;if(!s||s!=remote_session||local_session||s->closed||!command)return 125;
 ShellSession local;session_capture(&local);local_session=&local;session_activate(s);session_io=&s->io;
 session_exit_requested=0;int rc=shell_run(command);s->closed=session_exit_requested;
 session_capture(s);session_io=0;session_activate(&local);local_session=0;return rc|(s->closed?256:0);
}
int cli_session_close(void *opaque){
 ShellSession *s=opaque;if(!s||s!=remote_session||local_session)return -1;
 env_free(s->shell.env);sh_functions_free(s->shell.functions);free(s->shell.args);free(s);remote_session=0;return 0;
}
int shell_session_active(void){return local_session!=0;}
int shell_session_ui_begin(void){
 if(!local_session||session_ui)return 0;
 session_capture(remote_session);session_activate(local_session);session_io=0;session_ui=1;return 1;
}
void shell_session_ui_end(int entered){if(entered){session_capture(local_session);session_activate(remote_session);session_io=&remote_session->io;session_ui=0;}}
void cli_session_prompt(void *opaque,char *out,unsigned long cap){
 ShellSession *s=opaque;if(!cap)return;out[0]=0;if(s!=remote_session||local_session)return;
#ifndef SH_HOST_TEST
 const char *d=strrchr(s->shell.unixdir,'/');d=d&&d[1]?d+1:"/";snprintf(out,cap,"%s > ",d);
#else
 snprintf(out,cap,"> ");
#endif
}
int shell_session_cancelled(void){return session_io?session_io->cancelled(session_io->context):0;}

int shell_session_remote_io(void){return session_io!=0;}
int shell_session_output(const char *data,long n){
 if(!session_io)return 0;
 if(n>0&&session_io->write(session_io->context,1,data,n)!=n)output_error=1;
 return 1;
}

#ifndef SH_HOST_TEST
/* Rebase only on successful native directory mutations, including suspended
   subshell/local/remote contexts. No repeated path walk or resident cache. */
static int relocate_cwd_pair(char *native,char *unixdir,const char *oldpath,const char *newpath,int commit){
 size_t n=strlen(oldpath),m=strlen(newpath);char next[PATH_CAP],canonical[PATH_CAP];
 if(strncasecmp(native,oldpath,n)|| (native[n]&&native[n]!=':'))return 0;
 /* Native Pascal paths are limited to 255 bytes, even though the Unix
    namespace buffer permits longer escaped spellings. */
 if(m+strlen(native+n)>255)return bdNamErr;
 memcpy(next,newpath,m);strcpy(next+m,native+n);
 int e=unix_from_native(next,canonical);if(e)return e;
 if(commit){strcpy(native,next);strcpy(unixdir,canonical);}return 0;
}
static int shell_relocate_cwd(const char *oldpath,const char *newpath,int commit){
 int e=relocate_cwd_pair(shell_cwd,unix_cwd,oldpath,newpath,commit);if(e)return e;
 for(struct sh_saved *s=sh_saved_frames;s;s=s->saved_next){e=relocate_cwd_pair(s->cwd,s->unixdir,oldpath,newpath,commit);if(e)return e;}
 if(local_session){e=relocate_cwd_pair(local_session->shell.cwd,local_session->shell.unixdir,oldpath,newpath,commit);if(e)return e;}
 if(remote_session){e=relocate_cwd_pair(remote_session->shell.cwd,remote_session->shell.unixdir,oldpath,newpath,commit);if(e)return e;}
 return 0;
}
#endif
