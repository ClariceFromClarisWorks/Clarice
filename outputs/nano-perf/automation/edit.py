from pathlib import Path
import subprocess,time,sys
pid=sys.argv[1]
def send(b):
 Path('work/nano-perf/input').write_bytes(b);subprocess.run(['work/nano-perf/keys',pid,'work/nano-perf/input','.12'],check=True,stdout=subprocess.DEVNULL)
def shot(name):subprocess.run(['screencapture','-x','-R220,52,1000,782','outputs/nano-perf/'+name+'.png'],check=True)
send(b'nano /tmp/eight.txt\r');time.sleep(21)
send(b'performance test ');time.sleep(2);shot('typing')
send(b'\x0f');time.sleep(1);send(b'\r');time.sleep(5);shot('saved')
send(b'\x01'*40);time.sleep(3);shot('scrolling')
send(b'\x17');time.sleep(1);send(b'trial\r');time.sleep(3);shot('search')
send(b'\x18');time.sleep(3)
send(b'echo $? > /tmp/edit\rstatus runtime >> /tmp/edit\r');time.sleep(2)
send(b'nano /tmp/save-note\r');time.sleep(20);send(b'perf ');send(b'\x0f');time.sleep(1);send(b'\r');time.sleep(3);send(b'\x18');time.sleep(3)
send(b'echo $? > /tmp/save\rstatus runtime >> /tmp/save\r');time.sleep(2)
send(b'nano /tmp/eight.txt\r');time.sleep(1);send(b'\x1c');time.sleep(3)
send(b'echo $? > /tmp/cancel\rstatus runtime >> /tmp/cancel\r');time.sleep(2)
print('DONE editing')
