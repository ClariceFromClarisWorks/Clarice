"""Stage pinned BusyBox additions; explicit, reviewable Macintosh boundaries."""
from pathlib import Path
import re, shutil, json
r=Path(__file__).resolve().parents[1]; u=r/'work/busybox/busybox-1.37.0'; w=r/'work/busybox-additions'; w.mkdir(exist_ok=True)
names=set()
for p in u.rglob('*.c'):
 names.update(re.findall(r'\b(?:ENABLE_|IF_NOT_|IF_)([A-Z][A-Z0-9_]*)',p.read_text(errors='replace')))
on={'LONG_OPTS','BASE64','XXD','FEATURE_DIFF_LONG_OPTIONS','FEATURE_CLEAN_UP','FEATURE_GZIP_LEVELS','FEATURE_GZIP_LONG_OPTIONS','TAR'}
with (w/'config.h').open('w') as f:
 f.write('#define CONFIG_FEATURE_COPYBUF_KB 4\n#define CONFIG_GZIP_FAST 0\n')
 for n in sorted(names):
  v=int(n in on); f.write(f'#define ENABLE_{n} {v}\n#define IF_{n}(...) '+('__VA_ARGS__' if v else '')+'\n#define IF_NOT_'+n+'(...) '+('' if v else '__VA_ARGS__')+'\n')
for name,src in {'diff':'editors/diff.c','base64':'coreutils/uudecode.c','xxd':'util-linux/hexdump_xxd.c','dump':'libbb/dump.c'}.items():
 shutil.copy2(u/src,w/(name+'.c'))
shutil.copy2(u/'include/dump.h',w/'dump.h')
# FILE streams belong to Clarice, not newlib's process descriptor table.
p=w/'base64.c';s=p.read_text().replace('baseNUM_main','base64_main').replace('src_fd = fileno(src_stream);','/* Read through the invocation-owned FILE. */').replace('full_read(src_fd, src_buf, SRC_BUF_SIZE)','fread(src_buf, 1, SRC_BUF_SIZE, src_stream)').replace('if ((ssize_t)size < 0)','if (ferror(src_stream))').replace('\n\tfflush_stdout_and_exit_SUCCESS();','\n\tdie_if_ferror(src_stream, "input");\n\tfflush_stdout_and_exit_SUCCESS();');p.write_text(s)
p=w/'diff.c';s=p.read_text();a=s.index('\t\tint fd = STDIN_FILENO;',s.index('static int diffreg'));b=s.index('\n\t}\n\n\tsetup_common_bufsiz',a);s=s[:a]+ '\t\tfp[i] = addition_diff_input(file[i]);'+s[b:];s=s.replace('xfunc_error_retval = 1;','xfunc_error_retval = 2;').replace('\targv += optind;', '\tif (option_mask32 & (FLAG(r)|FLAG(N)|FLAG(S))) bb_error_msg_and_die("directory options -r/-N/-S unavailable");\n\targv += optind;');s=s.replace('fseeko(ft->ft_fp, pos, SEEK_SET);','if (fseeko(ft->ft_fp, pos, SEEK_SET)) bb_simple_perror_msg_and_die("seek");').replace('fseeko(ft[i].ft_fp, 0, SEEK_SET);','if (fseeko(ft[i].ft_fp, 0, SEEK_SET)) bb_simple_perror_msg_and_die("seek");');p.write_text(s)
p=w/'dump.c';s=p.read_text();s=s.replace('#include "dump.h"','#include "dump.h"\nstatic FILE *dump_input;');s=re.sub(r'\bstdin\b','dump_input',s);s=s.replace('xfstat(STDIN_FILENO, &sbuf, fname);','memset(&sbuf, 0, sizeof(sbuf)); /* Seek this FILE, never host fd 0. */');s=s.replace('if (!freopen(fname, "r", dump_input)) {','if (dump_input && dump_input != port_standard(0)) fclose(dump_input);\n\t\t\t\tdump_input = fopen(fname, "r");\n\t\t\t\tif (!dump_input) {');s=s.replace('static NOINLINE int next(priv_dumper_t *dumper)\n{','static NOINLINE int next(priv_dumper_t *dumper)\n{\n if (!dump_input) dump_input = port_standard(0);');p.write_text(s)
p=w/'xxd.c';s=p.read_text().replace('\t//fclose(fp);','\tdie_if_ferror(fp, "input");\n\tfclose_if_not_stdin(fp);').replace('\targv += optind;', '\tif (opt & OPT_a) bb_error_msg_and_die("-a unavailable in BusyBox xxd");\n\targv += optind;');p.write_text(s)
helps=json.loads((r/'src/ports/busybox-additions/help.json').read_text());(w/'help.h').write_text('\n'.join('#define HELP_'+k+' '+json.dumps('Usage: '+k+' '+v+'\nData forks only. BusyBox 1.37.0 / Clarice.\n') for k,v in helps.items())+'\n')
# patch: retain BusyBox's hunk matching; explicit existing target, one transaction.
# No name from an untrusted patch can choose a destination. Commit only at EOF.
s=(u/'editors/patch.c').read_text().replace('#include "libbb.h"','#include "libbb.h"\n#include "../safe-save.h"\nstatic FILE *patch_input, *target_input, *target_output;\nstatic void *replacement;\nstatic int sections, applied;')
a=s.index('static void finish_oldfile(void)');b=s.index('static void fail_hunk(void)',a)
s=s[:a]+'''static void finish_oldfile(void)
{
 if (!replacement) return;
 char buf[4096]; size_t n;
 while ((n=fread(buf,1,sizeof(buf),target_input)))
  if (fwrite(buf,1,n,target_output)!=n) bb_simple_perror_msg_and_die("write");
 die_if_ferror(target_input,"input");
 if (fclose(target_input) || fclose(target_output)) bb_simple_perror_msg_and_die("close");
 target_input=target_output=NULL;
 void *token=replacement; replacement=NULL;
 if (option_mask32 & FLAG_dry_run) cli_replace_abort(token);
 else if (cli_replace_commit(token)) bb_simple_perror_msg_and_die("replace");
 TT.tempname=NULL; TT.filein=TT.fileout=-1;
}

'''+s[b:]
a=s.index('\t// Abort the copy');b=s.index('\n\tTT.state = 0;',a)
s=s[:a]+'''\tif(target_input) fclose(target_input);
 if(target_output) fclose(target_output);
 target_input=target_output=NULL;
 if(replacement) cli_replace_abort(replacement);
 replacement=NULL; TT.tempname=NULL; TT.filein=TT.fileout=-1;
 bb_simple_error_msg_and_die("patch rejected; original preserved");
'''+s[b:]
s=s.replace('char *data = xmalloc_reads(TT.filein, NULL);','char *data = xmalloc_fgetline(target_input);\n if(!data) die_if_ferror(target_input,"target");')
s=s.replace('fdprintf(TT.state == 2 ? 2 : TT.fileout,','fprintf(TT.state == 2 ? stderr : target_output,')
s=s.replace('TT.prefix = (opts & FLAG_PATHLEN) ? xatoi(opt_p) : 0;', 'TT.prefix = (opts & FLAG_PATHLEN) ? xatol(opt_p) : 0;')
a=s.index('\tif (opts & FLAG_INPUT) {');b=s.index('\n\t// Loop through the lines',a)
s=s[:a]+''' if (!argv[0] || (argv[1] && argv[2])) bb_show_usage();
 if (opts & (FLAG_PATHLEN|FLAG_RMEMPTY|FLAG_g_unused)) bb_error_msg_and_die("-p/-E/-g unavailable; supply an existing target");
 patch_input=(opts & FLAG_INPUT)?xfopen_stdin(opt_i):argv[1]?xfopen_stdin(argv[1]):stdin;
'''+s[b:]
s=s.replace('xmalloc_fgetline(stdin)','xmalloc_fgetline(patch_input)')
s=s.replace('\t\t\tfinish_oldfile();','\t\t\tif (*patchline==\'+\' && ++sections>1) bb_error_msg_and_die("multiple file patches unavailable; original preserved");')
a=s.index('\t\t\t\tif (empty) {');b=s.index('\n\t\t\t\tfflush_all();',a)
s=s[:a]+'''                if (empty || !oldsum) bb_error_msg_and_die("file creation/deletion patch unavailable");
                printf("patching file %s\\n",name);
                target_input=xfopen(name,"r");
                char stage[512];
                replacement=cli_replace_begin(name,stage,sizeof(stage));
                if(!replacement) bb_simple_perror_msg_and_die("begin replacement");
                target_output=xfopen(stage,"w");
                TT.filein=3; TT.fileout=4; TT.tempname=(char*)"active";
                TT.linenum=0; TT.hunknum=0;
'''+s[b:]
s=s.replace('TT.hunknum++;','TT.hunknum++; applied++;')
s=s.replace('\n\tfinish_oldfile();','\n die_if_ferror(patch_input,"patch input");\n if(TT.current_hunk || state>=2) bb_error_msg_and_die("incomplete patch; original preserved");\n if(!applied) bb_error_msg_and_die("no unified hunks found");\n\tfinish_oldfile();')
# Reject unsupported missing-newline marker instead of silently adding a newline.
s=s.replace('\t\t// Are we assembling a hunk?', '\t\tif (patchline[0]==\'\\\\\') bb_error_msg_and_die("missing-newline patches unavailable; original preserved");\n\t\t// Are we assembling a hunk?')
(w/'patch.c').write_text(s)

# The compressor and inflater are genuine upstream codecs. File policy stays
# at the Macintosh boundary: stdin/stdout, or explicit -c FILE; never unlink input.
s=(u/'include/bb_archive.h').read_text(); a=s.index('typedef struct transformer_state_t'); b=s.index('char* append_ext',a)
(w/'bb_archive.h').write_text('#ifndef ADDITIONS_ARCHIVE_H\n#define ADDITIONS_ARCHIVE_H\n#include "libbb.h"\n#define GZIP_MAGIC 0x1f8b\n'+s[a:b]+"\n#define BBUNPK_OPTSTR \"cfkvq\"\n#define BBUNPK_OPTSTRLEN 5\n#define BBUNPK_OPTSTRMASK 31\nint addition_pack(char **,int (*)(transformer_state_t *));\nextern int addition_input_fd;\n#endif\n")
s=(u/'archival/gzip.c').read_text().replace('#define ifd STDIN_FILENO','#define ifd addition_input_fd').replace('return bbunpack(argv, pack_gzip, append_ext, "gz");','return addition_pack(argv, pack_gzip);')
s=s.replace('option_mask32 &= BBUNPK_OPTSTRMASK;', 'if (option_mask32 & (2|8)) bb_error_msg_and_die("-f/-v unavailable in streaming port");\n\toption_mask32 &= BBUNPK_OPTSTRMASK;')
(w/'gzip.c').write_text(s)
shutil.copy2(u/'archival/libarchive/decompress_gunzip.c',w/'decompress_gunzip.c')
(w/'gunzip.c').write_text('/* gunzip_main is in the shared compression boundary. */\n')
# Tar's upstream command/parser, limited to listing and stdout extraction until
# directory traversal and HFS metadata policy have a reviewed extraction boundary.
shutil.copy2(u/'include/bb_archive.h',w/'bb_archive.h')
with (w/'bb_archive.h').open('a') as f:f.write('\nint addition_pack(char **,int (*)(transformer_state_t *));\nextern int addition_input_fd;\n')
s=(u/'archival/tar.c').read_text()
s=s.replace('if (getuid() != 0)','/* Macintosh has no Unix ownership or permission restoration. */\n\tif (1)')
s=s.replace('\targv += optind;', '\tif ((opt & OPT_EXTRACT) && !(opt & OPT_2STDOUT)) bb_error_msg_and_die("filesystem extraction unavailable; use -xO");\n\tif(opt & (OPT_BASEDIR|OPT_NOPRESERVE_OWNER|OPT_P|OPT_KEEP_OLD|OPT_AUTOCOMPRESS_BY_EXT)) bb_error_msg_and_die("unsupported option in archive reader");\n\targv += optind;')
s=s.replace('header_verbose_list','header_list').replace('data_extract_all','data_extract_to_stdout')
a=s.index('\tif (opt & OPT_2COMMAND) {',s.index('int tar_main'));b=s.index('\n\tif (opt & OPT_KEEP_OLD)',a);s=s[:a]+s[b:]
s=s.replace('if (base_dir)\n\t\txchdir(base_dir);','/* -C rejected above. */')
a=s.index('\tif (opt & OPT_ANY_COMPRESS) {',s.index('int tar_main'));b=s.index('\n\t/* Zero processed headers',a);s=s[:a]+s[b:]
s=s.replace('create_links_from_list(tar_handle->link_placeholders);','/* stdout extraction never creates links. */')
s=s.replace('check_errors_in_children(0);','bb_error_msg_and_die("inline compression unavailable");')
(w/'tar.c').write_text(s)
s=(u/'archival/libarchive/get_header_tar.c').read_text().replace('file_header->device = makedev(major, minor);','(void)major; (void)minor; /* Device numbers are not interpreted by this reader. */')
s=s.replace('overlapping_strcpy(file_header->name, strip_unsafe_prefix(file_header->name));','/* Preserve archive names: this reader never writes filesystem paths. */')
(w/'get_header_tar.c').write_text(s)
p=w/'tar.c';s=p.read_text();a=s.index('\n\t\tif (ENABLE_FEATURE_TAR_AUTODETECT',s.index('int tar_main'));b=s.index('\n\t\t\ttar_handle->src_fd = xopen',a);s=s[:a]+' { '+s[b:];p.write_text(s)
# Stop on malformed hunk headers and any earlier output error before commit.
p=w/'patch.c';s=p.read_text().replace('if (fclose(target_input) || fclose(target_output))','if (ferror(target_output) || fclose(target_input) || fclose(target_output))')
s=s.replace('static int sections, applied;', '''static int sections, applied;
static long hunk_number(char **p) {
 char *end; errno=0; if(!isdigit((unsigned char)**p)) bb_error_msg_and_die("invalid hunk number");
 long n=strtol(*p,&end,10); if(errno || end==*p || n<0 || n>INT_MAX/2) bb_error_msg_and_die("hunk number out of range"); *p=end; return n;
}''')
s=s.replace('TT.oldline = strtol(s, &s, 10);','TT.oldline = hunk_number(&s);').replace("if (*s == ',') TT.oldlen = oldlen = strtol(s+1, &s, 10);","if (*s == ',') {s++; TT.oldlen = oldlen = hunk_number(&s);}").replace('TT.newline = strtol(s+2, &s, 10);','if (s[0]!=\' \' || s[1]!=\'+\') bb_error_msg_and_die("invalid hunk header");\n s+=2; TT.newline = hunk_number(&s);').replace("if (*s == ',') TT.newlen = newlen = strtol(s+1, &s, 10);","if (*s == ',') {s++; TT.newlen = newlen = hunk_number(&s);}\n if(strncmp(s,\" @@\",3)) bb_error_msg_and_die(\"invalid hunk header\");")
s=s.replace('// If we didn\'t continue above, discard this line.', 'if(is_prefixed_with(patchline,"@@")) bb_error_msg_and_die("malformed hunk header");\n\t\t// If we didn\'t continue above, discard this line.')
p.write_text(s)
p=w/'dump.c';s=p.read_text().replace('bb_simple_perror_msg(dumper->argv[-1]);','bb_simple_perror_msg_and_die("input");');p.write_text(s)
p=w/'get_header_tar.c';s=p.read_text().replace('bb_error_msg("warning: skipping header \'%c\'", tar_typeflag);','bb_error_msg_and_die("unsupported extended tar header: %c", tar_typeflag);').replace('file_header->size = GET_OCTAL(tar.size);','{ unsigned long long size = GET_OCTAL(tar.size); if(size>LONG_MAX) bb_error_msg_and_die("member exceeds platform file range"); file_header->size=size; }');p.write_text(s)
s=(u/'archival/libarchive/data_extract_to_stdout.c').read_text().replace('\tbb_copyfd_exact_size', '\tif (archive_handle->file_header->link_target || !S_ISREG(archive_handle->file_header->mode)) bb_error_msg_and_die("stdout extraction requires a regular file");\n\tbb_copyfd_exact_size');(w/'data_extract_to_stdout.c').write_text(s)
# Both off_t and long are 32-bit on this target; exported fseek is sufficient.
for name in ['diff','xxd','dump']:
 p=w/(name+'.c');p.write_text(p.read_text().replace('fseeko(', 'fseek('))
p=w/'get_header_tar.c';s=p.read_text().replace('v = strtoull(str, &end, 8);','errno=0; v = strtoul(str, &end, 8);\n if(errno==ERANGE) bb_error_msg_and_die("octal value exceeds platform range");');p.write_text(s)
# Pinned newlib getopt mistakenly treats lone '-' as an option. Keep the vendor
# source intact; this narrow parser fix restores the conventional stdin operand.
s=(r/'src/vendor/newlib-getopt/getopt.c').read_text();old="(argv_element[0] == '-') || (only && argv_element[0] == '+')";assert old in s;s=s.replace(old,"((argv_element[0] == '-' || (only && argv_element[0] == '+')) && argv_element[1])");(w/'getopt.c').write_text(s)
# Text patching must never truncate embedded NULs in a matched/scanned line.
p=w/'patch.c';s=p.read_text();s=s.replace('static int sections, applied;', '''static int sections, applied;
extern int app_yield(void);
static void patch_io_failure(const char *message) {
 if(app_yield()) exit(130);
 bb_simple_perror_msg_and_die(message);
}
static char *patch_line(FILE *fp,int target) {
 char *line=NULL; size_t capacity=0; ssize_t n=getline(&line,&capacity,fp);
 if(n<0) {free(line); die_if_ferror(fp,"input"); return NULL;}
 if(memchr(line,0,n)) bb_error_msg_and_die("binary patch/target unsupported; original preserved");
 if(target && n && line[n-1]!='\\n') bb_error_msg_and_die("target lacks final newline; original preserved");
 if(n && line[n-1]=='\\n') line[n-1]=0;
 return line;
}''')
s=s.replace('xmalloc_fgetline(target_input)','patch_line(target_input,1)').replace('xmalloc_fgetline(patch_input)','patch_line(patch_input,0)')
s=s.replace('bb_simple_perror_msg_and_die("begin replacement")','patch_io_failure("begin replacement")').replace('bb_simple_perror_msg_and_die("replace")','patch_io_failure("replace")')
s=s.replace(' || n>INT_MAX/2','').replace('\n\t\t\t\toldsum =', '\n if(TT.oldline>LONG_MAX-oldlen || TT.newline>LONG_MAX-newlen) bb_error_msg_and_die("hunk range overflow");\n\t\t\t\toldsum =')
p.write_text(s)
p=w/'xxd.c';s=p.read_text().replace('filename ? xfopen_for_read(filename) : stdin','filename ? xfopen_stdin(filename) : stdin');s=s.replace('\targv += optind;', '\tif(cols>INT_MAX || cols>SIZE_MAX/(sizeof("/1 \\\"%02x \\\"")-1)) bb_error_msg_and_die("column count exceeds representable format");\n\targv += optind;');p.write_text(s)
p=w/'tar.c';s=p.read_text().replace('\targv += optind;', '\tif((opt & OPT_2STDOUT) && verboseFlag) bb_error_msg_and_die("-v with -O unavailable; list separately");\n\targv += optind;');p.write_text(s)
# Without extended-name support, skipping PAX could silently use a truncated name.
p=w/'get_header_tar.c';s=p.read_text().replace("case 'x': {\t/* pax extended header */", "case 'x': {\t/* pax extended header */\n\t\tbb_error_msg_and_die(\"PAX extended headers unavailable\");");s=s.replace('while (full_read(archive_handle->src_fd, &tar, 512) == 512)\n\t\t\t\tcontinue;', 'while ((i=full_read(archive_handle->src_fd, &tar, 512)) == 512)\n\t\t\t\tcontinue;\n\t\t\tif(i<0) bb_simple_perror_msg_and_die("archive input");');p.write_text(s)
