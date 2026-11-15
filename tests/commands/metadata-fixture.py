#!/usr/bin/env python3
"""Generate ONLY the disposable 56-byte CLX1 corruption-test fixture."""
import struct
import sys
from pathlib import Path
image = bytes.fromhex('70004e750000000000000000')  # return 0; unused data
metadata = struct.pack('>III', 4, 1, 0)
hash_value = 2166136261
for byte in image + metadata:
    hash_value = ((hash_value ^ byte) * 16777619) & 0xffffffff
header = b'CLX1' + struct.pack('>7I', 1, len(image), 0, 1, 0, hash_value, 0)
with Path(sys.argv[1]).open('xb') as output:
    output.write(header + image + metadata)
