#!/usr/bin/env python3
"""Bounded nano 9.2 read-buffer adaptation for a moving-realloc runtime.

Use gnulib's geometric-growth strategy, retaining the original exact request
under memory pressure. No initial-buffer, file format or editor changes.
"""
from pathlib import Path
OLD = '\t\t\t\tbufsize += LUMPSIZE;\n\t\t\t\tbuf = nrealloc(buf, bufsize);'
GROWTH = r'''
                /* CLARICE_LINE_GROWTH: geometric growth as in gnulib dynarray; retain exact-growth
                 * fallback under pressure, and nano's normal fatal-OOM path. */
                if (bufsize > SIZE_MAX - LUMPSIZE)
                    die(_("Nano is out of memory!\n"));
                size_t minimum = bufsize + LUMPSIZE;
                size_t extra = bufsize / 2 + 1;
                size_t grown = extra > SIZE_MAX - bufsize ? minimum : bufsize + extra;
                if (grown < minimum) grown = minimum;
                char *larger = realloc(buf, grown);
                if (larger) { buf = larger; bufsize = grown; }
                else { buf = nrealloc(buf, minimum); bufsize = minimum; }
'''.strip("\n")

def apply(source):
    if GROWTH in source:
        return source
    if source.count(OLD) != 1:
        raise ValueError("Unexpected nano line-buffer growth source")
    return source.replace(OLD, GROWTH)

if __name__ == '__main__':
    path = Path(__file__).resolve().parents[1] / 'work/nano/cross/nano-9.2/src/files.c'
    path.write_text(apply(path.read_text()))
