"""Validate captured installer UX evidence; no invented guest results."""
from pathlib import Path
import json,struct
r=Path(__file__).resolve().parents[2];o=r/'outputs/install-ux';w=r/'work/install-ux'
def forks(p):
 b=p.read_bytes();d,z=struct.unpack_from('>II',b,83);offset=128+(d+127)//128*128
 resource=b[offset:offset+z]
 return b[128:128+d],resource[:16]+resource[128:]
def metadata(p):
 b=p.read_bytes();return b[65:74]+b[91:99]+b[101:102]
assert (o/'ux-pwd.txt').read_text()=='/\n'
assert (o/'ux-awk.txt').read_text()=='installed\n'
assert len((o/'cancel-files.txt').read_text().splitlines())==6
assert len((o/'installed-files.txt').read_text().splitlines())==43
assert forks(w/'first/Clarice.bin')==forks(r/'outputs/distribution/build/Clarice.bin')
assert forks(w/'diskfull/Clarice.bin')==forks(w/'before-upgrade/Clarice.bin')
assert metadata(w/'diskfull/Clarice.bin')==metadata(w/'before-upgrade/Clarice.bin')
assert forks(w/'after-upgrade/Clarice.bin')==forks(r/'outputs/distribution/build/Clarice.bin')
assert forks(w/'after-upgrade/previous.bin')==forks(w/'before-upgrade/Clarice.bin')
assert metadata(w/'after-upgrade/previous.bin')==metadata(w/'before-upgrade/Clarice.bin')
assert (w/'after-upgrade/grep.bin').read_bytes()==(w/'before-upgrade/grep.bin').read_bytes()
assert (w/'after-upgrade/profile').read_bytes()==(w/'profile').read_bytes()
assert (w/'after-upgrade/user-file').read_bytes()==(w/'user-file').read_bytes()
assert (o/'upgrade-profile.txt').read_text()=='kept\n'
old=json.loads((r/'outputs/distribution/core-census.json').read_text())
new=json.loads((w/'core-census.json').read_text())
assert old['resources']==new['resources']
report={'ordinary_core_resources_byte_identical':True,'cancelled_after_commands':6,'installed_commands':43,'fresh_copy_forks_match':True,'disk_full_original_preserved':True,'upgrade_retains_previous_forks_metadata':True,'independent_command_exact_macbinary':True,'profile_and_user_file_unchanged':True,'guest':'Snow Plus / 4 MB / reported System 6.0.7 / maximum fast-forward','physical_acceptance':False}
(o/'results.json').write_text(json.dumps(report,indent=2)+'\n')
print('PASS: interrupted/resumed installation, fresh core, upgrade/previous version, independent utility, profile/user data and unchanged ordinary Core')
