"""Usage: python3 measure.py before|after SNOW_PID EXISTING_KEY_TOOL
Run after installing each variant as nano-profile and disabling services.
Uses prepared /tmp/nempty, nshort and nlong fixtures; never saves edits.
"""
from pathlib import Path
import subprocess,time
import sys
pid=sys.argv[2]
keytool=sys.argv[3]
def type(s):
 p=Path('/tmp/nano-paired-input');p.write_bytes(s.encode());subprocess.run([keytool,pid,str(p),'.025'],check=True,stdout=subprocess.DEVNULL)
type('\x18');time.sleep(.6)
for cmd in ['nano-profile']:
 type(cmd+'\r');time.sleep(3);type('\x18');time.sleep(.5)
for label,path in [('buffer',''),('empty',' /tmp/nempty'),('short',' /tmp/nshort'),('long',' /tmp/nlong')]:
 for i in range(3):
  for variant,cmd in [( sys.argv[1], 'nano-profile')]:
   type(cmd+path+'\r');time.sleep(3);type('\x18');time.sleep(.5)
   type('status runtime > /tmp/ns-'+label+'-'+variant+str(i)+'\r');time.sleep(.3)
   print(label,variant,i,flush=True)
type('telnetd\r')
