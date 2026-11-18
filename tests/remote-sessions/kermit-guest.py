"""Independent C-Kermit peer: exact binary/MacBinary container round trip in Snow.

Requires upstream C-Kermit 9.0.302 (BSD), configured with FORCE-3 to match the
existing E-Kermit adapter. Credentials and packet logs stay in ignored work.
"""
import argparse, os, subprocess, tempfile, hashlib, json, time, binascii
from pathlib import Path
p = argparse.ArgumentParser()
p.add_argument('--peer', required=True)
p.add_argument('--transport', choices=['telnet', 'serial'], required=True)
p.add_argument('--pty', default='/dev/ttys000')
p.add_argument('--output', required=True)
a = p.parse_args()
password = os.environ['CLARICE_TEST_PASSWORD']
assert password.isascii() and all(c.isalnum() or c in '-_.' for c in password), 'use a disposable simple test password'
if a.transport == 'serial':
    import re
    assert re.fullmatch(r'/dev/ttys\d+', a.pty)
connect = ('set network type tcp/ip\nset telnet binary-mode required\n'
           'set host 127.0.0.1 23231 /telnet') if a.transport == 'telnet' else (
           'set modem type none\nset carrier-watch off\nset flow none\n'
           f'set line {a.pty}\nset speed 57600\noutput \\13')
header = bytearray(128); header[1] = 7; header[2:9] = b'Fixture'
header[65:73] = b'TEXTttxt'; header[73] = 4
header[83:87] = (4096).to_bytes(4, 'big'); header[87:91] = (128).to_bytes(4, 'big')
header[91:95] = (3500000000).to_bytes(4, 'big')
header[95:99] = (3500000100).to_bytes(4, 'big')
header[122:124] = bytes([129, 129])
header[124:126] = binascii.crc_hqx(header[:124], 0).to_bytes(2, 'big')
source = bytes(header) + bytes(range(256)) * 16 + bytes(range(128))
with tempfile.TemporaryDirectory(prefix='clarice-kermit-') as d:
    d = Path(d); (d/'source.bin').write_bytes(source)
    # C-Kermit 9.0.302's old macOS NOUUCP zchko path crashes on a nonexistent
    # output file on modern macOS. An existing disposable destination avoids it.
    (d/'returned.bin').touch()
    guest = f'/tmp/{a.transport}-roundtrip.bin'
    script = f'''set exit warning off
set quiet on
set input echo off
set input timeout-action proceed
{connect}
if fail exit 1 connect
input 30 Password:
if fail exit 4 login-prompt
output {password}\\13
input 60 {{ > }}
if fail exit 5 authentication
output rm -f {guest}\\13
input 20 {{ > }}
if fail exit 6 prepare
output kermit receive {guest}\\13
input 20 {{Kermit ready;}}
if fail exit 7 receive-ready
set file type binary
set block-check force-3
set window 1
send source.bin
if fail exit 8 send
output \\13
input 20 {{ > }}
if fail exit 9 send-prompt
output kermit send {guest}\\13
input 20 {{Kermit ready;}}
if fail exit 10 send-ready
set file collision overwrite
receive returned.bin
if fail exit 11 receive
output \\13
input 20 {{ > }}
if fail exit 12 receive-prompt
output exit\\13
close
exit 0
'''
    path=d/'test.ksc'; path.write_text(script); path.chmod(0o600)
    start=time.monotonic()
    result=subprocess.run([str(Path(a.peer).resolve()), '-y', '/dev/null', '-C', 'take test.ksc'], cwd=d, capture_output=True, timeout=180)
    assert result.returncode == 0, (result.returncode, result.stdout.decode(errors='replace').replace(password, '[redacted]'))
    assert (d/'returned.bin').read_bytes() == source
    evidence=dict(transport=a.transport, peer='C-Kermit 9.0.302', bytes=len(source),
                  sha256=hashlib.sha256(source).hexdigest(), exact_roundtrip=True,
                  host_seconds=time.monotonic()-start,
                  scope='Snow; MacBinary preserved as a container, not extracted')
Path(a.output).write_text(json.dumps(evidence,indent=2)+'\n')
print('PASS',a.transport,'C-Kermit exact binary round trip')
