#!/usr/bin/env python3
"""One-session SE control proof. Python standard library only; no HTTP service.
The SE connects outward; the listener requires explicit local bind and peer IP.
Outputs JSON timings and PNG/PBM. QUEUED means posted, not delivered to an app.
"""
import argparse
import hashlib
import hmac
import ipaddress
import json
from pathlib import Path
import socket
import struct
import time
import zlib


def local_address(value):
    a = ipaddress.IPv4Address(value)
    networks = ('10.0.0.0/8', '172.16.0.0/12', '192.168.0.0/16', '127.0.0.0/8')
    if not any(a in ipaddress.IPv4Network(n) for n in networks):
        raise ValueError('Use an explicit RFC1918 or loopback address')
    return str(a)


class Wire:
    def __init__(self, sock, timeout=15):
        self.sock, self.timeout = sock, timeout
        self.deadline = time.monotonic() + timeout

    def reset(self):
        self.deadline = time.monotonic() + self.timeout

    def exact(self, n):
        result = bytearray()
        while len(result) < n:
            remaining = self.deadline - time.monotonic()
            if remaining <= 0:
                raise TimeoutError('SE response deadline exceeded')
            self.sock.settimeout(remaining)
            try:
                part = self.sock.recv(n - len(result))
            except socket.timeout as e:
                raise TimeoutError('SE response deadline exceeded') from e
            if not part:
                raise ConnectionError('SE disconnected before completing response')
            result.extend(part)
        return bytes(result)

    def line(self):
        result = bytearray()
        while len(result) < 160:
            c = self.exact(1)
            if c == b'\n':
                return result.decode('ascii')
            result.extend(c)
        raise ValueError('Oversized response header')

    def send(self, command):
        self.reset()
        self.sock.settimeout(self.timeout)
        self.sock.sendall(command.encode('ascii') + b'\n')


def png(frame, width=512, height=342, stride=64):
    def chunk(kind, data):
        return struct.pack('>I', len(data)) + kind + data + struct.pack('>I', zlib.crc32(kind + data))
    rows = b''.join(b'\0' + bytes(x ^ 255 for x in frame[y*stride:y*stride+(width+7)//8]) for y in range(height))
    return (b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', width, height, 1, 0, 0, 0, 0))
            + chunk(b'IDAT', zlib.compress(rows)) + chunk(b'IEND', b''))


def capture(wire, dest):
    start = time.monotonic()
    wire.send('GRAB')
    fields = wire.line().split()
    if len(fields) != 7 or fields[0] != 'FRAME':
        raise ValueError('Invalid screen header: ' + ' '.join(fields))
    width,height,stride,size,ticks,heap = map(int,fields[1:])
    if not (1<=width<=1024 and 1<=height<=768 and stride==((width+15)//16)*2 and size==stride*height):
        raise ValueError('Invalid screen dimensions')
    frame = wire.exact(size)
    wire.screen_size=(width,height)
    # Do not leave a plausible but truncated screenshot on failure.
    data = png(frame,width,height,stride) if dest.suffix.lower() == '.png' else ('P4\n%d %d\n'%(width,height)).encode()+b''.join(frame[y*stride:y*stride+(width+7)//8] for y in range(height))
    tmp = dest.with_name(dest.name + '.part')
    tmp.write_bytes(data)
    tmp.replace(dest)
    return dict(action='capture', file=str(dest), seconds=time.monotonic()-start,
                guest_capture_ticks=ticks, guest_free_heap_with_snapshot=heap,
                frame_sha256=hashlib.sha256(frame).hexdigest(), bytes=len(frame),width=width,height=height)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--bind', required=True, type=local_address)
    p.add_argument('--peer', required=True, type=local_address)
    p.add_argument('--port', type=int, default=5968)
    p.add_argument('--token-file', type=Path, required=True)
    p.add_argument('--timeout', type=float, default=15)
    p.add_argument('--wait', type=float, default=90, help='seconds to wait for SE launch')
    p.add_argument('--capture', type=Path, required=True)
    p.add_argument('--key', nargs=3, type=int, metavar=('CHAR', 'KEYCODE', 'MODIFIERS'))
    p.add_argument('--after', type=Path, help='capture after key, e.g. after.png')
    args = p.parse_args()
    token = args.token_file.read_text().strip()
    if len(token) != 32 or any(c not in '0123456789abcdef' for c in token):
        p.error('token must be 32 lowercase hex characters')
    if args.timeout <= 0 or args.wait <= 0 or not 1 <= args.port <= 65535:
        p.error('invalid timeout or port')
    if args.key:
        ch, code, mods = args.key
        if not 0 <= ch < 256 or not 0 <= code < 128 or mods < 0 or mods & ~0x1B00:
            p.error('invalid key character, keycode or modifiers')
    with socket.socket() as server:
        server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server.bind((args.bind, args.port)); server.listen(1); server.settimeout(args.wait)
        print(json.dumps(dict(state='listening', bind=args.bind, port=args.port)), flush=True)
        sock, peer = server.accept()
        with sock:
            if peer[0] != args.peer:
                raise PermissionError('Unexpected peer; session closed')
            wire = Wire(sock, args.timeout)
            if not hmac.compare_digest(wire.line(), 'HIL1 ' + token):
                raise PermissionError('SE pairing token mismatch')
            print(json.dumps(capture(wire, args.capture)), flush=True)
            if args.key:
                wire.send('KEY %d %d %d' % tuple(args.key))
                reply = wire.line()
                if reply != 'QUEUED':
                    raise RuntimeError(reply)
                print(json.dumps(dict(action='key', result='queued, delivery unconfirmed')), flush=True)
                time.sleep(0.5)
            if args.after:
                print(json.dumps(capture(wire, args.after)), flush=True)
            wire.send('QUIT')
            if wire.line() != 'BYE':
                raise RuntimeError('Missing shutdown acknowledgement')
            print(json.dumps(dict(state='completed')), flush=True)


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, RuntimeError) as e:
        print(json.dumps(dict(state='failed', error=str(e))), flush=True)
        raise SystemExit(1)
