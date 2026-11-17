from pathlib import Path
r=Path(__file__).resolve().parents[2];g=r/'work/shell-posix2/guest';g.mkdir(exist_ok=True)
lines=['set -e','hmount /src/work/parser-safety/audit.hda']
for i in range(54):
 for prefix in ['o','e','s']:lines.append(f'hcopy -r :posix:{prefix}{i} /src/work/shell-posix2/guest/{prefix}{i}')
for name in ['env.out','env.err','env.status','memory']:lines.append(f'hcopy -r :posix:{name} /src/work/shell-posix2/guest/{name}')
for i in range(41):
 for prefix in ['o','r','s']:lines.append(f'hcopy -r :elastic:{prefix}{i} /src/work/shell-posix2/guest/elastic-{prefix}{i}')
for i in range(8):
 for prefix in ['p','pe']:lines.append(f'hcopy -r :elastic:{prefix}{i} /src/work/shell-posix2/guest/elastic-{prefix}{i}')
for p in (r/'tests/shell/scenarios').glob('*.sh'):lines.append(f'hcopy -r :elastic:{p.stem}.out /src/work/shell-posix2/guest/elastic-{p.stem}.out')
for name in ['cancel-status','cancel-out','cancel-err']:lines.append(f'hcopy -r :elastic:{name} /src/work/shell-posix2/guest/{name}')
lines+=['humount'];(r/'work/shell-posix2/export.sh').write_text('\n'.join(lines)+'\n')
