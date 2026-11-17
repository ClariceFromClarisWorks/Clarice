#define SH_SESSION_HOST_TEST 1
#include "../../src/session.h"
static const ClariceSessionIO *session_io;
static int session_exit_requested;
#define main existing_shell_main
#include "../shell/host.c"
#undef main
#include "../../src/session.inc"
#include <assert.h>
struct IO {char out[4096];size_t used;const char *input;int cancelled,fail,check_ui;};
static long read_remote(void *v,void *p,unsigned long n){struct IO *io=v;size_t len=strlen(io->input);if(n>len)n=len;memcpy(p,io->input,n);io->input+=n;return n;}
static long write_remote(void *v,int fd,const void *p,unsigned long n){struct IO *io=v;(void)fd;if(io->check_ui){io->check_ui=0;int entered=shell_session_ui_begin();assert(entered);assert(!strcmp(env_get("SHARED"),"exported"));assert(!strcmp(sh_argv[1],"local"));assert(!shell_session_ui_begin());shell_session_ui_end(entered);assert(!strcmp(env_get("SHARED"),"remote"));}if(io->fail)return -1;if(n>sizeof io->out-io->used-1)return -1;memcpy(io->out+io->used,p,n);io->used+=n;io->out[io->used]=0;return n;}
static int cancel_remote(void *v){return ((struct IO*)v)->cancelled;}
int main(void){
 struct IO io={.input="hello remote\n"};ClariceSessionIO ops={&io,read_remote,write_remote,cancel_remote};
 assert(!shell_run("export SHARED=exported; LOCAL=private; localfn() { echo local; }; set -- local arguments"));
 size_t baseline=host_live;void *s=cli_session_open(&ops);assert(s&&!cli_session_open(&ops));
 assert(!cli_session_run(s,"printf '%s:%s\\n' \"$SHARED\" \"${LOCAL-unset}\"; SHARED=remote; remotefn() { printf remote; }; set -- remote args"));
 assert(!strcmp(io.out,"exported:unset\n"));io.used=0;io.out[0]=0;
 io.check_ui=1;assert(!cli_session_run(s,"remotefn; printf ':%s:%s\\n' \"$SHARED\" \"$1\"; read a b; printf '%s/%s\\n' \"$a\" \"$b\""));
 assert(!strcmp(io.out,"remote:remote:remote\nhello/remote\n"));assert(!strcmp(env_get("SHARED"),"exported")&&!strcmp(env_get("LOCAL"),"private"));assert(!strcmp(sh_argv[1],"local"));
 io.fail=1;assert(cli_session_run(s,"echo cannot-write"));io.fail=0;assert(!output_error&&input_ref==-1&&output_ref==-1&&error_ref==-3);
 assert(cli_session_run(s,"if")!=0);assert(!output_error);
 io.cancelled=1;assert(cli_session_run(s,"echo must-not-run")==130);
 io.cancelled=0;assert(!cli_session_run(s,"true"));assert(!shell_run("true"));
 size_t live=host_live;
 for(unsigned i=1;i<=80;i++){
  char n[32];snprintf(n,sizeof n,"%lu",(unsigned long)host_calls+i);
  setenv("CLARICE_SHELL_FAIL_ALLOC",n,1);
  cli_session_run(s,"(printf '%s' \"$(printf nested)\")");
  unsetenv("CLARICE_SHELL_FAIL_ALLOC");io.used=0;io.out[0]=0;
  assert(host_live==live);assert(!cli_session_run(s,"true"));
 }

 io.used=0;io.out[0]=0;
 assert(!cli_session_run(s,"(exit 3); sh -c 'exit 4'; value=$(exit 5); printf alive"));
 assert(!strcmp(io.out,"alive"));
 assert(!cli_session_run(s,"true"));
 assert(cli_session_run(s,"exit 7")==263);assert(cli_session_run(s,"echo after-exit")==125);assert(!cli_session_close(s));assert(host_live==baseline);
 for(unsigned i=1;i<=12;i++){char n[32];snprintf(n,sizeof n,"%lu",(unsigned long)host_calls+i);setenv("CLARICE_SHELL_FAIL_ALLOC",n,1);s=cli_session_open(&ops);unsetenv("CLARICE_SHELL_FAIL_ALLOC");if(s)assert(!cli_session_close(s));assert(host_live==baseline);}
 sh_functions_free(sh_functions);env_free(sh_env);free(sh_owned_args);assert(!host_live);for(int i=0;i<128;i++)assert(!refs[i]);
 puts("PASS independent session exports, variables, functions, arguments, read/write, exit, parser/write failure, allocation exhaustion and cleanup");return 0;
}
