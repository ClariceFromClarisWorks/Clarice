"""Publication replication must not expose incomplete or mutated collections."""
import binascii
import hashlib
from pathlib import Path
import sys
import tempfile
from unittest.mock import patch
sys.path.insert(0, str(Path(__file__).resolve().parents[2]/'scripts'))
import repository

def fixture(root, version, data):
    root.mkdir(exist_ok=True)
    digest = hashlib.sha256(data).hexdigest()
    leaf = digest[:24]+'.bin'
    (root/leaf).write_bytes(data)
    index = f'CLARICE-REPOSITORY 1\ncore\tClarice\t{version}\t{len(data)}\t{binascii.crc32(data):08x}\t{leaf}\n'.encode()
    (root/'index.txt').write_bytes(index)
    return index, leaf

with tempfile.TemporaryDirectory() as tmp:
    root=Path(tmp);src=root/'source';dst=root/'destination'
    first, leaf=fixture(src,1,b'first')
    repository.mirror(src,dst)
    assert (dst/'index.txt').read_bytes()==first
    (dst/'unrelated').write_bytes(b'keep')
    second, newleaf=fixture(src,2,b'second')
    with patch.object(repository,'immutable',side_effect=OSError('injected disk failure')):
        try: repository.mirror(src,dst)
        except OSError: pass
        else: raise AssertionError('failure not raised')
    assert (dst/'index.txt').read_bytes()==first
    (src/newleaf).write_bytes(b'broken')
    try: repository.mirror(src,dst)
    except ValueError: pass
    else: raise AssertionError('corruption accepted')
    assert (dst/'index.txt').read_bytes()==first
    (src/newleaf).unlink()
    try: repository.mirror(src,dst)
    except FileNotFoundError: pass
    else: raise AssertionError('missing payload accepted')
    assert (dst/'index.txt').read_bytes()==first
    fixture(src,2,b'second')
    write=repository.atomic_write
    def fail_index(path,data):
        if path.name=='index.txt': raise OSError('injected activation failure')
        return write(path,data)
    with patch.object(repository,'atomic_write',side_effect=fail_index):
        try: repository.mirror(src,dst)
        except OSError: pass
        else: raise AssertionError('activation failure not raised')
    assert (dst/'index.txt').read_bytes()==first
    assert (dst/newleaf).read_bytes()==b'second'
    repository.mirror(src,dst);repository.mirror(src,dst)
    assert (dst/'index.txt').read_bytes()==second
    assert (dst/leaf).read_bytes()==b'first'
    assert (dst/'unrelated').read_bytes()==b'keep'
    assert len(list(dst.glob('idx-*.txt')))==2
    for version in (1,2):
        fixture(src,version,b'mutated')
        try: repository.mirror(src,dst)
        except ValueError: pass
        else: raise AssertionError('downgrade/mutation accepted')
        assert (dst/'index.txt').read_bytes()==second
    assert not list(dst.glob('.pub-*'))
print('PASS identical mirror, repeat, failure/retry, missing/corrupt payload, downgrade, same-version mutation, retained recovery and unrelated files')
