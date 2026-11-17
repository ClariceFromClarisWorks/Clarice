from pathlib import Path
import subprocess,time,sys
pid=sys.argv[1]
def send(b):
 Path('work/nano-perf/input').write_bytes(b);subprocess.run(['work/nano-perf/keys',pid,'work/nano-perf/input','.12'],check=True,stdout=subprocess.DEVNULL)
for i in range(2):
 send(b'nano /tmp/eight.txt\r');time.sleep(25)
 if i==0:
  send(b'normal ');send(b'\x0f');time.sleep(1);send(b'\r');time.sleep(4)
  subprocess.run(['screencapture','-x','-R220,52,1000,782','outputs/nano-perf/normal-saved.png'],check=True)
 send(b'\x18');time.sleep(3)
 send(('echo $? > /tmp/normal'+str(i)+'\rstatus runtime >> /tmp/normal'+str(i)+'\r').encode());time.sleep(2)
print('DONE normal')
