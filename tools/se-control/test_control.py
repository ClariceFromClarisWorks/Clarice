import unittest
from unittest.mock import patch
import control

class Fake:
    def __init__(self, replies): self.replies=iter(replies); self.commands=[]
    def send(self,s): self.commands.append(s)
    def line(self): return next(self.replies)

IDLE='PONG 100 25000 bg=1 active=0 down=0 error=0 state=314'
class Commands(unittest.TestCase):
    def test_keys(self):
        self.assertEqual(control.key_spec('cmd+q'),(113,12,256))
        self.assertEqual(control.key_spec('ctrl+c'),(3,8,4096))
        for c in 'Hello, world! ~_+{}|:"<>?@#$%^&*()':
            ch,code,mods=control.character(c)
            self.assertEqual(ch,ord(c));self.assertLess(code,128)
        self.assertEqual(control.character('\n'),(13,36,0))
    def test_validate_before_sending(self):
        for req in ({'op':'type','text':'abc\u2603'}, {'op':'type','text':'x'*1025},
                    {'op':'click','x':-1,'y':20},
                    {'op':'drag','x':1,'y':2,'x2':3,'y2':4,'ticks':181}):
            w=Fake([])
            with self.assertRaises(ValueError): control.execute(w,req)
            self.assertEqual(w.commands,[])
    def test_fast_batches_without_submit(self):
        w=Fake(['CAPS keys=8 width=512 height=342','BUSY','QUEUED 8','QUEUED 1'])
        with patch('control.time.sleep'):
            result=control.execute(w,{'op':'type','text':'abcdefghi','fast':True})
        self.assertEqual(result['queued'],9)
        self.assertEqual(w.commands[0],'CAPS')
        self.assertEqual(w.commands[1],w.commands[2])
        self.assertEqual(len(w.commands[2]),69)
        self.assertEqual(len(w.commands[3]),13)
        for text in ('echo x\r','a\nb','abcdefgh\t'):
            w=Fake([])
            with self.assertRaises(ValueError):control.execute(w,{'op':'type','text':text,'fast':True})
            self.assertEqual(w.commands,[])
    def test_fast_partial_stops(self):
        w=Fake(['CAPS keys=8 width=512 height=342','ERR keys posted=2'])
        with self.assertRaisesRegex(RuntimeError,'posted=2'):
            control.execute(w,{'op':'type','text':'abcdefghi','fast':True})
        self.assertEqual(len(w.commands),2)
    def test_larger_screen_coordinates(self):
        w=Fake(['QUEUED',IDLE]);w.screen_size=(640,400)
        control.execute(w,{'op':'click','x':630,'y':390})
        self.assertEqual(w.commands[0],'CLICK 630 390 1')
    def test_finite_drag(self):
        w=Fake(['QUEUED',IDLE])
        s=control.execute(w,{'op':'drag','x':10,'y':20,'x2':100,'y2':200})
        self.assertEqual(w.commands,['DRAG 10 20 100 200 60','PING'])
        self.assertEqual(s['down'],0)
    def test_cancel(self):
        w=Fake(['CANCELLING',IDLE])
        self.assertEqual(control.execute(w,{'op':'cancel'})['active'],0)
    def test_error_not_success(self):
        w=Fake(['QUEUED',IDLE.replace('error=0','error=1')])
        with self.assertRaises(RuntimeError):control.execute(w,{'op':'move','x':1,'y':2})
    def test_partial_input_never_replayed(self):
        w=Fake(['QUEUED','ERR event post'])
        with patch('control.time.sleep'):
            with self.assertRaisesRegex(RuntimeError,'1/3 queued'):
                control.execute(w,{'op':'type','text':'abc'})
        self.assertEqual(len(w.commands),2)

if __name__=='__main__':unittest.main()
