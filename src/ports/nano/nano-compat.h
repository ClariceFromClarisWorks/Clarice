#ifndef CLARICE_NANO_COMPAT_H
#define CLARICE_NANO_COMPAT_H
#include <config.h>
#include "../terminal/ncurses-compat.h"
#include <curses.h>
#include <locale.h>
#include <time.h>
#include <libgen.h>
int port_nano_stat(const char *,struct stat *);
int port_nano_access(const char *,int);
char *port_nano_realpath(const char *,char *);
FILE *port_fdopen(int,const char *);
WINDOW *port_nano_initscr(void);
unsigned int port_nano_save_id(void);
int port_nano_putenv(char *);
time_t port_nano_time(time_t *);
#define stat(p,s) port_nano_stat(p,s)
#define access port_nano_access
#define realpath port_nano_realpath
#define fdopen port_fdopen
#define initscr port_nano_initscr
#define getpid port_nano_save_id
#define putenv port_nano_putenv
#define time port_nano_time
#endif
