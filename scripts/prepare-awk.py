"""Preserve upstream awk; stage explicit System 6 adaptations separately."""
from pathlib import Path
root=Path(__file__).resolve().parents[1]
out=root/'work/build/awk';out.mkdir(parents=True,exist_ok=True)
for p in (root/'src/vendor/awk').iterdir():
 if p.suffix not in ('.c','.h'):continue
 s=p.read_text()
 if p.name=='awk.h':
  s=s.replace('#define\tRECSIZE\t(8 * 1024)', '#define\tRECSIZE\t1024')
 if p.name=='b.c':
  s=s.replace('state + 10;', 'state + 3;').replace('calloc(NCHARS, sizeof(gtte))', 'calloc(8, sizeof(gtte))').replace('allocated = NCHARS;', 'allocated = 8;')
 if p.name=='main.c':
  help="awk [-F separator] [-v name=value] [-f programfile] [program] [file ...]\nOne True Awk: process records using patterns, actions, arrays/functions.\n-F sets FS; -v assigns before BEGIN; repeat -f to combine program files.\nWithout -f, first operand is the program. Files default to stdin; - is\nstdin. name=value file operands assign variables at that input position.\n-- ends options; --csv reads CSV records (do not also set FS).\n-safe disables file output and unsafe features; -d[N] enables debugging.\n-version/--version prints the upstream version. No GNU option parity.\nExported variables appear in ENVIRON; private shell variables do not.\nRecords, fields and arrays grow with application memory; call depth 64.\nNo system(), command pipes or child processes. File I/O uses data forks.\nExample: awk -F: '{print $1}' file\nStatus: exit expression (default 0), nonzero error, 130 interrupted.\n"
  import json
  s=s.replace('WARNING("unknown option %s ignored", argv[1]);', 'FATAL("unknown option %s; use --help", argv[1]);')
  s=s.replace('if (strcmp(argv[1], "-safe") == 0)\n\t\t\t\tsafe = true;', 'if (strcmp(argv[1], "-safe") == 0)\n\t\t\t\tsafe = true;\n\t\t\telse FATAL("unknown option %s; use --help", argv[1]);')
  s=s.replace('int main(int argc, char *argv[])\n{','int main(int argc, char *argv[])\n{\n if(argc==2&&!strcmp(argv[1],"--help")){puts('+json.dumps(help)+');return 0;}')
  s=s.replace('setlocale(LC_CTYPE, "");','/* The port uses the C locale. */').replace('setlocale(LC_NUMERIC, "C");','').replace('awk_mb_cur_max = MB_CUR_MAX;','awk_mb_cur_max = 1;')
  s=s.replace('(void)signal(SIGFPE, fpecatch);','/* Software floating point: math errors use errno. */')
 if p.name=='run.c':
  s=s.replace('snprintf(p, BUFSZ(p), fmt, (intmax_t) getfval(x))','port_awk_integer(p, BUFSZ(p), fmt, (uintmax_t)(intmax_t)getfval(x))').replace('snprintf(p, BUFSZ(p), fmt, (uintmax_t) getfval(x))','port_awk_integer(p, BUFSZ(p), fmt, (uintmax_t)getfval(x))')
  s=s.replace('if (minlen > *psiz) {', 'if (minlen < 0 || quantum < 0 || (quantum && minlen > INT_MAX - quantum)) FATAL("Clarice: text buffer size overflow");\n\tif (minlen > *psiz) {')
  s=s.replace('Cell *execute(Node *u)\t/* execute a node of the parse tree */','static Cell *execute_impl(Node *u);\nCell *execute(Node *u) {\n static unsigned depth;\n port_poll();\n if (++depth > 64) FATAL("Clarice: expression/call depth exceeds 64");\n Cell *result=execute_impl(u); --depth; return result;\n}\nstatic Cell *execute_impl(Node *u)')
  s=s.replace('(void) fcntl(fileno(fp), F_SETFD, FD_CLOEXEC);','(void) 0; /* No child process inherits file descriptors. */')
  s=s.replace('for (a = u; ; a = a->nnext) {','for (a = u; ; a = a->nnext) { port_poll();')
 if p.name=='math.c':
  s=s.replace('feclearexcept(FE_ALL_EXCEPT);','(void)0;').replace('fetestexcept(FE_INVALID)','0').replace('fetestexcept(FE_DIVBYZERO | FE_OVERFLOW |\n\t    FE_UNDERFLOW)','0')
 if p.name in ('lex.c','b.c','lib.c'):
  # Poll loop-heavy parsing/matching as well as execution and stream refills.
  s=s.replace('for (;;) {','for (;;) { port_poll();').replace('while (1) {','while (1) { port_poll();')
 if p.name=='lex.c':s=s.replace('int input(void)\t/* get next lexical input character */\n{','int input(void)\t/* get next lexical input character */\n{\n port_poll();')
 (out/p.name).write_text(s)
