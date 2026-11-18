"""Run production recursive removal against disposable dot/dotdot sentinels."""
from pathlib import Path
import subprocess,sys
r=Path.cwd();text=(r/'tests/utility-correctness/file-depth.py').read_text()
# Reuse its native-boundary fixture without running its original main.
ns={'__file__':str(r/'tests/utility-correctness/file-depth.py')};exec(text[:text.index('tests=r')],ns)
s=(r/'src/commands/files.c').read_text();parts=s[s.index('typedef struct RemoveFrame'):s.index('static int temporary_sibling')]
test=r'''
int main(void){char base[]="/tmp/clarice-operands-XXXXXX";assert(mkdtemp(base));assert(!chdir(base));char *bad[]={"tree/.","tree/.///","tree/sub/..","tree/sub/..///",".",".."};
for(unsigned i=0;i<sizeof bad/sizeof *bad;i++){assert(!mkdir("tree",0700));assert(!mkdir("tree/sub",0700));FILE*f=fopen("tree/sub/sentinel","w");fputs("KEEP",f);fclose(f);int rc=remove_one(bad[i],0,0);assert(rc==paramErr);assert(!access("tree/sub/sentinel",F_OK));assert(!live);assert(!remove_one("tree",0,0));}
assert(!chdir("/"));assert(!rmdir(base));puts("PASS rm dot/dotdot operands rejected before traversal, slash suffixes, sentinels retained, normal removal unaffected");}
'''
w=r/'work/filesystem-confidence';w.mkdir(parents=True,exist_ok=True);(w/'remove-operands.c').write_text(ns['stub']+parts+test)
subprocess.run(['cc','-g','-O1','-fsanitize=address,undefined',str(w/'remove-operands.c'),'-o',str(w/'remove-operands')],check=True)
subprocess.run([str(w/'remove-operands')],check=True)
