from pathlib import Path
p=Path('outputs/ls-metadata')
visible=(p/'visible-only.txt').read_text().splitlines();assert visible==['folder/','visible'],visible
all_text=(p/'visible-all.txt').read_text();lines=all_text.splitlines()
assert len(lines)==7,lines
for name in ['.dot','invisible','visible']:
 line=next(x for x in lines if x.endswith(' '+name));assert '2000-02-29 12:34' in line and '2025-01-01' not in line,line
 assert line.split()[1:3]==['4','0'],line
for name in ['folder/','hidden-folder/']:
 line=next(x for x in lines if x.endswith(' '+name));assert line[4:26].strip()=='',line
assert next(x for x in lines if x.endswith(' invisible')).startswith('--I ')
assert next(x for x in lines if x.endswith(' hidden-folder/')).startswith('d-I ')
full=(p/'visible-full.txt').read_text();assert 'TEXT TEST' in full and '4000' in full and '---- ----' in full
explicit=(p/'visible-explicit.txt').read_text();assert '2000-02-29 12:34' in explicit and '/invisible' in explicit
help_text=(p/'ls-help.txt').read_text();assert 'Created: YYYY-MM-DD HH:MM' in help_text and 'Stored ???? is literal' in help_text
print('PASS: default filtering, -a invisible files/folders/dot names, explicit hidden operand, leap-day creation (not modification), blank directory sizes, --full and --help')
