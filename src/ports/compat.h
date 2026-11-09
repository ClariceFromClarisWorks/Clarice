/* Clarice port boundary. Include system declarations before redirecting APIs. */
#ifndef CLARICE_PORT_COMPAT_H
#define CLARICE_PORT_COMPAT_H
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
void *port_malloc(size_t);
void *port_calloc(size_t,size_t);
void *port_realloc(void *,size_t);
void port_free(void *);
char *port_strdup(const char *);
char *port_strndup(const char *,size_t);
FILE *port_standard(int);
FILE *port_fopen(const char *,const char *);
FILE *port_fmemopen(void *,size_t,const char *);
int port_fclose(FILE *);
ssize_t port_getline(char **,size_t *,FILE *);
void port_exit(int) __attribute__((noreturn));
int port_snprintf(char *,size_t,const char *,...);
int port_printf(const char *,...);
int port_fprintf(FILE *,const char *,...);
int port_vfprintf(FILE *,const char *,va_list);
int port_putchar(int);
int port_puts(const char *);
void port_perror(const char *);
void port_abort(void) __attribute__((noreturn));
void port_poll(void);
int port_open(const char *,int,...);
int port_close(int);
ssize_t port_read(int,void *,size_t);
ssize_t port_write(int,const void *,size_t);
int port_fstat(int,struct stat *);
off_t port_lseek(int,off_t,int);
unsigned port_sleep(unsigned);
const char *port_temp_new(void);
void port_temp_remove(const char *);

#ifndef PORT_RUNTIME
#define open port_open
#define close port_close
#define read port_read
#define write port_write
#define fstat port_fstat
#define lseek port_lseek
#define sleep port_sleep
#define snprintf port_snprintf
#define fprintf port_fprintf
#define vfprintf port_vfprintf
#define printf port_printf
#define puts port_puts
#undef putchar
#define putchar port_putchar
#define perror port_perror
#define abort port_abort
#define malloc port_malloc
#define calloc port_calloc
#define realloc port_realloc
#define free port_free
#define strdup port_strdup
#define strndup port_strndup
#define fopen port_fopen
#define fmemopen port_fmemopen
#define fclose port_fclose
#define getline port_getline
#define exit port_exit
#undef stdin
#undef stdout
#undef stderr
#define stdin port_standard(0)
#define stdout port_standard(1)
#define stderr port_standard(2)
#endif
#endif
