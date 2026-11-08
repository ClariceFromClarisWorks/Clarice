#!/usr/bin/env python3
"""Set the classic Finder hasBundle flag in the generated MacBinary header."""
from pathlib import Path
import binascii
p=Path(__file__).resolve().parents[1]/'work/build/Clarice.bin'
b=bytearray(p.read_bytes())
assert b[65:73] == b'APPLCLRC'
flags=((b[73]<<8)|b[101]|0x2000)&~0x0100
b[73]=flags>>8;b[101]=flags&255
b[124:126]=binascii.crc_hqx(b[:124],0).to_bytes(2,'big')
p.write_bytes(b)
