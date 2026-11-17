from pathlib import Path
import subprocess,time
pid='49284'
def send(b):
 Path('work/safe-save/input').write_bytes(b);subprocess.run(['work/safe-save/keys',pid,'work/safe-save/input','.12'],check=True,stdout=subprocess.DEVNULL)
send(b'\x18');time.sleep(2)
send(b'echo $? > /tmp/nano-one\rstatus runtime >> /tmp/nano-one\r');time.sleep(1)
send(b'nano /tmp/save-new\r');time.sleep(12)
send(b'new document\x0f');time.sleep(2);send(b'\r');time.sleep(3);send(b'\x18');time.sleep(2)
send(b'echo $? > /tmp/nano-two\rstatus runtime >> /tmp/nano-two\r');time.sleep(1)
send(b'saveprobe /tmp/probe-new > /tmp/probe-one\r');time.sleep(12)
send(b'echo $? >> /tmp/probe-one\rstatus runtime >> /tmp/probe-one\r');time.sleep(1)
send(b'saveprobe /tmp/probe-new > /tmp/probe-two\r');time.sleep(12)
send(b'echo $? >> /tmp/probe-two\rstatus runtime >> /tmp/probe-two\r');time.sleep(1)
print('DONE: inspect guest reports')
