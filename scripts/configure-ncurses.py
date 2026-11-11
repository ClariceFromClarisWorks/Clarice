#!/usr/bin/env python3
"""Configure the services supplied by the Clarice virtual terminal adapter."""
from pathlib import Path
root=Path(__file__).resolve().parents[1]
build=root/'work/ncurses/build'
p=build/'include/ncurses_cfg.h'
services={'USE_SIGWINCH':0,'HAVE_TERMIOS_H':1,'HAVE_TCGETATTR':1,'HAVE_TCSETATTR':1,'HAVE_SYS_IOCTL_H':1,'HAVE_SELECT':1,'HAVE_SIZECHANGE':1,'HAVE_TSEARCH':0,'HAVE_LANGINFO_CODESET':0,'HAVE_LOCALE_H':0}
with p.open('a') as f:
    f.write('\n/* Clarice virtual-terminal platform configuration. */\n')
    for key,value in services.items(): f.write(f'#undef {key}\n#define {key} {value}\n')
# These public headers are generated from configure substitutions, rather than
# ncurses_cfg.h. Keep their generator consistent with the adapter services.
for name in ('term.h','MKterm.h.awk'):
    p=build/'include'/name
    if not p.exists(): continue
    s=p.read_text()
    for old in ('#if 0 && 0','#if 0 && 1','#if 1 && 0'):
        s=s.replace(old,'#if 1 && 1')
    p.write_text(s)
