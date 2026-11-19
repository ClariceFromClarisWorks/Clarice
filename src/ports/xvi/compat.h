/* Clarice xvi adaptation; distributed under licenses/xvi/COPYING. */
#include "../terminal/ncurses-compat.h"
#include "../safe-save.h"
/* Native single-Finder invocations cannot suspend Unix processes. */
#undef SIGSTOP
#undef SIGTSTP
int xvi_access(const char *,int);
int xvi_remove(const char *);
int xvi_chdir(const char *);
char *xvi_getcwd(char *,size_t);
FILE *xvi_save_open(const char *,const char *);
int xvi_save_close(FILE *);
#define access xvi_access
#define remove xvi_remove
#define chdir xvi_chdir
#define getcwd xvi_getcwd
#undef fclose
#define fclose xvi_save_close
