"""Publish bounded test-only repositories; never a production feed."""
from pathlib import Path
import zlib
r=Path(__file__).resolve().parents[2];d=r/'work/self-update-snow/http'
def row(kind,name,version,path):
 b=path.read_bytes();return f'{kind}\t{name}\t{version}\t{len(b)}\t{zlib.crc32(b):08x}\t{path.name}\n'
core=row('core','Clarice',2026111901,d/'Core1901.bin')
for n in [1,2]:
 (d/f'tools{n}.txt').write_text('CLARICE-REPOSITORY 1\n'+core+row('tool','selfprobe',n,d/f'probe{n}.clx')+'tool\tclarice\t2026111901\t36\t00000000\tabsent-old-updater.clx\n')
(d/'bad.txt').write_text('CLARICE-REPOSITORY 1\n'+core+'tool\tselfprobe\t3\t36\t00000000\tcorrupt.clx\n')
(d/'corrupt.clx').write_bytes(b'x'*36)
