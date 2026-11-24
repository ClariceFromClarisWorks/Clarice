from pathlib import Path
import json
import re
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
source = (root/'src/help.inc').read_text()
literal = r'"(?:[^"\\]|\\.)*"'
rows = {json.loads(name): ''.join(map(json.loads, re.findall(literal, body)))
        for name, body in re.findall(r'\{('+literal+r'),\s*((?:'+literal+r'\s*)+)\}', source)}
assert set(rows) == set('cd pwd echo printf export unset set shift read sh . test [ true false : break continue return exit history prefs status clear selftest checks scrollbench env time command clarice appinstall remote help'.split())
for name, words in {'read':['-r','IFS','grow','NUL'], 'export':['-p','memory'],
                    'unset':['-v','-f'], 'printf':['%b','%X','32-bit'],
                    'test':['-L','-h','-ge'], 'prefs':['scrollback','control','columns']}.items():
    assert all(word in rows[name] for word in words), name

harness = r'''
#include <stdlib.h>
#include "help-output.h"
static int width, cancel, fail, chunks;
static long capture(int fd,const void *p,unsigned long n){
 if(fail)return n-1;
 ++chunks;
 if(n>264)abort();
 return help_host_write(fd,p,n);
}
static int poll(void){return cancel&&chunks>=cancel;}
static int mock_help(int fd,const char *text){return help_format(width,fd,text,capture,poll);}
#define help_output mock_help
static void errsay(const char *s){fputs(s,stderr);}
#include "help.inc"
int main(int argc,char **argv){
 width=atoi(argv[1]);cancel=atoi(argv[2]);fail=atoi(argv[3]);
 if(argc==5&&!strcmp(argv[4],"@long"))
  return help_output(1,"  --an-unusually-long-option-name\tOne two three four five.\n\n"
                       "  AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA xyz\n");
 return help_command(argc-3,argv+3);
}
'''
with tempfile.TemporaryDirectory() as d:
    path = Path(d)
    (path/'test.c').write_text(harness)
    subprocess.run(['cc','-std=c99','-fsanitize=address,undefined','-I',str(root/'src'),
                    str(path/'test.c'),'-o',str(path/'test')], check=True)

    def run(width, *args, cancel=0, fail=0):
        return subprocess.run([str(path/'test'), str(width), str(cancel), str(fail), *args],
                              capture_output=True, text=True, timeout=5)

    for width in (40, 50, 66, 80, 130):
        for name, original in rows.items():
            q = run(width, name)
            assert q.returncode == 0 and not q.stderr, (name, width, q)
            assert q.stdout.split() == original.split(), (name, width, q.stdout)
            assert all(len(line) <= width and line == line.rstrip() for line in q.stdout.splitlines())
        listing = run(width)
        assert listing.returncode == 0 and not listing.stderr and 'Standard commands:' in listing.stdout
        assert max(map(len, listing.stdout.splitlines())) <= width
        assert run(width, '--help').stdout == run(width, 'help').stdout

    narrow = run(40, 'env').stdout
    assert '  -u NAME\n    Remove a shell identifier from the' in narrow
    assert '  -u NAME       Remove' in run(66, 'env').stdout
    for width in (1, 7, 26, 40, 50, 66, 80, 130):
        q = run(width, '@long')
        assert q.returncode == 0 and not q.stderr
        assert all(len(line) <= width and line == line.rstrip() for line in q.stdout.splitlines())
        assert ''.join(q.stdout.split()) == '--an-unusually-long-option-nameOnetwothreefourfive.'+'A'*64+'xyz'
    q = run(66, 'missing')
    assert q.returncode == 1 and q.stderr and not q.stdout
    q = run(66, 'read', 'sh')
    assert q.returncode == 2 and not q.stdout and not q.stderr
    q = run(66, 'prefs', cancel=1)
    assert q.returncode == 130 and 0 < len(q.stdout) < len(run(66, 'prefs').stdout) and not q.stderr
    q = run(66, 'prefs', fail=1)
    assert q.returncode == 1 and not q.stdout and not q.stderr
print(f'PASS: {len(rows)} builtin topics at five widths, indentation, word boundaries, content, streams, cancellation and output failure; ASan/UBSan')
