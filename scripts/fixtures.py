#!/usr/bin/env python3
"""Create disposable guest fixtures; run only while project emulator is stopped."""
from pathlib import Path
import subprocess, struct, binascii, sys
ROOT=Path(__file__).resolve().parents[1]
W=ROOT/'work/emulator'
if (W/'pid').exists():
    proc=subprocess.run(['ps','-p',(W/'pid').read_text().strip(),'-o','command='],capture_output=True,text=True)
    if str(W/'Sys6Snow.app/Contents/MacOS/Snow') in proc.stdout:
        raise SystemExit('Stop the project emulator before changing fixtures')
p=W/'fixture.bin'
name=b'fork-test';data=b'original data\r';resource=b'SYS6 RESOURCE FORK MUST SURVIVE\x00\xff'
h=bytearray(128);h[1]=len(name);h[2:2+len(name)]=name;h[65:69]=b'TEXT';h[69:73]=b'TEST';h[83:91]=struct.pack('>II',len(data),len(resource))
h[122]=h[123]=129;h[124:126]=struct.pack('>H',binascii.crc_hqx(h[:124],0))
p.write_bytes(h+data+b'\0'*((-len(data))%128)+resource+b'\0'*((-len(resource))%128))
(W/'small.txt').write_bytes(b'alpha beta\rsecond line\r\nthird line\n')
commands=['hmount /src/work/emulator/system.hda','hls :Bench >/dev/null 2>&1 || hmkdir :Bench']
for count in (() if '--small-only' in sys.argv else (10,100,1000)):
    commands += [f'hls :Bench:n{count} >/dev/null 2>&1 || hmkdir :Bench:n{count}']
    commands += [f'hcopy -r /src/work/emulator/small.txt :Bench:n{count}:file{i:04d}.txt' for i in range(count)]
commands += ['hcopy -m /src/work/emulator/fixture.bin :Documents:fork-test','hcopy -r /src/work/emulator/small.txt ":Documents:with space.txt"','humount']
(W/'fixtures.sh').write_text('set -e\n'+'\n'.join(commands)+'\n')
image='ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c'
subprocess.run(['docker','run','--rm','--pull','never','--platform','linux/amd64','--network','none','-v',str(ROOT)+':/src',image,'sh','/src/work/emulator/fixtures.sh'],check=True)
