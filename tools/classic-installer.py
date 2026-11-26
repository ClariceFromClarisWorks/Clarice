#!/usr/bin/env python3
"""Wrap a verified release-kit ZIP for extraction entirely on System 6.

Uses the established upstream sit packager; never rebuilds product binaries.
The caller must independently extract/verify before internal publication.
"""
import argparse
import binascii
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import zipfile

def sha(data):
    return hashlib.sha256(data).hexdigest()

def stage(source, output, sit, final=False):
    with zipfile.ZipFile(source) as archive:
        names = archive.namelist()
        if not names or '/' not in names[0]:
            raise ValueError('missing release ZIP root')
        prefix = names[0].split('/', 1)[0] + '/'
        if len(names) != len(set(names)) or any(not n.startswith(prefix) for n in names):
            raise ValueError('unexpected ZIP inventory')
        files = {n[len(prefix):]: archive.read(n) for n in names}
    manifest = json.loads(files.pop('Package.json'))
    if prefix != f"Clarice {manifest.get('release_version', '0.5.0')}/":
        raise ValueError('release ZIP root does not match the manifest')
    if set(files) != set(manifest['files']) or any(sha(v) != manifest['files'][k] for k, v in files.items()):
        raise ValueError('release ZIP checksum/inventory mismatch')
    build = manifest['build']
    output.mkdir(parents=True, exist_ok=False)
    folder = output / f'Clarice {str(build)[-4:]} Installer'
    folder.mkdir()
    native = {}
    for name in ('Clarice', 'Install Clarice', 'Recover Clarice'):
        data = files.pop(name + '.bin')
        if len(data) < 128 or binascii.crc_hqx(data[:124], 0) != int.from_bytes(data[124:126], 'big'):
            raise ValueError('invalid MacBinary header')
        dl, rl = struct.unpack_from('>II', data, 83)
        offset = 128 + (dl + 127) // 128 * 128
        identity = b'APPLCLUP' if name == 'Recover Clarice' else b'APPLCLRC'
        if not rl or data[65:73] != identity or len(data) != offset + (rl + 127) // 128 * 128:
            raise ValueError('invalid application forks or Finder identity')
        path = folder / name
        path.write_bytes(data[128:128+dl])
        Path(str(path) + '/..namedfork/rsrc').write_bytes(data[offset:offset+rl])
        finder = data[65:73] + bytes([data[73], data[101]]) + bytes(22)
        subprocess.run(['xattr', '-wx', 'com.apple.FinderInfo', finder.hex(), str(path)], check=True)
        native[name] = dict(macbinary_sha256=sha(data), data_sha256=sha(data[128:128+dl]),
                            resource_sha256=sha(data[offset:offset+rl]), finder=identity.decode())
    version = manifest.get('release_version', '0.5.0')
    title = (f'Clarice {version} installer - build {build}' if final
             else f'Clarice {build} - internal System 6 testing kit')
    distribution = (f'Matching source: {manifest["source_archive"]}\r'
                    if final else 'Local review draft. Public redistribution approval is pending.\r')
    files['Start Here.txt'] = (
        f'{title}\r\r'
        'Expand the .sit.bin with StuffIt Expander on the Macintosh.\r'
        'Keep this complete folder together. The applications are already decoded.\r'
        'Quit the installed Clarice, then open Install Clarice in this folder.\r'
        'Use Install/Upgrade; retain existing Data locations and recovery copies.\r'
        'Do not run Uninstall or delete Clarice Data. Repair requires a matching-version kit.\r'
        'If interrupted, retain the kit and retry. Preserve the previous working installation.\r'
        'No HDA, disk-image mounting, modern computer extraction or SD-card removal is needed.\r'
        f'{distribution}'
        'See Third Party.txt for redistribution restrictions.\r'
    ).encode('mac_roman')
    for name, data in files.items():
        parts = Path(name).parts
        if not parts or Path(name).is_absolute() or any(p in ('.', '..') or ':' in p or len(p.encode('mac_roman')) > 31 for p in parts):
            raise ValueError('unsafe or non-HFS filename: ' + name)
        path = folder / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data)
    manifest.update(source_zip_sha256=sha(source.read_bytes()), native_applications=native,
                    files={k: sha(v) for k, v in files.items()})
    (folder / 'Package.json').write_text(json.dumps(manifest, indent=2) + '\n')
    target = output / f'Clarice-{str(build)[-4:]}-Installer.sit'
    subprocess.run([str(sit.resolve()), '-T', 'TEXT', '-C', 'ttxt', '-o', str(target.resolve()), folder.name],
                   cwd=output, check=True)
    payload = target.read_bytes()
    header = bytearray(128)
    name = target.name.encode('mac_roman')
    header[1] = len(name)
    header[2:2+len(name)] = name
    header[65:73] = b'SIT!SITx'
    struct.pack_into('>I', header, 83, len(payload))
    header[102:106] = b'mBIN'
    header[122:124] = bytes([130, 129])
    header[124:126] = binascii.crc_hqx(header[:124], 0).to_bytes(2, 'big')
    wrapped = target.with_suffix('.sit.bin')
    wrapped.write_bytes(header + payload + bytes((-len(payload)) % 128))
    (output / 'SHA256SUMS.txt').write_text(''.join(f'{sha(p.read_bytes())}  {p.name}\n' for p in (target, wrapped)))
    print(wrapped)

if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('source', type=Path)
    p.add_argument('output', type=Path)
    p.add_argument('--sit', type=Path, default=Path('work/sit-packager/sit'))
    p.add_argument('--final', action='store_true', help='Use final release-kit wording')
    a = p.parse_args()
    stage(a.source, a.output, a.sit, a.final)
