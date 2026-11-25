#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
test_tmp=$(mktemp -d)
trap 'rm -rf "$test_tmp"' EXIT HUP INT TERM
cc -Itests/installer-b/include -Wno-unknown-escape-sequence -fsanitize=address,undefined tests/installer-a/dialog.c -o "$test_tmp/dialog"
"$test_tmp/dialog"
python3 - <<'PY'
from pathlib import Path
app=Path('src/app.c').read_text().split('int main(void)',1)[1]
assert app.index('return installer_main(launch)') < app.index('shell_init()')
assert app.index('return installer_main(launch)') < app.index('NewWindow(')
ui=Path('src/installer-ui.inc').read_text()
assert ui.index('if(installer_choice==2)') < ui.index('shell_install_init(installer_core,installer_data);')
shell=Path('src/shell.c').read_text().split('int shell_install_init(const char *core,const char *data)',1)[1].split('int shell_init(void)',1)[0]
assert 'namespace_install()' in shell
assert all(x not in shell for x in ('migrate_reports','env_set','history_ref'))
print('PASS installer decision precedes terminal/shell/namespace initialization; narrow installer init excludes history and profile execution')
PY
