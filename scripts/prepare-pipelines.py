"""Pristine sbase sources plus Clarice help and cooperative cancellation hooks."""
from pathlib import Path
import json
root=Path(__file__).resolve().parents[1]
out=root/'work/build/pipelines';out.mkdir(parents=True,exist_ok=True)
help_text={
 'basename':'basename path [suffix]\nPrint the final path component, optionally removing a suffix.\n',
 'dirname':'dirname path\nPrint the parent directory portion.\n',
 'tee':'tee [-a] [file ...]\nCopy stdin to stdout and files; -a appends. -i (Unix signal masking) unsupported.\n',
 'tail':'tail [-f] [-n count|-c count] [file ...]\nLast 10 lines by default; +count starts at that line/byte.\n-f follows one file descriptor and yields until Ctrl+C; truncation restarts it.\n-m UTF-8 counting is deferred because the pinned upstream implementation has bugs.\nLarge retained tails fail at the allocation cap; this is not a disk-backed tail.\n',
 'sort':'sort [-Cbcdfimnru] [-o output] [-t delimiter] [-k key] [file ...]\nSort lines; -n numeric, -r reverse, -u unique, -f ignore case.\n-c/-C check order (C quiet); -k start[,end] selects fields; -b ignores blanks.\n-d dictionary, -i printable only; -m uses the general merge-sort path.\n16 KiB/128-line chunks spill to private /tmp files; 16 merge levels.\n',
 'wc':'wc [-l] [-w] [-c|-m] [file ...]\nCount lines, words, bytes (-c) or UTF-8 characters (-m). Default: lines words bytes.\n',
 'head':'head [-n count] [file ...]\nPrint the first count lines (default 10). Multiple files get headers.\n',
 'cut':'cut -b list [-n] [file ...]\ncut -c list [file ...]\ncut -f list [-d delimiter] [-s] [file ...]\nSelect byte, UTF-8 character or field ranges (1,3-5,8-). Default delimiter: tab.\n-n avoids splitting UTF-8 bytes; -s omits lines without the field delimiter.\n',
 'tr':'tr [-cCds] set1 [set2]\nTranslate UTF-8 characters, -d delete, -s squeeze repeats, -c/-C complement.\nSets accept ranges, escapes and character classes such as [:upper:].\n',
 'uniq':'uniq [-c|-d|-u] [-f fields] [-s chars] [input [output]]\nFilter adjacent repeated lines; -c count, -d duplicates only, -u unique only.\n-f/-s skip fields/characters when comparing. Sort first for nonadjacent repeats.\n'
}
for name,help in help_text.items():
 s=(root/f'src/vendor/sbase/{name}.c').read_text()
 marker='main(int argc, char *argv[])\n{'
 assert marker in s
 help+='sbase port for Clarice. Status: 0 success; nonzero error; 130 interrupted.\n96 KiB allocation cap; line-based tools limit each line to 16 KiB.\nUTF-8 processing does not convert the terminal font from MacRoman.\nGNU-only long options are not implemented; --help is a Clarice addition.\n'
 s=s.replace(marker,marker+'\n\tif (argc == 2 && !strcmp(argv[1], "--help")) { puts('+json.dumps(help)+'); return 0; }')
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
