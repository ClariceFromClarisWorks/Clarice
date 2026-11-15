"""Host daemon loopback fixture; does not claim guest MacTCP coverage."""
import json,socket,subprocess,sys,tempfile,time,unittest
from pathlib import Path
from host import Wire

class Session(unittest.TestCase):
 def test_pairing_disconnect_reconnect(self):
  with tempfile.TemporaryDirectory() as d:
   d=Path(d);token=d/'token';token.write_text('a'*32);unix=d/'control.sock'
   with socket.socket() as probe:
    probe.bind(('127.0.0.1',0));port=probe.getsockname()[1]
   p=subprocess.Popen([sys.executable,str(Path(__file__).with_name('control.py')),'--socket',str(unix),'serve','--bind','127.0.0.1','--peer','127.0.0.1','--port',str(port),'--token-file',str(token)],stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
   def rpc(op,**kwargs):
    with socket.socket(socket.AF_UNIX) as c:
     c.settimeout(3);c.connect(str(unix));c.sendall((json.dumps({'op':op,**kwargs})+'\n').encode());return json.loads(c.recv(4096))
   try:
    self.assertIn('listening',p.stdout.readline())
    self.assertFalse(rpc('status')['ok'])
    with socket.create_connection(('127.0.0.1',port),3) as bad:
     bad.sendall(b'HIL2 bad\n');self.assertEqual(bad.recv(1),b'')
    self.assertIn('rejected',p.stdout.readline())
    for _ in range(2):
     with socket.create_connection(('127.0.0.1',port),3) as guest:
      guest.sendall(b'HIL2 '+b'a'*32+b'\n')
      self.assertIn('connected',p.stdout.readline())
      self.assertFalse(rpc('quiet',seconds=181)['ok'])
      self.assertFalse(rpc('quiet',seconds=True)['ok'])
      self.assertEqual(rpc('quiet',seconds=120)['quiet_seconds'],120)
      guest.settimeout(.03)
      with self.assertRaises(socket.timeout):guest.recv(1)
      self.assertEqual(rpc('quiet',seconds=0)['quiet_seconds'],0)
      self.assertTrue(rpc('disconnect')['ok'])
      self.assertEqual(guest.recv(1),b'')
    self.assertTrue(rpc('stop')['ok']);p.wait(timeout=3)
    self.assertEqual(p.returncode,0);self.assertFalse(unix.exists())
   finally:
    if p.poll() is None:p.kill();p.wait()
    p.stdout.close();p.stderr.close()
if __name__=='__main__':unittest.main()
