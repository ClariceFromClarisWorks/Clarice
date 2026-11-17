import socket
import tempfile
import threading
import unittest
from pathlib import Path
import struct
import zlib
import host


class Protocol(unittest.TestCase):
    def test_local_only(self):
        for address in ('0.0.0.0', '8.8.8.8', '169.254.1.1', '224.0.0.1'):
            with self.assertRaises(ValueError):
                host.local_address(address)
        self.assertEqual(host.local_address('192.168.1.252'), '192.168.1.252')

    def test_png_pixels(self):
        frame = bytes(range(256)) * 85 + bytes(range(128))
        self.assertEqual(len(frame), 21888)
        data = host.png(frame)
        pos, packed = 8, b''
        while pos < len(data):
            size, = struct.unpack('>I', data[pos:pos+4])
            kind = data[pos+4:pos+8]; chunk = data[pos+8:pos+8+size]
            crc, = struct.unpack('>I', data[pos+8+size:pos+12+size])
            self.assertEqual(crc, zlib.crc32(kind+chunk))
            if kind == b'IDAT': packed += chunk
            pos += size + 12
        raw = zlib.decompress(packed)
        self.assertEqual(b''.join(bytes(x^255 for x in raw[y*65+1:y*65+65]) for y in range(342)), frame)

    def test_capture_fragmentation_and_disconnect(self):
        for truncate in (False, True):
            a,b = socket.socketpair()
            def peer():
                with b:
                    self.assertEqual(b.recv(64), b'GRAB\n')
                    b.sendall(b'FRAME 512 342 64 21888 1 60000\n')
                    frame = b'\xaa'* (100 if truncate else 21888)
                    for i in range(0,len(frame),17): b.sendall(frame[i:i+17])
            t = threading.Thread(target=peer);t.start()
            with a, tempfile.TemporaryDirectory() as d:
                path = Path(d)/'screen.pbm'
                if truncate:
                    with self.assertRaises(ConnectionError):host.capture(host.Wire(a),path)
                    self.assertFalse(path.exists())
                else:
                    result=host.capture(host.Wire(a),path)
                    self.assertEqual(result['bytes'],21888)
                    self.assertEqual(path.read_bytes(),b'P4\n512 342\n'+b'\xaa'*21888)
            t.join()

    def test_deadline(self):
        a,b=socket.socketpair()
        with a,b:
            with self.assertRaises(TimeoutError):host.Wire(a,.02).line()

    def test_bad_header(self):
        a,b=socket.socketpair()
        with a,b,tempfile.TemporaryDirectory() as d:
            b.sendall(b'FRAME 99999999 342 64 21888 0 0\n')
            with self.assertRaises(ValueError):host.capture(host.Wire(a),Path(d)/'bad.png')


if __name__=='__main__':unittest.main()
