"""Host shell semantics; native HFS/CLX resolution is covered in Snow evidence."""
import subprocess
for source in ['command -v printf','command -v read','f() { :; }; command -v f','command -v for','command -v no_such_command','command -v -- echo']:
 a=subprocess.run(['work/sh-host',source],capture_output=True);b=subprocess.run(['dash','-c',source],capture_output=True);assert (a.returncode,a.stdout)==(b.returncode,b.stdout),(source,a,b)
print('PASS: six command -v comparisons against dash')
