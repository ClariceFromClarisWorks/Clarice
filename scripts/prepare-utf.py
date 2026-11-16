"""Isolated sbase ASCII fast paths; preserve upstream Unicode/error behaviour."""
from pathlib import Path
r=Path(__file__).resolve().parents[1];out=r/'work/build/utf';out.mkdir(parents=True,exist_ok=True)
for name in ['fgetrune','fputrune','isspacerune']:
 s=(r/f'src/vendor/sbase/libutf/{name}.c').read_text()
 s=s.replace('"../utf.h"','"utf.h"').replace('"runetype.h"','"libutf/runetype.h"')
 if name=='fgetrune':
  old='\t\tbuf[i++] = c;';assert old in s
  s=s.replace(old,'\t\t/* ASCII is a complete rune; avoid the general UTF decoder. */\n\t\tif (!i && c < 0x80) { *r = c; return ferror(fp) ? -1 : 1; }\n'+old)
 elif name=='fputrune':
  old='\treturn fwrite(buf, runetochar(buf, r), 1, fp);';assert old in s
  s=s.replace(old,'\t/* Preserve fwrite item-count convention, including failure = 0. */\n\tif ((unsigned long)*r < 0x80) return fputc((int)*r, fp) == EOF ? 0 : 1;\n'+old)
 else:
  old='\tif(bsearch(';assert old in s
  s=s.replace(old,'\t/* Exact ASCII subset of upstream space2, including 0x1c..0x1f. */\n\tif ((unsigned long)r < 0x80) return (r >= 9 && r <= 13) || (r >= 0x1c && r <= 0x20);\n'+old,1)
 (out/f'{name}.c').write_text(s)
