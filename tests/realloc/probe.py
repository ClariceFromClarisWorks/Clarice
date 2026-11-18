from pathlib import Path
import sys,time
r=Path(__file__).resolve().parents[2];sys.path.insert(0,str(r/'tests/remote-sessions'));from client import Client
c=Client(port=23245,password=(r.parent/'Clarice-Main/work/remote-sessions/test-password').read_text().strip());data=b''
try:
 c.t.write(b"alloctest; printf '\\nRESULT=%s\\n' $?\r\n")
 for i in range(600):
  data+=c.t.read_very_eager();(r/'outputs/realloc/capacity-probe.txt').write_bytes(data)
  if b'\r\nRESULT=' in data and data.endswith(b' > '):break
  time.sleep(.2)
 else:raise RuntimeError('Guest did not return within 120 seconds; partial transcript retained')
finally:c.close()
print(data.decode('latin1'))
