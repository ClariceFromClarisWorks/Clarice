from pathlib import Path
import subprocess,os
r=Path(__file__).resolve().parents[2]
cases=["env -i", "env -i A=one B='two words'", "export A=parent; env -i A=child sh -c 'echo $A'; echo $A", "export A=parent; env -u A sh -c 'echo ${A-unset}'; echo $A", "env -i PATH=/bin sh -c 'echo ready'", "env -i false; echo $?", "env -i missing_command; echo $?", "env -i A= sh -c 'printf \"<%s>\\n\" \"$A\"'", "env -i A=one env -i B=two; echo done"]
for source in cases:
 a=subprocess.run([str(r/'work/sh-host'),source],env={**os.environ,'CLARICE_SHELL_ALLOC_STATS':'1'},capture_output=True);b=subprocess.run(['dash','-c',source],capture_output=True)
 if source=="env -i A=one B='two words'":assert a.returncode==b.returncode and sorted(a.stdout.splitlines())==sorted(b.stdout.splitlines()),(a,b)
 else:assert (a.returncode,a.stdout)==(b.returncode,b.stdout),(source,a,b)
 assert b'remaining=0' in a.stderr,a.stderr
for source,status in [('time false',1),('time true',0),('time --bad',2),('env --bad',2),('env -i exit 7; echo alive',0)]:
 a=subprocess.run([str(r/'work/sh-host'),source],env={**os.environ,'CLARICE_SHELL_ALLOC_STATS':'1'},capture_output=True);assert a.returncode==status and b'remaining=0' in a.stderr,a
 if source.startswith('time ') and status!=2:assert b'real ' in a.stderr
# Exercise failure while constructing/restoring a scoped environment.
hits=0
for point in range(1,150):
 e={**os.environ,'CLARICE_SHELL_ALLOC_STATS':'1','CLARICE_SHELL_FAIL_ALLOC':str(point),'CLARICE_SHELL_RECOVERY':'1'}
 a=subprocess.run([str(r/'work/sh-host'),"export A=outer; env -i A=inner sh -c 'echo $A'; echo $A"],env=e,capture_output=True)
 hits+=b'fault=1' in a.stderr
 assert b'AddressSanitizer' not in a.stderr and b'remaining=0' in a.stderr and b'STREAMS-RESTORED' in a.stdout,(point,a)
print(f'PASS: nine dash scope comparisons, timing/status/exit isolation; 149 recovery trials, {hits} injected failures')
