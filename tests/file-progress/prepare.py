from pathlib import Path
import re,subprocess,shutil,json,struct,binascii
root=Path.cwd();work=root/'work/file-progress';out=Path('/tmp/clarice-file-progress');out.mkdir(exist_ok=True)
prior=Path('/tmp/clarice-ls-output')
for name in ['system.hda','bench.hda']:shutil.copyfile(prior/name,work/name)
for name in ['plus.pram','plus.rom']:shutil.copyfile(prior/name,out/name)
# MacBinary II fixture: deterministic fork contents, native metadata retained.
for name,size in [('fp-source',262144),('fp-large',1048576)]:
 h=bytearray(128);h[1]=len(name);h[2:2+len(name)]=name.encode();h[65:73]=b'TEXTFPts';struct.pack_into('>II',h,83,size,size//4);struct.pack_into('>II',h,91,3800000000,3800000001)
 h[122]=h[123]=129;h[124:126]=struct.pack('>H',binascii.crc_hqx(h[:124],0))
 data=bytes(range(256))*(size//256);resource=bytes(reversed(range(256)))*(size//1024)
 (work/(name+'.bin')).write_bytes(h+data+resource)
(work/'profile').write_text('export PATH=/bin:/usr/bin\nsh /tmp/fp-tests\n')
im=re.search(r'ghcr.io/[^ ]+',Path('tests/file-progress/build.sh').read_text())[0]
prefix=':System Folder:Clarice Data:'
cmd=['hmount /src/work/file-progress/system.hda']
for v in ['before','after']:
 cmd += [f'hmkdir "{prefix}tmp:fp-{v}"']
 for name in ['cp','mv','rm','hfsattr']:
  cmd += [f'hcopy -r /src/work/file-progress/{v}/files "{prefix}tmp:fp-{v}:{name}"']
for name in ['fp-source','fp-large']:cmd += [f'hcopy -m /src/work/file-progress/{name}.bin "{prefix}tmp:{name}"']
(work/'small.txt').write_text('small file\n')
cmd += [f'hmkdir "{prefix}tmp:fp-small"']
for i in range(10):cmd += [f'hcopy -r /src/work/file-progress/small.txt "{prefix}tmp:fp-small:file{i:02d}"']
cmd += [f'hcopy -r /src/tests/file-progress/guest.sh "{prefix}tmp:fp-tests"',f'hcopy -r /src/work/file-progress/profile "{prefix}etc:profile"','humount']
subprocess.run(['docker','run','--rm','--pull','never','--platform','linux/amd64','--network','none','-v',str(root)+':/src',im,'sh','-ec','\n'.join(cmd)],check=True)
for name in ['system.hda','bench.hda']:shutil.copyfile(work/name,out/name)
c=json.loads((prior/'test.snoww').read_text());c['rom_path']=str(out/'plus.rom');c['pram_path']=str(out/'plus.pram');c['scsi_targets'][0]={'Disk':str(out/'system.hda')};c['scsi_targets'][1]={'Disk':str(out/'bench.hda')};(out/'test.snoww').write_text(json.dumps(c))
