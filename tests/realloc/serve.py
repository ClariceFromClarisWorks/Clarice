from pathlib import Path
import hashlib,zlib,subprocess
r=Path(__file__).resolve().parents[2];d=r/'work/additions-snow/http';lines=(d/'index.txt').read_text().splitlines();names=['difftrace','diffgrow','alloctest']
lines=[l for l in lines if not any(l.startswith('tool\t'+n+'\t') for n in names)]
for name in names:
 p=r/'work/realloc'/f'{name}.clx'
 if not p.exists():continue
 b=p.read_bytes();fn=name+'-'+hashlib.sha256(b).hexdigest()+'.clx';(d/fn).write_bytes(b)
 version=2026111896 if name in ('alloctest','diffgrow') else 2026111894
 lines.append(f'tool\t{name}\t{version}\t{len(b)}\t{zlib.crc32(b):08x}\t{fn}')
(d/'index.txt').write_text('\n'.join(lines)+'\n')
print('Updated private index')
