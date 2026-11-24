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
  help="awk [-F SEP] [-v NAME=VALUE] [-f FILE] [program] [file ...]\nProcess records with One True Awk. Without -f, the first operand is the program. Input defaults to stdin; - also means stdin.\nOptions:\n  -F SEP\tSet the field separator FS.\n  -v NAME=VALUE\tAssign before BEGIN.\n  -f FILE\tRead a program file. Repeat to combine files.\n  --csv\tRead CSV records; do not set FS.\n  -safe\tDisable file output and unsafe features.\n  -d[N]\tEnable debugging.\n  -version, --version\tPrint the upstream version.\nUse -- to end options. NAME=VALUE file operands assign at that input position. ENVIRON contains exported variables.\nRecords, fields and arrays grow with memory. Call depth is 64. File I/O uses data forks.\nNo system(), command pipelines, child processes or GNU option parity.\nExample: awk -F: '{print $1}' file\nStatus: exit expression (default 0), nonzero error, 130 interrupted.\n"
  import json
  s=s.replace('WARNING("unknown option %s ignored", argv[1]);', 'FATAL("unknown option %s; use --help", argv[1]);')
  s=s.replace('if (strcmp(argv[1], "-safe") == 0)\n\t\t\t\tsafe = true;', 'if (strcmp(argv[1], "-safe") == 0)\n\t\t\t\tsafe = true;\n\t\t\telse FATAL("unknown option %s; use --help", argv[1]);')
  usage,body=help.split('\n',1)
  help='Usage:\n  '+usage+'\n'+body.replace('Example: ', 'Example:\n  ')
  s='#include "../../../src/help-output.h"\n'+s
  s=s.replace('int main(int argc, char *argv[])\n{','int main(int argc, char *argv[])\n{\n if(argc==2&&!strcmp(argv[1],"--help"))return help_output(1, '+json.dumps(help)+');')
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
