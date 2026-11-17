from pathlib import Path
import subprocess,tempfile
r=Path(__file__).resolve().parents[2]
stub='''#include <stdio.h>
#include "hardware.h"
#include "memory-info.h"
long cli_write(int fd,const void *p,unsigned long n){return fwrite(p,1,n,fd==2?stderr:stdout);}
int cli_hardware(ClariceHardware *p,unsigned long n){(void)n;p->ram=4194304;return 0;}
int cli_memory(ClariceMemory *p,unsigned long n){(void)n;*p=(ClariceMemory){1048576,131072,65536,196608,131072};return 0;}
extern int clarice_main(int,char**);int main(int n,char**v){return clarice_main(n,v);}
'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'stub.c').write_text(stub);subprocess.run(['cc','-fsanitize=address,undefined','-I'+str(r/'src'),str(p/'stub.c'),str(r/'src/commands/free.c'),'-o',str(p/'free')],check=True)
 for args,word in [([],b'4096 KiB'),(['-b'],b'4194304 bytes'),(['-h'],b'4.00 MiB'),(['-b','-m'],b'4 MiB'),(['--help'],b'application heap')]:
  q=subprocess.run([str(p/'free'),*args],capture_output=True);assert q.returncode==0 and word in q.stdout and not q.stderr,q
 q=subprocess.run([str(p/'free'),'--bad'],capture_output=True);assert q.returncode==2 and q.stderr and not q.stdout
print('PASS: free formatting, default/last unit, stdout/error status; ASan/UBSan; hardware query mocked')
