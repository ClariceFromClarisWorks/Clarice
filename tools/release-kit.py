#!/usr/bin/env python3
"""Stage a fork-safe distribution draft from an already accepted build."""
import argparse
import binascii
import gzip
import hashlib
import io
import json
from pathlib import Path
import struct
import subprocess
import sys
import tarfile
import tempfile
import zipfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'scripts'))
import release

BUSYBOX = 'work/busybox/busybox-1.37.0.tar.bz2'
BUSYBOX_SHA = '3311dff32e746499f4df0d5df04d7eb396382d7e108bb9250e7b519b837043a4'
NOTICES = {
    'sbase-LICENSE.txt': 'src/vendor/sbase/LICENSE',
    'musl-regex-COPYRIGHT.txt': 'src/vendor/musl-regex/COPYRIGHT',
    'musl-fnmatch-COPYRIGHT.txt': 'src/vendor/musl-fnmatch/COPYRIGHT',
    'awk-LICENSE.txt': 'src/vendor/awk/LICENSE',
    'mrsh-LICENSE.txt': 'src/vendor/mrsh/LICENSE',
    'newlib-getopt-NOTICE.txt': 'src/vendor/newlib-getopt/getopt.c',
    'ekermit-COPYING.txt': 'src/ports/ekermit/upstream/COPYING',
    'libtelnet-COPYING.txt': 'src/commands/vendor/libtelnet/COPYING',
    'libtelnet-AUTHORS.txt': 'src/commands/vendor/libtelnet/AUTHORS',
    'pico-LICENSE.txt': 'src/commands/vendor/picohttpparser/LICENSE',
    'mactcp-LICENSE.txt': 'src/commands/vendor/mactcp/LICENSE',
    'microtar-LICENSE.txt': 'src/update/vendor/microtar/LICENSE',
}

def git(*args):
    return subprocess.check_output(['git', '-C', str(ROOT), *args])

def sha(data):
    return hashlib.sha256(data).hexdigest()

def forks(data):
    if len(data) < 128 or binascii.crc_hqx(data[:124], 0) != int.from_bytes(data[124:126], 'big'):
        raise ValueError('invalid MacBinary header')
    d, r = struct.unpack_from('>II', data, 83)
    offset = 128 + (d + 127) // 128 * 128
    if not r or len(data) != offset + (r + 127) // 128 * 128:
        raise ValueError('invalid MacBinary fork bounds')
    return data[65:73], data[128:128+d], data[offset:offset+r]

def native_app(path, accepted, name, finder):
    data = path.read_bytes()
    actual = forks(data)
    reference = forks(accepted.read_bytes())
    # Native HFS copies may rewrite the resource header's reserved bytes.
    normal = lambda r: r[:16] + r[128:]
    if (actual[0] != finder or actual[:2] != reference[:2]
            or len(actual[2]) != len(reference[2])
            or normal(actual[2]) != normal(reference[2])):
        raise ValueError(f'{name} differs from accepted offline readback')
    header = bytearray(data[:128])
    name = name.encode('ascii')
    header[1:65] = bytes([len(name)]) + name.ljust(63, b'\0')
    header[124:126] = binascii.crc_hqx(header[:124], 0).to_bytes(2, 'big')
    result = bytes(header) + data[128:]
    if forks(result) != actual:
        raise ValueError('MacBinary relabelling changed a fork or Finder identity')
    return result

def source_archive(path, product):
    busybox = (ROOT / BUSYBOX).read_bytes()
    if sha(busybox) != BUSYBOX_SHA:
        raise ValueError('BusyBox source archive hash mismatch')
    archive = git('archive', product, 'src', 'scripts', 'assets', 'licenses',
                  'tests', 'patches')
    with path.open('wb') as raw, gzip.GzipFile(filename='', fileobj=raw, mode='wb', mtime=0) as zipped:
        with tarfile.open(fileobj=zipped, mode='w', format=tarfile.PAX_FORMAT) as out:
            with tarfile.open(fileobj=io.BytesIO(archive)) as src:
                for member in src:
                    if not (member.isfile() or member.isdir()):
                        raise ValueError(f'unexpected source archive member: {member.name}')
                    out.addfile(member, src.extractfile(member) if member.isfile() else None)
            member = tarfile.TarInfo(BUSYBOX)
            member.size = len(busybox)
            member.mode = 0o644
            out.addfile(member, io.BytesIO(busybox))

def stage(product, accepted, updater_tar, out):
    subprocess.run(['git', '-C', str(ROOT), 'diff', '--quiet', product, '--',
                    'src', 'scripts', 'assets'], check=True)
    seal = release.verified(ROOT)
    build = seal['build']
    stem = f'Clarice-0.5.0-{build}'
    source_name = f'{stem}-Source.tar.gz'
    kit = ROOT / 'work/distribution/kit'
    core = (ROOT / 'work/build/Clarice.bin').read_bytes()
    if release.check_core(core, build) != (ROOT / 'work/update-command/clarice.clx').read_bytes():
        raise ValueError('bundled updater differs from the accepted updater')
    native_app(ROOT / 'work/build/Clarice.bin', accepted / 'Clarice.bin', 'Clarice', b'APPLCLRC')
    files = {
        'Clarice.bin': core,
        'Install Clarice.bin': native_app(ROOT / 'work/install-ux/build/Clarice.bin',
                                         accepted / 'Install.bin', 'Install Clarice', b'APPLCLRC'),
        'Recover Clarice.bin': native_app(ROOT / 'work/update-helper/ClariceUpdater.bin',
                                         accepted / 'Recover.bin', 'Recover Clarice', b'APPLCLUP'),
        'Release': (kit / 'Release').read_bytes(),
    }
    if files['Release'] != (accepted / 'Release').read_bytes():
        raise ValueError('release manifest differs from accepted offline readback')
    for record in seal['commands']:
        name = record['name']
        data = (ROOT / 'work/build' / (record['image'] + '.clx')).read_bytes()
        if sha(data) != record['sha256'] or (accepted / name).read_bytes() != data:
            raise ValueError(f'{name} differs from seal or accepted offline readback')
        files[f'Utilities/{name}'] = data
    updater_paths = {n for n in files if n.startswith('Utilities/')} | {'Release', 'Clarice.bin'}
    with tarfile.open(updater_tar) as tar:
        members = tar.getmembers()
        if len(members) != len(updater_paths) or {m.name for m in members} != updater_paths:
            raise ValueError('updater USTAR inventory mismatch')
        for member in members:
            if not member.isfile() or tar.extractfile(member).read() != files[member.name]:
                raise ValueError(f'updater USTAR mismatch: {member.name}')
    for path in sorted((ROOT/'licenses').rglob('*')):
        if path.is_file():
            files['Licenses/' + path.relative_to(ROOT/'licenses').as_posix()] = path.read_bytes()
    files.update({'Licenses/' + name: (ROOT/path).read_bytes() for name, path in NOTICES.items()})
    for name, path in [('Start Here.txt', 'RELEASE-START.md'), ('Release Notes.txt', 'RELEASE-NOTES.md'),
                       ('Third Party.txt', 'THIRD_PARTY.md')]:
        value = (ROOT / path).read_text().replace('@SOURCE_ARCHIVE@', source_name)
        files[name] = value.replace('\n', '\r').encode('mac_roman', errors='replace')
    provenance = dict(build=build, product_commit=product, seal_identity=seal['identity'],
                      updater_ustar_sha256=release.digest(updater_tar), source_archive=source_name,
                      licensing_status='local draft; see Third Party.txt',
                      files={name: sha(data) for name, data in sorted(files.items())})
    files['Package.json'] = (json.dumps(provenance, indent=2) + '\n').encode()
    if out.exists():
        raise FileExistsError(out)
    out.parent.mkdir(parents=True, exist_ok=True)
    target = out / f'{stem}-Installer.zip'
    with tempfile.TemporaryDirectory(dir=out.parent) as temp:
        draft = Path(temp) / 'draft'
        draft.mkdir()
        with zipfile.ZipFile(draft / target.name, 'w') as zip_file:
            for name, data in sorted(files.items()):
                info = zipfile.ZipInfo(f'Clarice 0.5.0/{name}')
                info.external_attr = 0o100644 << 16
                zip_file.writestr(info, data, compress_type=zipfile.ZIP_DEFLATED)
        with zipfile.ZipFile(draft / target.name) as zip_file:
            expected = {f'Clarice 0.5.0/{name}': data for name, data in files.items()}
            if (set(zip_file.namelist()) != set(expected)
                    or any(zip_file.read(name) != data for name, data in expected.items())):
                raise ValueError('installer archive readback mismatch')
        source_archive(draft / source_name, product)
        (draft / 'SHA256SUMS.txt').write_text(''.join(
            f'{release.digest(path)}  {path.name}\n' for path in sorted(draft.iterdir())))
        draft.rename(out)
    print(f'{target}: 3 MacBinary apps, {len(seal["commands"])} utilities, '
          f'{sum(n.startswith("Licenses/") for n in files)} notice files')

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--product', required=True)
    parser.add_argument('--offline-readback', required=True, type=Path)
    parser.add_argument('--updater-tar', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    a = parser.parse_args()
    product = git('rev-parse', '--verify', a.product + '^{commit}').decode().strip()
    stage(product, a.offline_readback, a.updater_tar, a.output)
