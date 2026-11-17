#!/usr/bin/env python3
"""Minimal, idempotent adapter to pinned nano 9.2 write_file. No global open shim."""
from pathlib import Path
p=Path(__file__).resolve().parents[1]/'work/nano/cross/nano-9.2/src/files.c'
s=p.read_text()
if 'CLARICE_SAFE_SAVE' in s: raise SystemExit(0)
changes=[
('bool write_file(const char *name, FILE *thefile, writing_type method, bool annotate)\n{','bool write_file(const char *name, FILE *thefile, writing_type method, bool annotate)\n{\n /* CLARICE_SAFE_SAVE: explicit staged overwrite, ordinary POSIX open unchanged. */\n void *replacement = NULL;\n char save_target[520];\n const char *write_name = NULL;'),
('descriptor = open(realname, O_WRONLY | O_CREAT | ((method == APPEND) ?', '''write_name = realname;
        if (normal) {
            if (method != OVERWRITE) { errno = EINVAL; goto cleanup_and_exit; }
            replacement = cli_replace_begin(realname, save_target, sizeof save_target);
            if (!replacement) { statusline(ALERT, _("Error writing %s: %s"), realname, strerror(errno)); goto cleanup_and_exit; }
            write_name = save_target;
        }
        descriptor = open(write_name, O_WRONLY | O_CREAT | ((method == APPEND) ?'''),
('\tif (fclose(thefile) == EOF) {','\tif (port_nano_finish_save(thefile, &replacement) == EOF) {'),
('  cleanup_and_exit:\n','  cleanup_and_exit:\n        if (replacement) { cli_replace_abort(replacement); replacement = NULL; }\n')]
for old,new in changes:
 assert s.count(old)==1,(old,s.count(old))
 s=s.replace(old,new)
p.write_text(s)
