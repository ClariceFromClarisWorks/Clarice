from pathlib import Path
import subprocess,time,sys
pid,label=sys.argv[1:]
def send(data):
 Path('work/nano-perf/input').write_bytes(data);subprocess.run(['work/safe-save/keys',pid,'work/nano-perf/input','.12'],check=True,stdout=subprocess.DEVNULL)
for index,path in enumerate(['/tmp/eight.txt','/tmp/eight.txt']):
 send(('nano '+path+'\r').encode());time.sleep(20)
 send(b'z');time.sleep(2)
 subprocess.run(['screencapture','-x','-R220,52,1000,782','outputs/nano-perf/'+label+str(index)+'.png'],check=True)
 # Cancel so measured fixtures stay unchanged.
 send(b'\x1c');time.sleep(2)
 send(('echo $? > /tmp/'+label+str(index)+'\rstatus runtime >> /tmp/'+label+str(index)+'\r').encode());time.sleep(1)
print('DONE '+label)
