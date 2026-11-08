from pathlib import Path
p=Path(__file__).resolve().parents[2]/'outputs/file-tools'
read=lambda n:(p/(n+'.txt')).read_text()
assert read('dots').splitlines()==['.','..']
assert read('mounts').splitlines()==['Sys6Unix/']
assert read('empty')==''
assert read('invalid-status').strip()=='2'
assert read('missing-status').strip()=='1'
assert read('thousand').splitlines()==['.','..']+[f'file{i:04d}.txt' for i in range(1000)]
assert 'KiB' in read('forks') and 'APPL CLRC' in read('forks')
assert '/mnt/Sys6Unix' in read('df') and 'MiB' in read('dfh')
assert read('multiple').splitlines()==['/bin:', '.', '..', '', '/usr/bin:', '.', '..']
print('Guest output assertions PASS: dots, mounts, empty directory, errors, 1000 distinct rows, forks, df, multiple operands.')
