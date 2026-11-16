"""Exercise the production startup wrapper with the existing shell/IO harness."""
from pathlib import Path
import subprocess
r=Path(__file__).resolve().parents[2]; w=r/'work/utility-closeout';w.mkdir(exist_ok=True)
s=(r/'tests/shell/host.c').read_text().split('int main(int argc,char **argv){')[0]
s=s.replace('#include "alloc.h"', '#include "../../tests/shell/alloc.h"')
wrapper=(r/'src/sh.inc').read_text().split('void shell_profile(void){',1)[1].split('\n#endif',1)[0]
s+='\n#include <assert.h>\nstatic void term_puts(const char *s){say(s);}\nvoid shell_profile(void){'+wrapper.replace('/etc/profile',str(w/'profile'))+'\n'
s+=r'''
static void profile(const char *text){FILE *f=fopen("PROFILE_PATH","wb");assert(f);fputs(text,f);fclose(f);}
int main(void){
 assert(shell_run("sh -c 'while exit 7; do :; done'")==7);
 profile("false\n");shell_profile();assert(shell_status==1&&!sh_flow);
 profile("true\n");shell_profile();assert(shell_status==0&&!sh_flow);
 profile("while return 7; do :; done\n");shell_profile();assert(shell_status==7&&!sh_flow);
 profile("while :; do :; done\necho BAD\n");
 for(int at=1;at<=80;at++){cancel_after=at;polls=0;shell_profile();assert(shell_status==130&&!sh_flow);cancel_after=polls=0;assert(!shell_run("true"));}
 assert(!shell_run("true")&&!shell_status);
 profile("echo unreachable\n");char fail[32];snprintf(fail,sizeof fail,"%lu",(unsigned long)host_calls+1);
 setenv("CLARICE_SHELL_FAIL_ALLOC",fail,1);shell_profile();assert(shell_status==2&&!sh_flow);
 unsetenv("CLARICE_SHELL_FAIL_ALLOC");assert(!shell_run("true"));
 for(int i=0;i<128;i++)assert(!refs[i]);
 sh_functions_free(sh_functions);free(sh_owned_args);env_free(sh_env);assert(!host_live);
 puts("PASS: startup success/error/return/OOM status, 80 cancellation points, prompt recovery, stream and allocation cleanup");
}
'''.replace('PROFILE_PATH',str(w/'profile'))
p=w/'profile.c';p.write_text(s)
files=['ast.c','array.c','buffer.c','arithm.c','parser/parser.c','parser/program.c','parser/word.c','parser/arithm.c']
subprocess.run(['cc','-std=c99','-g','-fsanitize=address,undefined','-I',str(r/'src/vendor/mrsh/include'),str(p),*[str(r/'src/vendor/mrsh'/x) for x in files],'-o',str(w/'profile-test')],check=True)
subprocess.run([str(w/'profile-test')],check=True)

assert subprocess.run(["/bin/dash","-c","while exit 7; do :; done"]).returncode == 7
