from pathlib import Path
root=Path(__file__).resolve().parents[1]/'work/nano/cross/nano-9.2/src'
p=root/'nano.c';s=p.read_text()
if 'NANO_PERF(7)' not in s:
 s=s.replace('\t\tprocess_a_keystroke();','\t\tNANO_PERF(7);\n\t\tprocess_a_keystroke();')
 s=s.replace('if (!open_buffer(filename, TRUE))','NANO_PERF(5);\n\t\t\tbool opened = open_buffer(filename, TRUE);\n\t\t\tNANO_PERF(6);\n\t\t\tif (!opened)')
 s=s.replace('\t\topen_buffer("", TRUE);','\t\tNANO_PERF(5);open_buffer("", TRUE);NANO_PERF(6);')
 p.write_text(s)
p=root/'files.c';s=p.read_text()
if 'NANO_PERF(11)' not in s:
 s=s.replace(' /* CLARICE_SAFE_SAVE:', ' NANO_PERF(11);\n /* CLARICE_SAFE_SAVE:')
 # successful write_file returns true; add mark at function-specific tail only
 start=s.index('bool write_file(');end=s.index('\n}',start)
 chunk=s[start:end];assert chunk.count('return TRUE;')==1
 chunk=chunk.replace('return TRUE;', 'NANO_PERF(12);return TRUE;')
 s=s[:start]+chunk+s[end:];p.write_text(s)

if 'NANO_PERF(9)' not in s:
 s=s.replace('\t/* Read in the entire file, byte by byte, line by line. */','\tNANO_PERF(9);\n\t/* Read in the entire file, byte by byte, line by line. */')
 s=s.replace('\terrornumber = errno;', '\tNANO_PERF(10);\n\terrornumber = errno;')
 p.write_text(s)
