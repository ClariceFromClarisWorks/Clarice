"""Read-only baseline reproductions; no production files replaced."""
from pathlib import Path
import subprocess,tempfile,sys,types,hashlib,binascii
r=Path(__file__).resolve().parents[2]
def old(p):return subprocess.check_output(['git','show','f64c9d0:'+p],cwd=r,text=True)
with tempfile.TemporaryDirectory() as d:
 t=Path(d);subprocess.run(['git','archive','f64c9d0','src/update'],cwd=r,stdout=(t/'old.tar').open('wb'),check=True);subprocess.run(['tar','xf',str(t/'old.tar'),'-C',d],check=True)
 # The production codecs accept a member that the production restore plan rejects.
 code='''#include <assert.h>\n#include <stdio.h>\n#include "src/update/recovery-record.h"\n#include "src/update/plan.h"\nint main(){RecoveryFile f={0};strcpy(f.name,"large");f.data=16777217;unsigned char b[RECOVERY_ENTRY];assert(!recovery_file_encode(&f,b));UpdatePlan p={0};p.kind=1;p.original_id=42;p.stage_id=2;p.build=2;p.data_size=f.data;strcpy(p.target,"Disk:large");strcpy(p.stage,"Disk:.cr-0000002a-00000002.new");strcpy(p.backup,"Disk:.cr-0000002a-00000002.old");assert(!update_plan_valid(&p));char a[256],c[256];assert(!recovery_member("Disk:snapshot","@core",a));assert(!recovery_member("Disk:snapshot","core",c));assert(!strcasecmp(a,c));puts("REPRO P2-4 accepted >16MiB member cannot restore; P2-5 Core/core HFS collision");}'''
 (t/'test.c').write_text(code);subprocess.run(['cc','-I'+d,str(t/'test.c'),'-o',str(t/'test')],check=True);subprocess.run([str(t/'test')],check=True)
 sys.path.insert(0,str(r/'scripts'));mod=types.ModuleType('oldrepository');exec(old('scripts/repository.py'),mod.__dict__)
 data=b'core';leaf=hashlib.sha256(data).hexdigest()[:24]+'.bin';(t/leaf).write_bytes(data)
 good=f'CLARICE-REPOSITORY 1\ncore\tClarice\t1\t4\t{binascii.crc32(data):08x}\t{leaf}\n'.encode()
 for bad in (good[:-1],good.replace(b'\n',b'\r\n')):(t/'index.txt').write_bytes(bad);mod.verified_repository(t)
 print('REPRO P2-7 missing final LF and CRLF accepted by baseline publisher')
 repo=old('src/commands/repository-client.inc');branch=repo[repo.index('if(r->core){core=i;'):repo.index('}else{\n   char path',repo.index('if(r->core){core=i;'))]+'}'
 code='''#include <assert.h>\n#include <stdio.h>\nint main(){struct R{int core,version;}record={1,10},*r=&record;struct I{int needed;}item={0},*i=&item,*core=0;int current=20,repository_migrate=1;'''+branch+'''assert(i->needed);puts("REPRO P2-1 older Core marked needed during root migration");}'''
 (t/'test.c').write_text(code);subprocess.run(['cc',str(t/'test.c'),'-o',str(t/'test')],check=True);subprocess.run([str(t/'test')],check=True)
 # Actual layout synchronisation path fails the new no-save assertion.
 geometry=(r/'tests/geometry/host.py').read_text().replace("s=Path('src/app.c').read_text()",'s='+repr(old('src/app.c'))).replace('static int font_layout(void){','static void font_layout(void){')
 result=subprocess.run([sys.executable,'-c',geometry],cwd=r,capture_output=True,text=True)
 assert result.returncode and 'saves==previous_saves' in result.stderr,result.stderr
 print('REPRO P2-6 failed reflow still saves preferences')
 # Run the production old copy routine with the existing native fault model.
 native=(r/'tests/recovery/native.c').read_text();start=native.index('int main(void){');native=native[:start]+'''int main(void){file("Disk:locked","both forks");fs[find("Disk:locked")].info.hFileInfo.ioFlAttrib|=1;RecoveryFile f;assert(recovery_copy("Disk:locked","Disk:copy","ls",&f)==permErr);puts("REPRO P2-3 read-only source incorrectly rejected");}\n'''
 p=t/'tests/recovery';p.mkdir(parents=True);(p/'native.c').write_text(native)
 subprocess.run(['cc','-Wno-multichar','-I'+str(r/'tests/recovery/include'),str(p/'native.c'),'-o',str(t/'native')],check=True);subprocess.run([str(t/'native')],check=True)
 # In the old path changes==0 only suppresses confirmation; recovery is unconditional.
 middle=repo[repo.index(' if(check)goto done;'):repo.index(' if(repository_migrate){')]
 assert 'if(!changes)' not in middle and 'repo_recovery_prepare(items,own,current,index_size,index_crc,changes)' in middle and 'e=repo_marker(text)' in middle
 print('CONFIRM P2-2 no-change branch unconditionally prepares recovery and writes marker (new regression executes corrected branch)')
