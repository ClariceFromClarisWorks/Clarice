#ifndef CLARICE_NCURSES_COMPAT_H
#define CLARICE_NCURSES_COMPAT_H
#include "../compat.h"
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/select.h>
int port_fileno(FILE *);
int cli_isatty(int);
char *port_curses_getenv(const char *);
ssize_t port_terminal_read(int,void *,size_t);
#undef read
#define read port_terminal_read
#define fileno port_fileno
#define isatty cli_isatty
#define getenv port_curses_getenv
#define _exit port_exit
#endif
