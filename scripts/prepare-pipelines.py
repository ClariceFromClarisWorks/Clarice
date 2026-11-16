"""Pristine sbase sources plus Clarice help and cooperative cancellation hooks."""
from pathlib import Path
import json, textwrap
root=Path(__file__).resolve().parents[1]
out=root/'work/build/pipelines';out.mkdir(parents=True,exist_ok=True)
help_text={
 'dd':'dd [if=file] [of=file] [bs=N|ibs=N obs=N] [count=N] [skip=N] [seek=N]\nCopy binary data forks; stdin/stdout by default (- also means a stream).\nBlocks default to 512 bytes. bs overrides ibs/obs. count counts input\nblocks; skip uses input blocks; seek uses output blocks and needs of=.\nSizes accept positive decimal, k (1024), b (512) and x products.\nOffsets must fit HFS signed 32-bit file positions. count=0 copies none.\nconv=notrunc preserves the existing tail; sync pads short input blocks;\nswab swaps byte pairs; lcase/ucase convert ASCII case. Comma combinations.\nconv=noerror is unsupported: read errors fail rather than looping.\nPartial output remains on error/cancellation. No raw devices, resource\nforks, Finder-metadata copying, authentication or safe editor replacement.\nExample: dd if=/dev/zero of=/tmp/zeros bs=1024 count=8\nRecord counts go to stderr. Status: 0 success, nonzero error, 130 cancel.\n',

 'mktemp':'mktemp [-dqt] [-p directory] [--] [template]\nCreate an empty HFS data-fork file exclusively; -d creates a directory.\nDefault: /tmp/tmp.XXXXXXXXXX; last six X characters are replaced.\n-p or -t selects temporary-directory mode; exported TMPDIR takes\nprecedence over -p, otherwise /tmp. -q suppresses creation errors.\n-t rejects directory separators in template unless -p is also used.\nWithout directory mode, an explicit template is relative to cwd.\nPrint the created path. Unsafe -u name-only mode is not supported.\nNames are collision-checked, not cryptographically unpredictable.\nNo Unix ownership/permission protections on System 6. Caller owns the created file or directory.\nExample: mktemp /tmp/report.XXXXXX\n',

 'sleep':'sleep [--] seconds\nWait cooperatively for a nonnegative whole number of seconds.\nMacintosh ticks (60 per second) measure the wait; Ctrl+C cancels.\nNo fractional, suffixed or multiple duration operands.\nExample: sleep 2\nNo data input or normal output.\n',
 'printenv':'printenv [--] [name ...]\nPrint exported environment values, one per line; all NAME=value\nentries with no operands. Private shell variables are not visible.\nMissing names produce no output and status 1; output failure is 2.\nClarice supplies HOME/PWD defaults; names and values grow with memory.\nExample: printenv PATH TERM\nNo variable assignment, command execution or NUL-delimited option.\n',

 'cmp':'cmp [-l|-s] file1 file2\nCompare data forks byte for byte; - means stdin.\nDefault: report the first difference with byte and line positions.\n-l reports every differing byte (decimal offset, octal byte values).\n-s suppresses difference and file-error diagnostics; cannot combine -l.\nStatus: 0 equal, 1 different, 2 error, 130 interrupted.\nExample: cmp -s original copied\nNo text conversion, resource-fork comparison or GNU skip operands.\n',
 'cksum':'cksum [file ...]\nPrint the portable POSIX CRC checksum and byte count for each data fork.\nNo operands reads stdin; - reads stdin and labels it <stdin>.\nNot a cryptographic hash or authenticity check. Resource forks excluded.\nExample: cksum downloaded.clx\n',
 'basename':'basename path [suffix]\nPrint the final path component, optionally removing a suffix.\n',
 'dirname':'dirname path\nPrint the parent directory portion.\n',
 'tee':'tee [-a] [file ...]\nCopy stdin to stdout and files; -a appends. -i (Unix signal masking) unsupported.\n',
 'tail':'tail [-f] [-n count|-c count] [file ...]\nLast 10 lines by default; +count starts at that line/byte.\n-f follows one file descriptor and yields until Ctrl+C; truncation restarts it.\n-m UTF-8 counting is deferred because the pinned upstream implementation has bugs.\nRetained tails use available application memory; this is not a disk-backed tail.\n',
 'sort':'sort [-Cbcdfimnru] [-o output] [-t delimiter] [-k key] [file ...]\nSort lines; -n numeric, -r reverse, -u unique, -f ignore case.\n-c/-C check order (C quiet); -k start[,end] selects fields; -b ignores blanks.\n-d dictionary, -i printable only; -m uses the general merge-sort path.\n16 KiB/128-line batches spill to private /tmp files; longer records use singleton runs.\n16 merge levels; records grow with available memory.\n',
 'wc':'wc [-l] [-w] [-c|-m] [file ...]\nCount lines, words, bytes (-c) or UTF-8 characters (-m). Default: lines words bytes.\n',
 'head':'head [-count|-n count] [file ...]\nPrint the first count lines (default 10). Multiple files get headers.\n',
 'cut':'cut -b list [-n] [file ...]\ncut -c list [file ...]\ncut -f list [-d delimiter] [-s] [file ...]\nSelect byte, UTF-8 character or field ranges (1,3-5,8-). Default delimiter: tab.\n-n avoids splitting UTF-8 bytes; -s omits lines without the field delimiter.\n',
 'tr':'tr [-cCds] set1 [set2]\nTranslate UTF-8 characters, -d delete, -s squeeze repeats, -c/-C complement.\nSets accept ranges, escapes and character classes such as [:upper:].\n',
 'uniq':'uniq [-c] [-d|-u] [-f fields] [-s chars] [input [output]]\nFilter adjacent repeated lines; -c count, -d duplicates only, -u unique only.\n-f/-s skip fields/characters when comparing. Sort first for nonadjacent repeats.\n-c may combine with -d or -u; both -d and -u select no lines.\n'
}
for name,help in help_text.items():
 s=(root/f'src/vendor/sbase/{name}.c').read_text()
 marker='main(int argc, char *argv[])\n{'
 assert marker in s
 if name not in ('basename','dirname','tr','tee','cmp','sleep','printenv','mktemp'):
  help+='Input defaults to stdin (or use -); output defaults to stdout.\nData forks only; resource forks and Finder metadata are not stream data.\n'
 if name=='sort':help+='-o selects output (may equal an input); -t sets one field separator.\nDefault keys compare the whole line; -C suppresses check diagnostics.\n'
 if name=='tee':help+='Reads stdin; each file operand receives a copy as well as stdout.\n'
 if name=='tr':help+='Reads stdin and writes stdout; no file operands.\n'
 if name!='dd':help+='-- ends option parsing where options are supported.\n'
 help+=('sbase port for Clarice.\n' if name=='cmp' else 'sbase port for Clarice. Status: 0 success; nonzero error; 130 interrupted.\n')
 help+='Invocation allocations are released on completion.\n'
 if name in ('wc','cut','tr','sort','uniq','tail'):help+='UTF-8 processing does not convert the terminal font from MacRoman.\n'
 help+='GNU-only long options are not implemented; --help is a Clarice addition.\n'
 help='\n'.join('\n'.join(textwrap.wrap(line,72,break_long_words=False,break_on_hyphens=False)) for line in help.splitlines())+'\n'
 s=s.replace(marker,marker+'\n\tif (argc == 2 && !strcmp(argv[1], "--help")) { puts('+json.dumps(help)+'); return 0; }')
 if name in ('printenv','mktemp'):
  s='extern char **port_environ; extern char *port_getenv(const char*); extern void port_awk_init(void);\n#define environ port_environ\n#define getenv port_getenv\n'+s
  at=s.index('\n\tchar *var;' if name=='printenv' else '\n\tint dflag',s.index(marker));s=s[:at]+'\n port_awk_init();'+s[at:]
 if name=='mktemp':
  s='extern void port_mktemp_track(const char*),port_mktemp_commit(void); extern int port_mkstemp(char*); extern char *port_mkdtemp(char*); extern char *port_path_dirname(char*); extern int cli_remove(const char*);\n#define mkstemp port_mkstemp\n#define mkdtemp port_mkdtemp\n#define dirname port_path_dirname\n'+s
  s=s.replace('uflag = 1;', 'eprintf("mktemp: unsafe -u is not supported\\n");')
  s=s.replace('path[PATH_MAX], tmp[PATH_MAX]', 'path[512], tmp[512]')
  s=s.replace('\tif (dflag) {', '\tport_mktemp_track(path);\n\tif (dflag) {')
  s=s.replace('efshut(stdout, "<stdout>");', 'if (fshut(stdout, "<stdout>")) return 2; port_mktemp_commit();')
 if name=='dd':
  # Native off_t is signed 32-bit; avoid unsupported intmax printf ABI.
  s=s.replace('"%"PRIdMAX"+%"PRIdMAX" records', '"%ld+%ld records')
  for counter in ('ifull','ipart','ofull','opart'):s=s.replace('(intmax_t)'+counter,'(long)'+counter)
  start=s.index('static size_t\nparsesize(');end=s.index('\nstatic void\nbswap(',start)
  s=s[:start]+(root/'src/ports/dd-size.inc').read_text()+s[end:]
  s=s.replace('estrtonum(val, 0, LLONG_MAX)', 'estrtonum(val, 0, 2147483647L)')
  s=s.replace('conv |= NOERROR;', 'eprintf("dd: conv=noerror is not supported\\n");')
  s=s.replace('seek * ibs', 'seek * obs')
  # HFS must not expose an uninitialised gap for notrunc seeks beyond EOF.
  s=s.replace('if (!(conv & NOTRUNC) && ftruncate(ofd, seek * obs) != 0)', 'if ((!(conv & NOTRUNC) || lseek(ofd,0,SEEK_END) < seek * obs) && ftruncate(ofd, seek * obs) != 0)')

  old='\tlen = MAX(ibs, obs) + ibs;\n\tbuf = emalloc(len);'
  assert old in s;s=s.replace(old,'')
  marker='\tif (strcmp(iname, "-") != 0) {'
  setup='\tif (MAX(ibs,obs)>2147483647UL-ibs || (skip && (unsigned long)skip>2147483647UL/ibs) || (seek && (unsigned long)seek>2147483647UL/obs)) eprintf("dd: size/offset exceeds HFS range\\n");\n\tlen = MAX(ibs,obs)+ibs; buf=emalloc(len);\n'
  s=s.replace(marker,setup+marker,1)
  s=s.replace('while (!eof) {','while (!eof) {\n port_poll();').replace('while (skip--) {','while (skip--) {\n port_poll();')
 if name=='cmp':s=s.replace('for (n = 1; ; n++) {','for (n = 1; ; n++) {\n if (!(n & 255)) port_poll();')
 if name=='cksum':s=s.replace('while ((n = read(fd, buf, sizeof(buf))) > 0) {','while ((n = read(fd, buf, sizeof(buf))) > 0) {\n port_poll();')
 if name=='tr':s=s.replace('read:\n','read:\n\tport_poll();\n')
 if name=='tee':
  old='if (iflag && signal(SIGINT, SIG_IGN) == SIG_ERR)\n\t\teprintf("signal:");'
  assert old in s;s=s.replace(old,'if (iflag) enprintf(2, "tee: -i signal masking is not supported\\n");')
 if name=='tail':
  s=s.replace("case 'm':\n\tcase 'n':", "case 'm': if (ARGC() == 'm') enprintf(2, \"tail: -m is not implemented on Clarice\\n\");\n\tcase 'n':")
  start=s.index('for (p = buf + len - 2, left = count;');end=s.index('\n\t\t\tbreak;',start)
  s=s[:start]+"p = buf; left = count;\n            for (size_t at = len > 1 ? len - 1 : 0; at > 0;) {\n                if (buf[--at] == '\\n' && !--left) {p = buf + at + 1; break;}\n            }"+s[end:]
  s=s.replace('lseek(fd, SEEK_SET, 0)', 'lseek(fd, 0, SEEK_SET)')
 if name=='sort':
  s=s.replace('slinecmp(struct line *a, struct line *b)\n{','slinecmp(struct line *a, struct line *b)\n{\n port_poll();')
  s=s.replace('static int\ncheck(FILE *fp', '#include "sort-spill.inc"\n\nstatic int\ncheck(FILE *fp')
  s=s.replace('getlines(stdin, &linebuf);','spill_add(stdin);').replace('getlines(fp, &linebuf);','spill_add(fp);')
  start=s.index('\t\tqsort(linebuf.lines,');end=s.index('\n\t}\n\n\tif (fshut',start)
  s=s[:start]+'\t\tspill_finish(ofp);\n\t\tif (ofp != stdout && fclose(ofp)) ret = 2;'+s[end:]
  s=s.replace('if ((len = getline(&prev.data, &prevsize, fp)) < 0)\n\t\t\teprintf("getline:");','if ((len = getline(&prev.data, &prevsize, fp)) < 0) { if (ferror(fp)) enprintf(2,"getline:"); free(prev.data); prev.data=NULL; prevsize=0; return 0; }')
  s=s.replace('\n\treturn 0;\n}\n\nstatic int\nparse_flags','\n\tif (ferror(fp)) enprintf(2,"getline:");\n\treturn 0;\n}\n\nstatic int\nparse_flags')
 if name in ('basename','dirname'):
  s='char *port_path_basename(char *); char *port_path_dirname(char *);\n'+s
  s=s.replace('p = basename(argv[0]);','p = port_path_basename(argv[0]);').replace('puts(dirname(argv[0]));','puts(port_path_dirname(argv[0]));')
 (out/f'{name}.c').write_text(s)
