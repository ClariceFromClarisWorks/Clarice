"""Actual CLX verifier under an allocation ceiling; optional pre-fix comparison.
Usage: python3 tests/recovery/verify-memory.py NANO_PROFILE_CLX [OLD_MODULE OLD_INSTALL]
Uses the existing native API shim; this is host evidence, not guest heap timing.
"""
from pathlib import Path
import re,subprocess,tempfile,sys
root=Path.cwd();fixture=Path(sys.argv[1]).resolve()
prefix=(root/'tests/commands/update-host.c').read_text().split('int main(')[0]
exports='\n'+'\n'.join(re.findall(r'CLX_IMPORT\("([^"\n]+)"',(root/'src/clx-imports.inc').read_text()))+'\n'
with tempfile.TemporaryDirectory() as td:
 d=Path(td);(d/'exports').write_text(exports)
 for old in ([True,False] if len(sys.argv)>2 else [False]):
  module=(Path(sys.argv[2]) if old else root/'src/module.inc').read_text()
  module=module.replace('#include "clx-record.inc"',f'#include "{root}/src/clx-record.inc"')
  install=(Path(sys.argv[3]) if old else root/'src/command-install.inc').read_text().replace('#include "standard-commands.inc"',f'#include "{root}/src/standard-commands.inc"')
  (d/'install.inc').write_text(install);module=module.replace('#include "command-install.inc"',f'#include "{d}/install.inc"');(d/'module.inc').write_text(module)
  text=prefix.replace('#include "../../src/module.inc"',f'#include "{d}/module.inc"')
  text+='''int main(int argc,char **argv){(void)argc;FILE *f=fopen(argv[1],"rb");assert(f);fread(export_names,1,sizeof export_names-1,f);fclose(f);f=fopen(argv[2],"rb");assert(f);strcpy(files[0].name,dest);files[0].len=fread(files[0].data,1,sizeof files[0].data,f);fclose(f);allocation_limit=131072;int e=cli_appcheck(dest);printf("status=%d requests=%d largest_request=%lu retained=%d\\n",e,alloc_calls,largest_request,allocs);assert(!allocs&&!files[0].open);'''
  text+=('assert(e==paramErr&&largest_request==clx_u32(files[0].data+8));' if old else 'assert(!e&&!alloc_calls);allocation_limit=0;assert(!cli_appcheck(dest));assert(!alloc_calls&&!allocs);')+'return 0;}'
  (d/'test.c').write_text(text)
  subprocess.run(['cc','-Wno-multichar','-fsanitize=address,undefined',str(d/'test.c'),'-o',str(d/'test')],check=True)
  print('BEFORE' if old else 'AFTER',flush=True);subprocess.run([str(d/'test'),str(d/'exports'),str(fixture)],check=True)
