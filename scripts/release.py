#!/usr/bin/env python3
"""Coherent release build provenance and deterministic, single-download USTAR kit.
Host-only: no hashes or caches added to the idle Macintosh runtime.
"""
import argparse
import binascii
import hashlib
import io
import json
from pathlib import Path
import re
import shutil
import struct
import subprocess
import tarfile
import uuid

ROOT = Path(__file__).resolve().parents[1]

def digest(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()

def entries(root):
    pairs = re.findall(r'CLARICE_COMMAND\("([a-z0-9_-]+)", "([a-z0-9_-]+)"\)',
                       (root/'src/standard-commands.inc').read_text())
    if not pairs or len(pairs) != len({n for n, _ in pairs}):
        raise ValueError('invalid standard command inventory')
    return pairs

def inputs(root):
    h = hashlib.sha256()
    for folder in ('src', 'scripts', 'assets'):
        for p in sorted((root/folder).rglob('*')):
            if p.is_file() and '__pycache__' not in p.parts:
                h.update(str(p.relative_to(root)).encode()+b'\0')
                h.update(p.read_bytes())
    return h.hexdigest()

def build_number(root):
    return int(re.search(r'#define CLARICE_UPDATE_BUILD (\d+)',
                          (root/'src/update/version.h').read_text())[1])

def save(path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    tmp = path.with_suffix(path.suffix+'.writing')
    tmp.write_text(json.dumps(data, indent=2)+'\n')
    tmp.replace(path)

def begin(root):
    # Move, never erase, the prior build. Failure cannot expose its outputs as
    # products of this invocation. Also invalidates any previously sealed kit.
    state = root/'work/release'
    state.mkdir(parents=True, exist_ok=True)
    (state/'sealed.json').unlink(missing_ok=True)
    session = uuid.uuid4().hex
    build = root/'work/build'
    if build.exists():
        build.rename(root/'work'/('build-before-'+session))
    build.mkdir()
    save(state/'session.json', dict(session=session, build=build_number(root),
                                  inputs=inputs(root)))

def check_core(data, build):
    if len(data) < 128 or data[65:73] != b'APPLCLRC':
        raise ValueError('not a Clarice MacBinary application')
    if binascii.crc_hqx(data[:124], 0) != int.from_bytes(data[124:126], 'big'):
        raise ValueError('MacBinary header checksum')
    d, r = struct.unpack_from('>II', data, 83)
    off = 128 + ((d+127)//128)*128
    if not r or len(data) != off+((r+127)//128)*128:
        raise ValueError('MacBinary fork lengths')
    resource = data[off:off+r]
    if len(resource) < 16:
        raise ValueError('missing resource map')
    rd, rm, dl, ml = struct.unpack_from('>4I', resource)
    if rd+dl > r or rm+ml > r or ml < 30:
        raise ValueError('resource bounds')
    types = rm+struct.unpack_from('>H', resource, rm+24)[0]
    if types+2 > rm+ml:
        raise ValueError('resource types bounds')
    count = struct.unpack_from('>H', resource, types)[0]+1
    if types+2+count*8 > rm+ml:
        raise ValueError('resource type table bounds')
    stamp = None
    updater = None
    code = False
    for i in range(count):
        pos = types+2+i*8
        kind = resource[pos:pos+4]
        n, refs = struct.unpack_from('>HH', resource, pos+4)
        refs += types
        if refs+(n+1)*12 > rm+ml:
            raise ValueError('resource references bounds')
        for j in range(n+1):
            ref = refs+j*12
            ident = struct.unpack_from('>h', resource, ref)[0]
            offset = rd+int.from_bytes(resource[ref+5:ref+8], 'big')
            if offset+4 > rd+dl:
                raise ValueError('resource data bounds')
            length = struct.unpack_from('>I', resource, offset)[0]
            if offset+4+length > rd+dl:
                raise ValueError('resource length bounds')
            if kind == b'CODE' and ident == 0:
                code = True
            if kind == b'CLXU' and ident == 128:
                updater = resource[offset+4:offset+4+length]
            if kind == b'CLUP' and ident == 128:
                stamp = resource[offset+4:offset+4+length]
    if not code or stamp != struct.pack('>4sI4H', b'CLU1', build, 0x600, 1, 1, 2):
        raise ValueError('Core build/compatibility stamp does not match release')
    return updater

def verify_commands(root, paths):
    # Reuse the actual production CLX loader and import table via the established
    # native-file mock; do not create another CLX validator.
    work = root/'work/release'
    checker = work/'check-clx'
    subprocess.run(['cc', '-std=c99', '-Wno-multichar', '-Wno-unused-function',
                    '-Wno-misleading-indentation', str(root/'tests/commands/update-host.c'),
                    '-o', str(checker)], check=True)
    names = re.findall(r'CLX_IMPORT\("([^"\n]+)"', (root/'src/clx-imports.inc').read_text())
    exports = work/'exports.txt'
    exports.write_text('\n'+'\n'.join(sorted(set(names)))+'\n')
    subprocess.run([str(checker), str(exports), *map(str, paths)], check=True)

def seal(root):
    state = root/'work/release'
    (state/'sealed.json').unlink(missing_ok=True)
    session = json.loads((state/'session.json').read_text())
    if session['inputs'] != inputs(root) or session['build'] != build_number(root):
        raise ValueError('source changed during build; restart the complete build')
    inventory = entries(root)
    paths = [root/'work/build'/f'{image}.clx' for image in sorted({i for _, i in inventory})]
    for p in paths:
        if not p.is_file() or p.is_symlink():
            raise ValueError(f'missing command build: {p.name}')
    updater = root/'work/update-command/clarice.clx'
    verify_commands(root, paths+[updater])
    core = root/'work/build/Clarice.bin'
    if check_core(core.read_bytes(), session['build']) != updater.read_bytes():
        raise ValueError('Core bundled updater does not match validated build')
    records = [dict(name=name, image=image, bytes=(root/f'work/build/{image}.clx').stat().st_size,
                    sha256=digest(root/f'work/build/{image}.clx'),
                    crc32=f'{binascii.crc32((root/f"work/build/{image}.clx").read_bytes()):08x}')
               for name, image in inventory]
    release = dict(format=1, build=session['build'], session=session['session'],
                   inputs=session['inputs'], core=dict(bytes=core.stat().st_size, sha256=digest(core),
                             crc32=f'{binascii.crc32(core.read_bytes()):08x}'),
                   commands=records)
    release['identity'] = hashlib.sha256(json.dumps(release, sort_keys=True).encode()).hexdigest()
    save(state/'sealed.json', release)
    return release

def verified(root):
    release = json.loads((root/'work/release/sealed.json').read_text())
    identity = release.pop('identity')
    if hashlib.sha256(json.dumps(release, sort_keys=True).encode()).hexdigest() != identity:
        raise ValueError('release manifest identity mismatch')
    release['identity'] = identity
    if release['inputs'] != inputs(root) or release['build'] != build_number(root):
        raise ValueError('source/build changed since complete build')
    if [(x['name'], x['image']) for x in release['commands']] != entries(root):
        raise ValueError('incomplete command inventory')
    for path, rec in [(root/'work/build/Clarice.bin', release['core'])]+[
            (root/f'work/build/{x["image"]}.clx', x) for x in release['commands']]:
        if path.is_symlink() or path.stat().st_size != rec['bytes'] or digest(path) != rec['sha256']:
            raise ValueError(f'missing, replaced or stale build output: {path.name}')
    return release

def bundle(root, kit):
    release = verified(root)
    expected = {x['name'] for x in release['commands']}
    if {p.name for p in (kit/'Utilities').iterdir()} != expected:
        raise ValueError('kit contains missing or unexpected commands')
    for path, rec in [(kit/'Clarice.bin', release['core'])]+[
            (kit/'Utilities'/x['name'], x) for x in release['commands']]:
        if path.is_symlink() or digest(path) != rec['sha256']:
            raise ValueError(f'kit differs from sealed build: {path.name}')
    save(kit/'release.json', release)
    # Small endian-independent manifest for native streaming installation.
    manifest = bytearray(struct.pack('>8sII32sII', b'CLREL001', release['build'],
        len(release['commands']), bytes.fromhex(release['identity']),
        release['core']['bytes'], int(release['core']['crc32'], 16)))
    for item in release['commands']:
        manifest.extend(struct.pack('>24sII', item['name'].encode(), item['bytes'],
                                    int(item['crc32'], 16)))
    manifest.extend(struct.pack('>I', binascii.crc32(manifest)))
    (kit/'Release').write_bytes(manifest)
    target = kit.parent/'Clarice-release.tar'
    temporary = target.with_suffix('.writing')
    # Plain USTAR: no decompressor cost on 68000; only approved regular paths.
    with tarfile.open(temporary, 'w', format=tarfile.USTAR_FORMAT) as archive:
        for p in [kit/'Release', kit/'Clarice.bin']+[
                kit/'Utilities'/x['name'] for x in release['commands']]:
            data = p.read_bytes()
            info = tarfile.TarInfo(p.relative_to(kit).as_posix())
            info.size = len(data)
            info.mode = 0o644
            archive.addfile(info, io.BytesIO(data))
    temporary.replace(target)
    return target

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('action', choices=['begin', 'seal', 'check', 'bundle'])
    a = parser.parse_args()
    try:
        if a.action == 'begin': begin(ROOT)
        elif a.action == 'seal': seal(ROOT)
        elif a.action == 'check': verified(ROOT)
        else: print(bundle(ROOT, ROOT/'work/distribution/kit'))
    except (ValueError, OSError, KeyError, subprocess.CalledProcessError) as e:
        parser.exit(1, f'Release consistency: {e}\n')
