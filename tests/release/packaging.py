"""Host-only pipeline tests. Native installation acceptance is separate."""
import importlib.util
import json
from pathlib import Path
import shutil
import tempfile
import unittest
spec=importlib.util.spec_from_file_location('release',Path(__file__).resolve().parents[2]/'scripts/release.py')
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
class Packaging(unittest.TestCase):
 def setUp(self):
  self.tmp=tempfile.TemporaryDirectory();self.addCleanup(self.tmp.cleanup);self.r=Path(self.tmp.name)
  for d in ['src/update','scripts','assets','work/build','work/release']:(self.r/d).mkdir(parents=True,exist_ok=True)
  (self.r/'src/update/version.h').write_text('#define CLARICE_UPDATE_BUILD 42\n')
  (self.r/'src/standard-commands.inc').write_text('CLARICE_COMMAND("ls", "core")\nCLARICE_COMMAND("cat", "core")\n')
 def sealed(self):
  r=self.r
  (r/'work/build/Clarice.bin').write_bytes(b'fixture-core')
  (r/'work/build/core.clx').write_bytes(b'fixture-command')
  core=dict(bytes=12,sha256=m.digest(r/'work/build/Clarice.bin'),crc32='00000000')
  commands=[dict(name=n,image='core',bytes=15,sha256=m.digest(r/'work/build/core.clx'),crc32='00000000') for n in ['ls','cat']]
  data=dict(format=1,build=42,inputs=m.inputs(r),session='fixture',core=core,commands=commands)
  data['identity']=m.hashlib.sha256(json.dumps(data,sort_keys=True).encode()).hexdigest()
  m.save(r/'work/release/sealed.json',data)
  return data
 def test_missing_seal(self):
  with self.assertRaises(FileNotFoundError):m.verified(self.r)
 def test_missing_command(self):
  self.sealed();(self.r/'work/build/core.clx').unlink()
  with self.assertRaises(FileNotFoundError):m.verified(self.r)
 def test_old_command(self):
  self.sealed();(self.r/'work/build/core.clx').write_bytes(b'older-command!!')
  with self.assertRaises(ValueError):m.verified(self.r)
 def test_changed_core(self):
  self.sealed();(self.r/'work/build/Clarice.bin').write_bytes(b'old-app-core')
  with self.assertRaises(ValueError):m.verified(self.r)
 def test_incomplete_new_build_cannot_reuse_old(self):
  self.sealed();m.begin(self.r)
  self.assertFalse((self.r/'work/build/core.clx').exists())
  self.assertFalse((self.r/'work/release/sealed.json').exists())
  self.assertTrue(list((self.r/'work').glob('build-before-*/core.clx')))
  with self.assertRaises(ValueError):m.seal(self.r)
 def test_source_change(self):
  self.sealed();(self.r/'src/new.c').write_text('changed')
  with self.assertRaises(ValueError):m.verified(self.r)
 def kit(self):
  self.sealed();k=self.r/'work/distribution/kit';(k/'Utilities').mkdir(parents=True)
  shutil.copyfile(self.r/'work/build/Clarice.bin',k/'Clarice.bin')
  for n in ['ls','cat']:shutil.copyfile(self.r/'work/build/core.clx',k/'Utilities'/n)
  return k
 def test_stale_kit(self):
  k=self.kit();(k/'Utilities/stale').write_bytes(b'old')
  with self.assertRaises(ValueError):m.bundle(self.r,k)
 def test_replaced_kit(self):
  k=self.kit();(k/'Utilities/cat').write_bytes(b'older-command!!')
  with self.assertRaises(ValueError):m.bundle(self.r,k)
 def test_complete_bundle(self):
  k=self.kit();p=m.bundle(self.r,k)
  with m.tarfile.open(p) as t:
   self.assertEqual(t.getnames(),['Release','Clarice.bin','Utilities/ls','Utilities/cat'])
   self.assertTrue(all(i.isfile() for i in t))
   self.assertEqual(t.extractfile('Utilities/cat').read(),b'fixture-command')
  original=p.read_bytes();m.bundle(self.r,k);self.assertEqual(original,p.read_bytes())
 def test_invalid_core(self):
  with self.assertRaises(ValueError):m.check_core(b'invalid',42)
if __name__=='__main__':unittest.main()
