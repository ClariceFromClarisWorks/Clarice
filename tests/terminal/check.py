from pathlib import Path
import subprocess
root=Path(__file__).resolve().parents[2]
source=(root/'src/app.c').read_text()
a=source.index('void term_write(');b=source.index('\nvoid term_puts',a)
parts=[(root/'tests/terminal'/n).read_text() for n in ('harness-head.inc','reference.inc')]
parts+=[source[a:b],(root/'tests/terminal/harness-tail.inc').read_text()]
out=root/'work/ls-fast';out.mkdir(parents=True,exist_ok=True)
(out/'terminal-test.c').write_text('\n'.join(parts))
subprocess.run(['cc','-std=c99','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-fsanitize=address,undefined',str(out/'terminal-test.c'),'-o',str(out/'terminal-test')],check=True)
subprocess.run([str(out/'terminal-test')],check=True)
