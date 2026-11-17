#!/usr/bin/env python3
import os,pty,select,subprocess,sys,time,json,signal,tty
from pathlib import Path
root=Path(__file__).resolve().parents[2];out=root/'outputs/serial-2';out.mkdir(exist_ok=True)
m,s=pty.openpty();tty.setraw(s)
p=subprocess.Popen([sys.executable,str(root/'scripts/serial-shell.py'),os.ttyname(s),'--log',str(out/'host-shell.jsonl'),'--seconds','30'],stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
assert p.stdout.readline().startswith('READY')
transcript=bytearray();latencies=[]
def expect(text,timeout=4):
 end=time.monotonic()+timeout;buf=bytearray()
 while text not in buf:
  assert time.monotonic()<end,repr(buf)
  if select.select([m],[],[],.05)[0]:buf.extend(os.read(m,4096))
 transcript.extend(buf);return bytes(buf)
def command(data,expected):
 t=time.monotonic();os.write(m,data);b=expect(b'MACBOOK> ');assert expected in b,repr(b);latencies.append(time.monotonic()-t);return b
try:
 expect(b'MACBOOK> ')
 command(b'pwd\r',b'clarice-remote-')
 command(b'ls\r',b'sample.txt')
 b=command(b'echo hello\r',b'hello\r\n');assert b.count(b'echo hello')==1
 command(b'echo bad\x7f\x7f\x7fgood\r',b'good\r\n')
 os.write(m,b'sleep 20\r');time.sleep(.2);t=time.monotonic();os.write(m,b'\x03');expect(b'MACBOOK> ');latencies.append(time.monotonic()-t)
 command(b'cat sample.txt\r',b'remote fixture')
 command(b'echo $TERM\r',b'dumb\r\n')
 command(b'cd /\r',b'restricted')
 os.write(m,b'exit\r');assert p.wait(timeout=4)==0,p.stderr.read()
finally:
 if p.poll() is None:p.send_signal(signal.SIGINT);p.wait(timeout=4)
 os.close(m);os.close(s)
(out/'host-shell-transcript.bin').write_bytes(transcript)
(out/'host-shell-results.json').write_text(json.dumps({'pass':True,'prompt_latency_seconds':latencies,'max':max(latencies)},indent=2)+'\n')
print('PASS: real PTY shell prompt, pwd/ls/echo/cat, erase, remote Ctrl+C, single echo, TERM=dumb, restricted cd, exit; max prompt response %.3fs'%max(latencies))
