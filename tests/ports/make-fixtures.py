#!/usr/bin/env python3
"""Prepare disposable guest inputs; install only with the emulator stopped."""
from pathlib import Path
out=Path('work/grep-fixtures'); out.mkdir(parents=True,exist_ok=True)
(out/'input').write_bytes(b'alpha\nBeta\nalphabet\n123\nword word\nlast\n')
(out/'long').write_bytes(b'a'*17000)
(out/'many').write_bytes(b'alpha beta gamma\n'*20000)
(out/'short').write_bytes(b'alpha beta gamma\n'*1000)
for name in ('grep','sed','perf'):
 (out/(name+'.sh')).write_bytes(Path('tests/ports/'+name+'-guest.sh').read_bytes())
print(out)
