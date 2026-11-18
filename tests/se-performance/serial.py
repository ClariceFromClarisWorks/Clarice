"""Physical-SE serial acceptance: explicit USB adapter, 57600 8N1, no flow control."""
import fcntl,struct
import os, tty, termios, select, time, json, re, argparse
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument('--pty', required=True)
p.add_argument('--output', required=True)
a = p.parse_args()
assert re.fullmatch(r'/dev/cu\.usbmodem[0-9]+', a.pty), 'USB modem adapter required'
password = os.environ['CLARICE_TEST_PASSWORD'].encode()
fd = os.open(a.pty, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
original = termios.tcgetattr(fd)
tty.setraw(fd)
attrs = termios.tcgetattr(fd); attrs[4] = attrs[5] = termios.B57600
attrs[2] |= termios.CLOCAL | termios.CREAD
termios.tcsetattr(fd, termios.TCSANOW, attrs)
fcntl.ioctl(fd,termios.TIOCMBIS,struct.pack("I",termios.TIOCM_DTR|termios.TIOCM_RTS))
rows = []
def send(data):
    for byte in data:
        assert os.write(fd, bytes([byte])) == 1
        time.sleep(.002)
def until(needle, timeout=60):
    data = b''; end = time.monotonic() + timeout
    while time.monotonic() < end:
        if select.select([fd], [], [], .1)[0]: data += os.read(fd, 4096)
        if needle in data: return data
    raise AssertionError(repr(data))
def login():
    send(b'\r'); until(b'Password: ')
    send(password + b'\r'); text = until(b' > ', 120)
    assert password not in text
def command(s):
    send(s.encode() + b'\r'); return until(b' > ')
try:
    login()
    assert b'/\r\n' in command('pwd')
    assert b'remote-value\r\n' in command('V=remote-value; echo "$V"')
    send(b'while true; do :; done\r'); time.sleep(.3); send(b'\x03')
    until(b' > ')
    assert b'130\r\n' in command('echo $?')
    rows.append({'test': 'serial login, variables, Ctrl+C and status recovery', 'pass': True})
    heaps = []
    for i in range(3):
        assert b'1\r\n' in command('test -e /tmp/se1818-incomplete.bin; echo $?')
        send(b'kermit receive /tmp/se1818-incomplete.bin\r')
        until(b'Kermit ready;')
        # Incomplete packet must not become a shell command after cancellation.
        send(b'\x01! Dpartial'); time.sleep(.1); send(b'\x03')
        time.sleep(.8); login()
        assert b'1\r\n' in command('test -e /tmp/se1818-incomplete.bin; echo $?')
        listing = command('ls /tmp')
        assert b'Clv kermit' not in listing
        text = command('status runtime').decode('latin1')
        heaps.append([int(x) for x in re.search(r'heap=(\d+) largest=(\d+)', text).groups()])
        assert 'remaining=0' in text
    assert max(h[0] for h in heaps) - min(h[0] for h in heaps) <= 64, heaps
    rows.append({'test': 'three interrupted receives, no target/temp files, reconnect and cleanup', 'pass': True, 'heap_largest': heaps})
    send(b'exit\r'); time.sleep(.5); login()
    assert b'AFTER_LOGOUT\r\n' in command('echo AFTER_LOGOUT')
    send(b'exit\r'); time.sleep(.5)
    rows.append({'test': 'logout and reconnect', 'pass': True})
finally:
    termios.tcsetattr(fd, termios.TCSANOW, original)
    os.close(fd)
Path(a.output).write_text(json.dumps(rows, indent=2) + '\n')
print('PASS', len(rows), 'physical serial acceptance groups')
