#!/usr/bin/env python3
"""Bounded byte-for-byte serial echo peer; no shell, network listener or line editing."""
import argparse
import json
import sys
import time
from pathlib import Path


def echo(port, log, seconds=0):
    deadline = time.monotonic() + seconds if seconds else float('inf')
    while time.monotonic() < deadline:
        data = port.read(256)
        if not data:
            continue
        if log:
            log.write(json.dumps({'time': time.monotonic(), 'rx': data.hex()}) + '\n')
            log.flush()
        pending = memoryview(data)
        while pending:
            n = port.write(pending)
            if not n:
                raise OSError('serial write made no progress')
            pending = pending[n:]
        if log:
            log.write(json.dumps({'time': time.monotonic(), 'tx': data.hex()}) + '\n')
            log.flush()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('device', help='/dev/cu.usbserial-… or Snow PTY path')
    parser.add_argument('-b', '--baud', type=int, default=57600)
    parser.add_argument('--log', type=Path)
    parser.add_argument('--seconds', type=float, default=0, help='0: run until Ctrl+C')
    args = parser.parse_args()
    try:
        import serial
    except ImportError:
        parser.error('install pyserial in a virtual environment: python3 -m pip install pyserial')
    try:
        # No flow control matches Clarice and a basic TX/RX/ground cable.
        with serial.Serial(args.device, args.baud, bytesize=8, parity='N', stopbits=1,
                           timeout=0.05, write_timeout=2, xonxoff=False,
                           rtscts=False, dsrdtr=False) as port:
            print(f'Echoing {args.device}: {args.baud} 8N1, no flow control', flush=True)
            if args.log:
                with args.log.open('a') as log:
                    echo(port, log, args.seconds)
            else:
                echo(port, None, args.seconds)
    except KeyboardInterrupt:
        return 0
    except (serial.SerialException, OSError) as error:
        print(f'serial-echo: {error}; reconnect the adapter and restart this command', file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
