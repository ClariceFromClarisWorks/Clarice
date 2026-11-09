from pathlib import Path
import struct,binascii,random
root=Path(__file__).resolve().parents[2];out=root/'work/userland-fixtures';out.mkdir(exist_ok=True,parents=True)
name=b'fork-source';data=b'original data\n';resource=b'CLARICE RESOURCE FORK MUST SURVIVE\x00\xff'
h=bytearray(128);h[1]=len(name);h[2:2+len(name)]=name;h[65:69]=b'TEXT';h[69:73]=b'TEST';h[83:91]=struct.pack('>II',len(data),len(resource));h[122]=h[123]=129;h[124:126]=struct.pack('>H',binascii.crc_hqx(h[:124],0))
(out/'fork-source.bin').write_bytes(h+data+b'\0'*((-len(data))%128)+resource+b'\0'*((-len(resource))%128))
# 20 KB > 16 KiB chunk threshold: five 128-line runs, then bounded merges.
(out/'ul-sort').write_bytes(b''.join(('%04d sort fixture line abcdefghijklmnop\n'%n).encode() for n in random.Random(7).sample(range(520),520)))
(out/'ul-numbers').write_bytes(b'20\n3\n-1\n10\n')
(out/'ul-bigcopy').write_bytes(bytes(range(256))*8192)
(out/'ul-follow').write_bytes(b'initial\n')
