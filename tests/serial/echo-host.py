#!/usr/bin/env python3
"""PTY transport integration for the real macOS pyserial echo program."""
import os, pty, select, subprocess, sys, time, tty
from pathlib import Path
root=Path(__file__).resolve().parents[2]
def start():
    master, slave=pty.openpty();tty.setraw(slave)
    p=subprocess.Popen([sys.executable,str(root/'scripts/serial-echo.py'),os.ttyname(slave)],stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
    assert 'Echoing' in p.stdout.readline()
    return master,slave,p
for repeat in range(2):
    master,slave,p=start()
    payload=bytes(range(256))*32
    # Pacing bounds PTY capacity; every byte is checked (including XON/XOFF).
    for i in range(0,len(payload),256):
        chunk=payload[i:i+256];assert os.write(master,chunk)==len(chunk)
        got=bytearray();deadline=time.monotonic()+3
        while len(got)<len(chunk):
            assert time.monotonic()<deadline
            if select.select([master],[],[],.1)[0]:got.extend(os.read(master,256))
        assert bytes(got)==chunk
    if repeat==0:
        os.close(master);os.close(slave)
        assert p.wait(timeout=3)==1
        assert 'reconnect' in p.stderr.read()
    else:
        import signal
        p.send_signal(signal.SIGINT);assert p.wait(timeout=3)==0
        os.close(master);os.close(slave)
print('PASS: two connections, 8192 exact bytes each, disconnect error, reconnect and Ctrl+C shutdown')
