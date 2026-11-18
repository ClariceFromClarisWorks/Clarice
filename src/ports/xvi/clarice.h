/* Clarice xvi adaptation; distributed under licenses/xvi/COPYING.
   Reuse upstream generic terminal declarations, not the Unix process backend. */
#include "unix.h"
#undef fopenwb
#undef fopenab
#define fopenwb(f) xvi_save_open((f),"w")
#define fopenab(f) xvi_save_open((f),"a")
#undef READBUFSIZ
#undef WRTBUFSIZ
#define READBUFSIZ 4096
#define WRTBUFSIZ 4096
int inchar(long);
