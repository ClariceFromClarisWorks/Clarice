import binascii
import hashlib
from pathlib import Path
import sys
import tempfile
from unittest.mock import patch
sys.path.insert(0, str(Path(__file__).resolve().parents[2]/'scripts'))
import repository
with tempfile.TemporaryDirectory() as directory:
    root=Path(directory); build=root/'work/build';build.mkdir(parents=True)
    core=b'core fixture'; tool=b'command fixture'
    (build/'Clarice.bin').write_bytes(core);(build/'core.clx').write_bytes(tool)
    def record(data):
        return dict(bytes=len(data),sha256=hashlib.sha256(data).hexdigest(),crc32=f'{binascii.crc32(data):08x}')
    manifest=dict(build=42,core=record(core),commands=[dict(name='ls',image='core',**record(tool))])
    repo=root/'repo'
    with patch.object(repository.release,'verified',return_value=manifest):
        index=repository.publish(root,repo); original=index.read_bytes()
        assert b'tool\tls\t42\t' in original
        repository.publish(root,repo);assert index.read_bytes()==original
        (build/'core.clx').write_bytes(b'changed')
        try: repository.publish(root,repo)
        except ValueError: pass
        else: raise AssertionError('changed payload accepted')
        assert index.read_bytes()==original
        # Even a newly sealed payload must not change an existing version.
        manifest['commands'][0].update(record(b'changed'))
        try: repository.publish(root,repo)
        except ValueError: pass
        else: raise AssertionError('same-version mutation accepted')
        assert index.read_bytes()==original
        (build/'core.clx').unlink()
        try: repository.publish(root,repo)
        except FileNotFoundError: pass
        else: raise AssertionError('missing payload accepted')
        assert index.read_bytes()==original
print('PASS repository publication, repeat publication, changed/missing payload and preservation of previous index')
