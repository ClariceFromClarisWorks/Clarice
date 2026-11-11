#ifndef CLARICE_DIRENT_H
#define CLARICE_DIRENT_H
#include <sys/types.h>
struct dirent {ino_t d_ino;unsigned char d_type;char d_name[256];};
typedef struct CliDirectory DIR;
DIR *opendir(const char *);
struct dirent *readdir(DIR *);
int closedir(DIR *);
#define DT_UNKNOWN 0
#define DT_DIR 4
#define DT_REG 8
#define DT_LNK 10
#endif
