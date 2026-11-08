"""Exercise production redirection and stream code with host File Manager shims."""
import subprocess,tempfile,pathlib,json,os
cases=[
("echo alpha > in; cat < in | cat > out; cat out",'alpha\n',''),
("echo a >&2 2> err; cat err",'','a\n'),
("echo alpha > in; cat in 1>&-; echo $?; echo after",'1\nafter\n',''),
("echo out 2> err 1>&2; cat err",'out\n',''),
("echo out > combined 2>&1; cat combined",'out\n',''),
("echo out 1>&-; echo $?; echo restored",'1\nrestored\n',''),
("echo out 2>&- 1>&2; echo $?",'1\n',''),
("false | cat; echo $?; true | false; echo $?; ! false | cat; echo $?",'0\n1\n1\n',''),
("echo out >&- | cat; echo $?",'0\n',''),
("echo first > file; echo second >> file; cat file",'first\nsecond\n',''),
("echo word > file; read x < file; echo \"$x\"",'word\n','')]
r=[];exe=str(pathlib.Path('work/sh-host').resolve())
for source,out,err in cases:
 with tempfile.TemporaryDirectory() as d:
  a=subprocess.run([exe,source],cwd=d,capture_output=True,text=True,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0'},timeout=5)
 r.append(dict(source=source,actual=a.stdout,stderr=a.stderr,status=a.returncode,passed=a.stdout==out and a.stderr==err and a.returncode==0))
pathlib.Path('outputs/external-tools/host-streams.json').write_text(json.dumps(r,indent=2))
assert all(x['passed'] for x in r),r
print(f'{len(r)} stream/redirection cases passed')
