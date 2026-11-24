#!/usr/bin/env python3
"""Prepare a private HTTP or mounted-volume repository from an existing verified complete build."""
import argparse
import binascii
import os
import re
import tempfile
import hashlib
from pathlib import Path
import release


def publish(root, destination):
    sealed = release.verified(root)
    destination.mkdir(parents=True, exist_ok=True)
    records = [('core', 'Clarice', root/'work/build/Clarice.bin', sealed['core'])]
    records += [('tool', r['name'], root/f"work/build/{r['image']}.clx", r)
                for r in sealed['commands']]
    lines = ['CLARICE-REPOSITORY 1']
    # Immutable payload names; the small index becomes visible only when all
    # payloads exist. Failed publication leaves the previous index usable.
    for kind, name, source, record in records:
        data = source.read_bytes()
        digest = hashlib.sha256(data).hexdigest()
        if len(data) != record['bytes'] or digest != record['sha256']:
            raise ValueError(f'build changed while publishing: {name}')
        suffix = 'bin' if kind == 'core' else 'clx'
        # Classic AFP/HFS clients require leaves no longer than 31 bytes.
        # Full SHA-256 still validates inputs; existing-name collisions fail below.
        filename = f'{digest[:24]}.{suffix}'
        target = destination/filename
        if target.exists():
            if target.read_bytes() != data:
                raise ValueError(f'conflicting repository payload: {filename}')
        else:
            temporary = destination/(filename+'.partial')
            with temporary.open('xb') as f:
                f.write(data)
            temporary.replace(target)
        lines.append(f"{kind}\t{name}\t{sealed['build']}\t{len(data)}\t{record['crc32']}\t{filename}")
    previous = destination/'index.txt'
    if previous.exists():
        old = {tuple(row.split('\t')[:3]): row for row in previous.read_text().splitlines()[1:]}
        for row in lines[1:]:
            key = tuple(row.split('\t')[:3])
            if key in old and old[key] != row:
                raise ValueError(f'already published version changed: {key[1]} {key[2]}')
    temporary = destination/'index.txt.partial'
    with temporary.open('x', encoding='ascii', newline='\n') as f:
        f.write('\n'.join(lines)+'\n')
    temporary.replace(destination/'index.txt')
    return destination/'index.txt'

def verified_repository(directory):
    """Read an existing publication once; verify bytes before any activation."""
    index = (directory/'index.txt').read_bytes()
    if not index.endswith(b'\n') or b'\r' in index or b'\x00' in index:
        raise ValueError('repository requires LF-terminated lines')
    rows = index.decode('ascii').split('\n')[:-1]
    if any(len(row) >= 256 for row in rows):
        raise ValueError('repository line exceeds guest reader capacity')
    if not rows or rows[0] != 'CLARICE-REPOSITORY 1':
        raise ValueError('invalid repository header')
    records, names = [], set()
    for row in rows[1:]:
        kind, name, version, length, crc, leaf = row.split('\t')
        if kind not in ('core', 'tool') or (kind, name) in names:
            raise ValueError('invalid or duplicate repository entry')
        names.add((kind, name))
        if not re.fullmatch(r'[0-9a-f]{24}\.(bin|clx)', leaf):
            raise ValueError('invalid immutable payload name')
        if int(version) <= 0 or int(length) <= 0:
            raise ValueError('invalid version or length')
        data = (directory/leaf).read_bytes()
        if (len(data) != int(length) or f'{binascii.crc32(data):08x}' != crc
                or not hashlib.sha256(data).hexdigest().startswith(leaf[:24])):
            raise ValueError(f'corrupt repository payload: {leaf}')
        records.append((row, leaf, data))
    if sum(row.startswith('core\tClarice\t') for row, _, _ in records) != 1:
        raise ValueError('repository needs exactly one Clarice Core')
    return index, records


def immutable(path, data):
    if path.exists():
        if path.read_bytes() != data:
            raise ValueError(f'conflicting immutable file: {path.name}')
        return
    atomic_write(path, data)
    if path.read_bytes() != data:
        raise ValueError(f'publication readback failed: {path.name}')


def atomic_write(path, data):
    # Short same-directory temporary names also work through classic AFP.
    fd, temporary = tempfile.mkstemp(prefix='.pub-', dir=path.parent)
    try:
        with os.fdopen(fd, 'wb') as stream:
            stream.write(data)
            stream.flush()
            os.fsync(stream.fileno())
        Path(temporary).replace(path)
    finally:
        Path(temporary).unlink(missing_ok=True)


def mirror(source, destination):
    """One verified publication, copied unchanged; active index always last."""
    index, records = verified_repository(source)
    destination.mkdir(parents=True, exist_ok=True)
    previous = destination/'index.txt'
    if previous.exists():
        old_index = previous.read_bytes()
        old = {tuple(row.split('\t')[:2]): row.split('\t')
               for row in old_index.decode('ascii').splitlines()[1:]}
        for row, _, _ in records:
            fields = row.split('\t'); prior = old.get(tuple(fields[:2]))
            if prior and (int(prior[2]) > int(fields[2]) or
                          (prior[2] == fields[2] and prior != fields)):
                raise ValueError('refusing downgrade or same-version mutation')
        immutable(destination/('idx-'+hashlib.sha256(old_index).hexdigest()[:20]+'.txt'), old_index)
    for _, leaf, data in records:
        immutable(destination/leaf, data)
    # Preserve a versioned index before exposing it as current.
    immutable(destination/('idx-'+hashlib.sha256(index).hexdigest()[:20]+'.txt'), index)
    atomic_write(previous, index)
    verified_repository(destination)
    return previous


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('directory', type=Path)
    parser.add_argument('--mirror-from', type=Path,
                        help='copy a verified publication unchanged, active index last')
    args = parser.parse_args()
    print(mirror(args.mirror_from, args.directory) if args.mirror_from else
          publish(Path(__file__).resolve().parents[1], args.directory))
