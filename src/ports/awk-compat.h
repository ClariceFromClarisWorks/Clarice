#include "compat.h"
#include <sys/stat.h>
#include <locale.h>
#include <time.h>
#include <stdint.h>
int port_awk_integer(char *,size_t,const char *,uintmax_t);
extern char **port_environ;
char *port_getenv(const char *);
int port_system(const char *);
FILE *port_popen(const char *,const char *);
int port_pclose(FILE *);
int port_stat(const char *,struct stat *);
FILE *port_null_reopen(const char *,const char *,FILE *);
time_t port_time(time_t *);
long port_random(void);
void port_srandom(unsigned long);
#ifndef PORT_RUNTIME
#define environ port_environ
#define getenv port_getenv
#define system port_system
#define popen port_popen
#define pclose port_pclose
#define stat(p,s) port_stat(p,s)
#define freopen port_null_reopen
#define time port_time
#define random port_random
#define srandom port_srandom
#endif
