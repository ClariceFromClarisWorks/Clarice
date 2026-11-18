"""Private isolated-Snow fixture repository; no release catalog mutation."""
from pathlib import Path
import shutil,json,hashlib,zlib,subprocess,tarfile,io,gzip,sys
r=Path(__file__).resolve().parents[2];w=r/'work/additions-snow';dest=w/'http';dest.mkdir(exist_ok=True)
old=r.parent/'Clarice-Busybox-Replacements/work/replacement-snow/http'
lines=(old/'index.txt').read_text().splitlines()
for line in lines[1:]:
 fields=line.split('\t');shutil.copy2(old/fields[5],dest/fields[5])
for row in json.loads((r/'outputs/busybox-additions/packages.json').read_text()):
 name=row['name'];data=(r/f'work/busybox-additions/bb{name}.clx').read_bytes();filename=f'{name}-{row["sha256"]}.clx';(dest/filename).write_bytes(data)
 version=2026111893 if name=='tar' else 2026111892 if name in ['patch','xxd'] else 2026111891
 lines.append(f'tool\t{name}\t{version}\t{len(data)}\t{zlib.crc32(data):08x}\t{filename}')
data=(r/'work/busybox-additions/forksum.clx').read_bytes();filename='forksum-'+hashlib.sha256(data).hexdigest()+'.clx';(dest/filename).write_bytes(data);lines.append(f'tool\tforksum\t2026111893\t{len(data)}\t{zlib.crc32(data):08x}\t{filename}')
(dest/'index.txt').write_text('\n'.join(lines)+'\n')
blob=bytes(range(256))*32;(dest/'binary8k').write_bytes(blob);(dest/'binary64k').write_bytes(blob*8)
text=(b'alpha beta gamma delta\n'*400)[:8192];(dest/'text8k').write_bytes(text)
(dest/'host.gz').write_bytes(gzip.compress(blob,mtime=0));bad=bytearray(gzip.compress(blob,mtime=0));bad[-8]^=1;(dest/'bad.gz').write_bytes(bad)
b=io.BytesIO()
with tarfile.open(fileobj=b,mode='w',format=tarfile.USTAR_FORMAT) as tf:
 for name,data in [('binary8k',blob),('hello.txt',b'hello tar\n')]:
  info=tarfile.TarInfo(name);info.size=len(data);tf.addfile(info,io.BytesIO(data))
(dest/'host.tar').write_bytes(b.getvalue());(dest/'truncated.tar').write_bytes(b.getvalue()[:1024])
a=''.join((f'{i:04d} alpha beta gamma delta'.ljust(31)+'\n') for i in range(256)).encode();b=a.replace(b'0128 alpha beta',b'0128 alpha BETA')
for name,data in [('lines8k',a),('edited8k',b),('lines256k',a*32),('edited256k',b*32)]: (dest/name).write_bytes(data)
for label,fmt in [('pax',tarfile.PAX_FORMAT),('gnu',tarfile.GNU_FORMAT)]:
 b=io.BytesIO()
 with tarfile.open(fileobj=b,mode='w',format=fmt) as tf:
  inf=tarfile.TarInfo('x'*120);inf.size=1;tf.addfile(inf,io.BytesIO(b'x'))
 (dest/(label+'.tar')).write_bytes(b.getvalue())
if '--no-server' in sys.argv:
 print('Refreshed private repository without restarting server');sys.exit(0)
p=subprocess.Popen(['python3','-m','http.server','8099','--bind','0.0.0.0','--directory',str(dest)],stdout=(w/'http.log').open('w'),stderr=subprocess.STDOUT,start_new_session=True);(w/'http.pid').write_text(str(p.pid));print('Private HTTP PID',p.pid)
