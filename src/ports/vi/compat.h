#ifndef CLARICE_VI_COMPAT_H
#define CLARICE_VI_COMPAT_H
#include "../terminal/ncurses-compat.h"
#include <stdint.h>
#include <time.h>
#include <sys/wait.h>
#undef SIGTSTP
#undef SIGSTOP
#undef _SC_JOB_CONTROL
#undef _SC_PAGESIZE
#undef _PC_VDISABLE
#undef SIGWINCH
#undef getline
#undef read
#define read vi_read
#undef write
#define write vi_write
#define stat(p,s) vi_stat(p,s)
#define unlink cli_remove
#define sbrk vi_sbrk
#define alarm vi_alarm
#define fork vi_fork
#define pipe vi_pipe
#define execl vi_execl
#define execv vi_execv
#define wait vi_wait
#define kill vi_kill
#define dup vi_dup
#define chdir vi_chdir
#define getcwd vi_getcwd
#define access vi_access
#define chmod vi_chmod
#define ttyname vi_ttyname
#undef getenv
#define getenv vi_getenv
int vi_ioctl(int,unsigned long,...);
int vi_sigaction(int,const struct sigaction *,struct sigaction *);
void *vi_sbrk(intptr_t);
ssize_t vi_read(int,void *,size_t);
ssize_t vi_write(int,const void *,size_t);
int vi_stat(const char *,struct stat *);
int cli_remove(const char *);
unsigned vi_alarm(unsigned);
pid_t vi_fork(void),vi_wait(int *);
int vi_pipe(int *),vi_execl(const char *,const char *,...),vi_execv(const char *,char *const *),vi_kill(pid_t,int),vi_dup(int),vi_chdir(const char *);
char *vi_ttyname(int),*vi_getenv(const char *);
char *vi_getcwd(char *,size_t);
int vi_access(const char *,int),vi_chmod(const char *,mode_t);
int vi_save_abort_close(int);
int vi_temp_seek(int,off_t,int);
int vi_temp_id(void),vi_save_create(const char *),vi_save_close(int);
#endif
