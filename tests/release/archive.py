"""Exercise production restricted-reader + pristine microtar under sanitizers."""
from pathlib import Path
import binascii, io, re, struct, subprocess, tarfile, tempfile
r=Path(__file__).resolve().parents[2]
names=re.findall(r'CLARICE_COMMAND\("([^"]+)",', (r/'src/standard-commands.inc').read_text())
with tempfile.TemporaryDirectory() as d:
 d=Path(d);exe=d/'reader'
 subprocess.run(['cc','-std=c99','-Wno-unused-function','-Wno-deprecated-declarations','-fsanitize=address,undefined',str(r/'tests/release/tar.c'),str(r/'src/update/vendor/microtar/microtar.c'),'-o',str(exe)],check=True)
 core=b'C'*128;tool=b'T'*36
 manifest=bytearray(struct.pack('>8sII32sII',b'CLREL001',42,len(names),b'I'*32,len(core),binascii.crc32(core)))
 for n in names:manifest.extend(struct.pack('>24sII',n.encode(),len(tool),binascii.crc32(tool)))
 manifest.extend(struct.pack('>I',binascii.crc32(manifest)))
 out=io.BytesIO()
 with tarfile.open(fileobj=out,mode='w',format=tarfile.USTAR_FORMAT) as t:
  for name,data in [('Release',manifest),('Clarice.bin',core)]+[('Utilities/'+n,tool) for n in names]:
   h=tarfile.TarInfo(name);h.size=len(data);t.addfile(h,io.BytesIO(data))
 good=out.getvalue()
 def run(data,ok):
  p=d/'fixture.tar';p.write_bytes(data);v=subprocess.run([str(exe),str(p)],capture_output=True,text=True)
  assert (v.returncode==0)==ok,(v.returncode,v.stdout,v.stderr)
  assert 'Sanitizer' not in v.stderr and 'runtime error:' not in v.stderr,v.stderr
 run(good,True)
 for size in [0,127,511,512,1023,len(good)-1]:run(good[:size],False)
 for offset in [0,100,108,124,136,148,156,157,257,345,512,2048,2560]:
  bad=bytearray(good);bad[offset]^=255;run(bad,False)
 # Checksummed malicious header must be rejected before upstream strcpy/sscanf.
 for field in [(0,100,b'A'),(124,12,b'7'),(156,1,b'2'),(345,1,b'x')]:
  bad=bytearray(good);start,n,value=field;bad[start:start+n]=value*n;bad[148:156]=b' '*8
  bad[148:156]=('%06o\0 '%sum(bad[:512])).encode();run(bad,False)
 bad=bytearray(good);bad[-512]=1;run(bad,False)
 print('PASS: complete single bundle; 24 truncation/corruption/unsafe-header/extra-member failures; ASan/UBSan clean')
