#!/usr/bin/env python3
"""Stage the offline collection from the core's name/image list; no new format."""
from pathlib import Path
import re, shutil, hashlib, json
import release
root=Path(__file__).resolve().parents[1]
sealed=release.verified(root)
out=root/'work/distribution/kit'
if out.exists():
 shutil.rmtree(out)  # generated kit only; never an installed Macintosh tree
out.mkdir(parents=True,exist_ok=True)
utilities=out/'Utilities';utilities.mkdir(exist_ok=True)
entries=re.findall(r'CLARICE_COMMAND\("([^"]+)", "([^"]+)"\)',(root/'src/standard-commands.inc').read_text())
assert len(entries)==len({n for n,_ in entries})
expected={n for n,_ in entries}
assert not ({p.name for p in utilities.iterdir()}-expected),'unexpected files in generated kit; inspect before packaging'
manifest=[]
for name,image in entries:
 data=(root/f'work/build/{image}.clx').read_bytes();assert data[:4]==b'CLX1'
 (utilities/name).write_bytes(data)
 manifest.append(dict(name=name,image=image,bytes=len(data),sha256=hashlib.sha256(data).hexdigest()))
shutil.copyfile(root/'work/build/Clarice.bin',out/'Clarice.bin')
(out/'Read Me.txt').write_bytes((root/'INSTALL.md').read_text().replace('→','>').replace('\n','\r').encode('mac_roman',errors='replace'))
shutil.copyfile(root/'THIRD_PARTY.md',out/'Third Party.txt')
shutil.copytree(root/'licenses',out/'Licenses',dirs_exist_ok=True)
for name,source in {
 'GPL-2.0.txt':'LICENSE',
 'sbase-LICENSE.txt':'src/vendor/sbase/LICENSE',
 'musl-COPYRIGHT.txt':'src/vendor/musl-regex/COPYRIGHT',
 'awk-LICENSE.txt':'src/vendor/awk/LICENSE',
 'mrsh-LICENSE.txt':'src/vendor/mrsh/LICENSE',
 'ekermit-COPYING.txt':'src/ports/ekermit/upstream/COPYING',
 'libtelnet-COPYING.txt':'src/commands/vendor/libtelnet/COPYING',
 'libtelnet-AUTHORS.txt':'src/commands/vendor/libtelnet/AUTHORS',
 'pico-LICENSE.txt':'src/commands/vendor/picohttpparser/LICENSE',
 'mactcp-LICENSE.txt':'src/commands/vendor/mactcp/LICENSE',
}.items(): shutil.copyfile(root/source,out/'Licenses'/name)
(out/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
print(f'Staged core and {len(entries)} external commands in {out}')

print(release.bundle(root,out))
