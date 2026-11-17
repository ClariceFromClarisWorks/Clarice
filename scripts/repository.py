#!/usr/bin/env python3
"""Prepare a private HTTP repository from an existing verified complete build."""
import argparse
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
        filename = f'{name}-{digest}.{suffix}'
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

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('directory', type=Path)
    args = parser.parse_args()
    print(publish(Path(__file__).resolve().parents[1], args.directory))
