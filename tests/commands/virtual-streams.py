from pathlib import Path
import subprocess,tempfile
r=Path(__file__).resolve().parents[2]
code=r'''
#include <assert.h>
#include <limits.h>
#include "virtual-stream.h"
int main(void){char b[258];
 assert(!vstream_code(INT_MIN)&&!vstream_code(INT_MAX)&&!vstream_code(-1)&&!vstream_code(-2)&&!vstream_code(-3));
 for(int kind=1;kind<=2;kind++)for(int mode=1;mode<=3;mode++)for(int form=0;form<2;form++){
 int code=kind*4+mode,ref=form?VSTREAM_TAG+code:-code;
 memset(b,0x5a,sizeof b);assert(vstream_code(ref)==code);
 long n=vstream_read(ref,b+1,256);
 if(mode&1){assert(n==(kind==1?0:256));if(kind==2)for(int i=1;i<=256;i++)assert(!b[i]);}
 else assert(n==-1&&errno==EBADF);
 assert(b[0]==0x5a&&b[257]==0x5a);
 n=vstream_write(ref,256);assert(n==((mode&2)?256:-1));
 }
 assert(vstream_kind("/dev/null")==1&&vstream_kind("/dev/zero")==2);
 assert(!vstream_kind("/dev/nullx"));assert(vstream_read(-9,b,0x80000000UL)==-1);
 return 0;}
'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.c').write_text(code);subprocess.run(['cc','-fsanitize=address,undefined','-I'+str(r/'src'),str(p/'test.c'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True)
for source in ['echo ignored > /dev/null; echo visible', 'read x < /dev/null; echo $?', '{ echo no; echo no; } > /dev/null; echo yes', 'printf no 2> /dev/null; echo yes', 'read x < /dev/null; echo restored']:
 a=subprocess.run([str(r/'work/sh-host'),source],capture_output=True);b=subprocess.run(['dash','-c',source],capture_output=True);assert (a.returncode,a.stdout)==(b.returncode,b.stdout),(source,a,b)
print('PASS: virtual stream mode/token/boundary tests ASan/UBSan; five shell redirection/restoration comparisons')
