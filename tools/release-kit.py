#!/usr/bin/env python3
"""Stage a fork-safe distribution from an already accepted build."""
import argparse
import binascii
import gzip
import hashlib
import io
import json
import re
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
# Full pinned upstream trees retain runtime sources, build scripts and notices.
# Source provenance is tied to this image by licenses/runtime-build-evidence.txt.
RUNTIME_SOURCES = {
    'Retro68-2aa5c87e88771c6057f52bf830af9d440a5e4e36.tar.gz':
        '2b68bb7ab2440083ab78ede1c17bed8fe6bf43c4f38434ed92438d98eb0f2d95',
    'multiversal-580f3309649ad4c788c048be3dff2416d7ba49f3.tar.gz':
        '2534535e1eaca0a0bb360998528f0c8ba4c2e57189cf7bc56cd9e88c721aa507',
}
NOTICES = {
    'GPL-2.0.txt': 'LICENSE',
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

SOURCE_DOCS = (
    'LICENSE', 'README.md', 'INSTALL.md', 'THIRD_PARTY.md', 'RELEASE-START.md',
    'RELEASE-NOTES.md', 'RELEASE-PACKAGING.md', 'SOURCE-BUILD.md',
    'TOOLCHAIN-PROVENANCE.md', 'LICENSING-DECISIONS.md',
    'tools/release-kit.py', 'tools/classic-installer.py',
)
BUSYBOX_CHANGES = {
    'sort.c': 'Reject unavailable options and poll cooperatively in key comparison.',
    'full_read.c': 'Extract full_read from libbb/read.c; Clarice supplies safe_read.',
    'sed.c': 'Reject in-place editing and poll the execution loops.',
    'grep.c': 'Remove the recursive file-walking path and reject recursive options.',
    'tee.c': 'Reject the interrupt-ignore option.',
    'uniq.c': 'Adapt stream selection and I/O error reporting; reject output operand.',
    'tail.c': 'Remove following and reject follow options.',
    'cksum.c': 'Check close errors on non-stdin descriptors.',
}

def busybox_notice(change):
    return (f'/* CLARICE MODIFICATION NOTICE (2026-11-18; recorded 2026-11-26).\n'
            f' * Clarice modified this BusyBox 1.37.0 source: {change}\n'
            ' * Upstream notices and licensing terms are retained.\n'
            ' */\n').encode('ascii')

def source_archive(path, product, seal, updater_sha):
    busybox = (ROOT / BUSYBOX).read_bytes()
    if sha(busybox) != BUSYBOX_SHA:
        raise ValueError('BusyBox source archive hash mismatch')
    archive = git('archive', product, 'src', 'scripts', 'assets', 'licenses',
                  'tests', 'patches')
    with tarfile.open(fileobj=io.BytesIO(archive)) as src:
        original = {}
        modes = {}
        for member in src:
            if member.isfile():
                original[member.name] = src.extractfile(member).read()
                modes[member.name] = member.mode
            elif not member.isdir():
                raise ValueError(f'unexpected source archive member: {member.name}')
    files = dict(original)
    files.update({name: (ROOT/name).read_bytes() for name in SOURCE_DOCS})
    for notice in sorted((ROOT/'licenses').rglob('*')):
        if notice.is_file():
            files[notice.relative_to(ROOT).as_posix()] = notice.read_bytes()
    files[BUSYBOX] = busybox
    for name, expected in RUNTIME_SOURCES.items():
        leaf = 'work/runtime-source/' + name
        data = (ROOT/leaf).read_bytes()
        if sha(data) != expected:
            raise ValueError(f'runtime source archive hash mismatch: {name}')
        files[leaf] = data
    # Keep the accepted application/build checkout untouched. The companion
    # regenerates prepared files with prominent, comments-only port notices.
    prepare = 'scripts/prepare-busybox-tools.py'
    overlay = '\n# Release companion: preserve notices on regenerated source files.\n'
    for name, change in BUSYBOX_CHANGES.items():
        overlay += (f"p=w/{name!r}; p.write_bytes("
                    f"{busybox_notice(change)!r}+p.read_bytes())\n")
    files[prepare] += overlay.encode('ascii')
    with_vi = any(record['name'] == 'vi' for record in seal['commands'])
    vi_prepare = 'scripts/prepare-busybox-vi.py'
    with tempfile.TemporaryDirectory() as directory:
        scratch = Path(directory)
        source_script = 'scripts/prepare-busybox-source.py'
        inputs = (BUSYBOX, source_script, prepare, 'src/ports/busybox-tools/help.json')
        if with_vi:
            inputs += (vi_prepare, 'src/ports/busybox/upstream.patch')
        for name in inputs:
            dest = scratch/name
            dest.parent.mkdir(parents=True, exist_ok=True)
            dest.write_bytes(files[name])
        subprocess.run([sys.executable, str(scratch/source_script)], check=True)
        subprocess.run([sys.executable, str(scratch/prepare)], check=True)
        for name in ('wc.c', 'tr.c', 'sort.c', 'full_read.c',
                     'config.h', 'help.h', 'unicode.h', 'xregex.h'):
            leaf = 'work/busybox-tools/' + name
            prepared = (scratch/leaf).read_bytes()
            notice = busybox_notice(BUSYBOX_CHANGES[name]) if name in BUSYBOX_CHANGES else b''
            if prepared != notice + (ROOT/leaf).read_bytes():
                raise ValueError(f'{name} does not match the accepted prepared input')
            files[leaf] = prepared
        if with_vi:
            subprocess.run([sys.executable, str(scratch/vi_prepare)], check=True)
            leaf = 'work/busybox/vi.c'
            prepared = (scratch/leaf).read_bytes()
            if prepared != (ROOT/leaf).read_bytes():
                raise ValueError('vi.c does not match the accepted prepared input')
            files[leaf] = prepared
    files['BUILD-SEAL.json'] = (ROOT/'work/release/sealed.json').read_bytes()
    manifest = dict(build=seal['build'], product_commit=product,
                    binary_seal_identity=seal['identity'], updater_ustar_sha256=updater_sha,
                    product_overlay={prepare: sha(original[prepare])},
                    files={name: sha(data) for name, data in sorted(files.items())})
    files['SOURCE-MANIFEST.json'] = (json.dumps(manifest, indent=2) + '\n').encode()
    with path.open('wb') as raw, gzip.GzipFile(filename='', fileobj=raw, mode='wb', mtime=0) as zipped:
        with tarfile.open(fileobj=zipped, mode='w', format=tarfile.PAX_FORMAT) as out:
            for name, data in sorted(files.items()):
                member = tarfile.TarInfo(name)
                member.size = len(data)
                member.mode = modes.get(name, 0o644)
                out.addfile(member, io.BytesIO(data))
    with tarfile.open(path) as check:
        members = check.getmembers()
        if ({m.name for m in members} != set(files) or len(members) != len(files)
                or any(not m.isfile() or check.extractfile(m).read() != files[m.name]
                       for m in members)):
            raise ValueError('source archive readback mismatch')

def stage(product, accepted, updater_tar, out, version, final=False):
    subprocess.run(['git', '-C', str(ROOT), 'diff', '--quiet', product, '--',
                    'src', 'scripts', 'assets'], check=True)
    seal = release.verified(ROOT)
    build = seal['build']
    stem = f'Clarice-{version}-{build}'
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
    licensing = ('GPL-2.0-only original contributions; see Third Party.txt for applicable licences and source terms'
                 if final else
                 'GPL-2.0-only original contributions; third-party permission review pending; see Third Party.txt')
    provenance = dict(build=build, release_version=version, product_commit=product, seal_identity=seal['identity'],
                      updater_ustar_sha256=release.digest(updater_tar), source_archive=source_name,
                      licensing_status=licensing,
                      files={name: sha(data) for name, data in sorted(files.items())})
    if out.exists():
        raise FileExistsError(out)
    out.parent.mkdir(parents=True, exist_ok=True)
    target = out / f'{stem}-Installer.zip'
    with tempfile.TemporaryDirectory(dir=out.parent) as temp:
        draft = Path(temp) / 'draft'
        draft.mkdir()
        source = draft / source_name
        source_archive(source, product, seal, provenance['updater_ustar_sha256'])
        provenance['source_sha256'] = release.digest(source)
        files['Package.json'] = (json.dumps(provenance, indent=2) + '\n').encode()
        with zipfile.ZipFile(draft / target.name, 'w') as zip_file:
            for name, data in sorted(files.items()):
                info = zipfile.ZipInfo(f'Clarice {version}/{name}')
                info.external_attr = 0o100644 << 16
                zip_file.writestr(info, data, compress_type=zipfile.ZIP_DEFLATED)
        with zipfile.ZipFile(draft / target.name) as zip_file:
            expected = {f'Clarice {version}/{name}': data for name, data in files.items()}
            if (set(zip_file.namelist()) != set(expected)
                    or any(zip_file.read(name) != data for name, data in expected.items())):
                raise ValueError('installer archive readback mismatch')
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
    parser.add_argument('--version', default='0.5.0')
    parser.add_argument('--final', action='store_true', help='Use final release-kit wording')
    a = parser.parse_args()
    if not re.fullmatch(r'[0-9]+\.[0-9]+\.[0-9]+', a.version):
        parser.error('version must have three numeric components')
    product = git('rev-parse', '--verify', a.product + '^{commit}').decode().strip()
    stage(product, a.offline_readback, a.updater_tar, a.output, a.version, a.final)
