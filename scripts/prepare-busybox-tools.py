"""Stage genuine selected BusyBox sources; configuration and boundary changes only."""
from pathlib import Path
import re,shutil
r=Path(__file__).resolve().parents[1];u=r/'work/busybox/busybox-1.37.0';w=r/'work/busybox-tools';w.mkdir(exist_ok=True)
# All omitted configuration switches are explicit off, as in BusyBox autoconf.
names=set()
for p in u.rglob('*.c'):
 names.update(re.findall(r'\b(?:ENABLE_|IF_NOT_|IF_)([A-Z][A-Z0-9_]*)',p.read_text(errors='replace')))
on={'LONG_OPTS','FEATURE_TR_CLASSES','FEATURE_TR_EQUIV','FEATURE_SORT_BIG','DESKTOP','FEATURE_GREP_CONTEXT','FEATURE_GREP_EGREP_ALIAS','FEATURE_GREP_FGREP_ALIAS','CKSUM','FEATURE_TEE_USE_BLOCK_IO','FEATURE_FANCY_HEAD','FEATURE_FANCY_TAIL'}
with (w/'config.h').open('w') as f:
 f.write('#define CONFIG_FEATURE_COPYBUF_KB 4\n')
 f.write('/* Generated explicit BusyBox selected-source configuration. */\n')
 for n in sorted(names):
  v=int(n in on);f.write(f'#define ENABLE_{n} {v}\n#define IF_{n}(...) '+('__VA_ARGS__' if v else '')+'\n#define IF_NOT_'+n+'(...) '+('' if v else '__VA_ARGS__')+'\n')
# unicode.h uses disabled implementation without pulling Unicode tables.
shutil.copy2(u/'include/unicode.h',w/'unicode.h')
for name in ['wc','tr','sort','cat','cut','head','tail','uniq','tee','cksum','basename','dirname']:
 shutil.copy2(u/f'coreutils/{name}.c',w/f'{name}.c')
for name in ['cmp','sed']:
 shutil.copy2(u/f'editors/{name}.c',w/f'{name}.c')
shutil.copy2(u/'findutils/grep.c',w/'grep.c')
shutil.copy2(u/'include/xregex.h',w/'xregex.h')
# Keep kernel-dependent optional paths out of these experiment packages. Reject
# them explicitly before processing input; never emulate successful operations.
p=w/'sort.c';s=p.read_text();start=s.index('\t\tcase FLAG_M: {');end=s.index('\n\t\t/* Full floating',start);s=s[:start]+'\t\tcase FLAG_M: bb_simple_error_msg_and_die("-M unavailable in this port");\n'+s[end:]
s=s.replace('xmove_fd(xopen(str_o, O_WRONLY|O_CREAT|O_TRUNC), STDOUT_FILENO);','bb_simple_error_msg_and_die("-o unavailable; use shell redirection to a separate file");')
s=s.replace('\t/* Parse command line options */','\t/* Parse command line options */')
needle='\t\t\t&str_ignored, &str_ignored, &str_o, &lst_k, &str_t\n\t);'
assert needle in s;s=s.replace(needle,needle+'\n if(opts & (FLAG_o|FLAG_M|FLAG_V|FLAG_m|FLAG_S|FLAG_T)) bb_simple_error_msg_and_die("unsupported option: -o/-M/-V/-m/-S/-T");')
s=s.replace('static int compare_keys(const void *xarg, const void *yarg)\n{','static int compare_keys(const void *xarg, const void *yarg)\n{\n port_poll();')
p.write_text(s)
p=w/'sed.c';s=p.read_text();s=s.replace('if (G.outname) unlink(G.outname);','/* In-place editing excluded; no temporary output is created. */')
s=s.replace('die_func = cleanup_outname;','bb_simple_error_msg_and_die("-i unavailable; use shell redirection to a separate file");')
start=s.index('\n\t\t\t/* -i: process each FILE separately: */');end=s.index('\n\t\t}\n\t\t/* Here,',start);s=s[:start]+s[end:]
s=s.replace('while (1) {','while (1) { port_poll();')
p.write_text(s)
p=w/'grep.c';s=p.read_text();start=s.index('static int FAST_FUNC file_action_grep(');end=s.index('int grep_main(',start);s=s[:start]+s[end:]
start=s.index('\t\t\tif (option_mask32 & (OPT_r|OPT_R)) {');end=s.index('\n\t\t\t/* else:',start);s=s[:start]+s[end:]
s=s.replace('\t/* If no files were specified,', '\tif(option_mask32 & (OPT_r|OPT_R)) bb_simple_error_msg_and_die("recursive grep unavailable in this port");\n\t/* If no files were specified,')
p.write_text(s)

p=w/'tee.c';s=p.read_text().replace('signal(SIGINT, SIG_IGN);','bb_simple_error_msg_and_die("-i unavailable; cancellation remains enabled");');p.write_text(s)

p=w/'uniq.c';s=p.read_text().replace('const char *input_filename;','const char *input_filename;\n FILE *input=stdin;');s=s.replace('close(STDIN_FILENO); /* == 0 */\n\t\t\txopen(input_filename, O_RDONLY); /* fd will be 0 */','input=xfopen_stdin(input_filename);');s=s.replace('xmove_fd(xopen(output, O_WRONLY | O_CREAT | O_TRUNC), STDOUT_FILENO);','bb_simple_error_msg_and_die("output operand unavailable; use shell redirection");');s=s.replace('xmalloc_fgetline(stdin)','xmalloc_fgetline(input)');p.write_text(s)
p=w/'tail.c';s=p.read_text();start=s.index('\tif (FOLLOW) while (1) {');end=s.index('\n\tif (ENABLE_FEATURE_CLEAN_UP)',start);s=s[:start]+s[end:];s=s.replace('#define FOLLOW (opt & 0x1)','#define FOLLOW (opt & 0x1)\n if(opt & (1|16|64)) bb_simple_error_msg_and_die("follow mode unavailable in this port");');p.write_text(s)
# Exact help describes this selected configuration, not all BusyBox features.
import json
helps=json.loads((r/'src/ports/busybox-tools/help.json').read_text())
(w/'help.h').write_text('\n'.join('#define HELP_'+k+' '+json.dumps('Usage: bb'+k+' '+v+'\nData forks only; stdin may be selected with - where supported.\nStatus: 0 success, nonzero failure (see above), 130 cancellation.\nExperimental BusyBox 1.37.0 / Clarice; not a replacement installation.\n') for k,v in helps.items())+'\n')
# Never turn an input error into a successful partial result.
p=w/'uniq.c';s=p.read_text().replace('die_if_ferror(stdin, input_filename);','die_if_ferror(input, input_filename ? input_filename : "stdin");');p.write_text(s)

p=w/'sed.c';s=p.read_text().replace('for (sed_cmd = G.sed_cmd_head; sed_cmd; sed_cmd = sed_cmd->next) {','for (sed_cmd = G.sed_cmd_head; sed_cmd; sed_cmd = sed_cmd->next) { port_poll();');p.write_text(s)
# full_read is an unmodified upstream excerpt; safe_read uses Clarice's existing
# cancellation-aware descriptor service instead of defining a second read layer.
s=(u/'libbb/read.c').read_text();(w/'full_read.c').write_text(s[:s.index('ssize_t FAST_FUNC safe_read')]+s[s.index('/*\n * Read all'):s.index('ssize_t FAST_FUNC read_close')])
p=w/'cksum.c';s=p.read_text().replace('close(fd);','if (fd != STDIN_FILENO && close(fd)) bb_simple_perror_msg_and_die("close");');p.write_text(s)
