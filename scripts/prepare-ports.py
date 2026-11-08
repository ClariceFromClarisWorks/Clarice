"""Keep upstream source pristine; stage the documented cooperative poll hooks."""
from pathlib import Path
root=Path(__file__).resolve().parents[1]
for name in ('regcomp.c','regexec.c'):
 text=(root/'src/vendor/musl-regex'/name).read_text()
 # The central parser/executor loops poll even without stream I/O.
 text=text.replace('for (;;) {', 'for (;;) { port_poll();')
 text=text.replace('while (1)\n', 'while (1)\n')
 import re
 text=re.sub(r'(while \(1\)\s*\{)',r'\1 port_poll();',text)
 text=re.sub(r'(while \(status == REG_OK && tre_stack_num_objects\(stack\) > bottom\)\s*\{)',r'\1 port_poll();',text)
 (root/'work/build/ports'/name).write_text(text)

text=(root/'src/vendor/sbase/grep.c').read_text()
old='if (regexec(&pnode->preg, buf, 0, NULL, 0) == 0) {'
new='int rc = regexec(&pnode->preg, buf, 0, NULL, 0);\n                if (rc != 0 && rc != REG_NOMATCH) {\n                    weprintf("regex execution failed (code %d)\\n", rc);\n                    return Error;\n                }\n                if (rc == 0) {'
assert old in text
text=text.replace(old,new)
(root/'work/build/ports/grep.c').write_text(text)

text=(root/'src/vendor/sbase/sed.c').read_text()
old='for (pc = prog; !gflags.halt; pc++)\n\t\tpc->fninfo->fn(pc);'
assert old in text
text=text.replace(old,'for (pc = prog; !gflags.halt; pc++) { port_poll(); pc->fninfo->fn(pc); }')
(root/'work/build/ports/sed.c').write_text(text)

# Diagnostic output itself may alter errno in pinned newlib. Preserve the cause.
text=(root/'src/vendor/sbase/libutil/eprintf.c').read_text()
text=text.replace('#include "../util.h"', '#include "util.h"')
text='#include <errno.h>\n'+text
text=text.replace('xvprintf(const char *fmt, va_list ap)\n{', 'xvprintf(const char *fmt, va_list ap)\n{\n\tint saved_errno = errno;')
text=text.replace('\t\tperror(NULL);', '\t\terrno = saved_errno;\n\t\tperror(NULL);')
(root/'work/build/ports/eprintf.c').write_text(text)
