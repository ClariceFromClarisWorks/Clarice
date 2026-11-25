#define connectionClosing (-23005)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define PATH_CAP 512
#define fnfErr -43
#define dupFNErr -48
#define bdNamErr -37
#define ioErr -36
typedef struct {struct {long ioFlLgLen;} hFileInfo;} CInfoPBRec;
typedef struct {int open;} NetConn;
int app_yield(void),checked(const char *,char *,char *,int),info(const char *,CInfoPBRec *);
unsigned long cli_ticks(void),NetParseIP(const char *);
int NetConnect(NetConn *,unsigned long,unsigned short),NetSend(NetConn *,const void *,unsigned short);
long NetAvailable(NetConn *),NetRecv(NetConn *,void *,unsigned short,unsigned char);
int NetClose(NetConn *);
void say(const char *),errsay(const char *);
int cli_native_create(const char *,unsigned long,unsigned long),cli_native_open(const char *,int,int,short *),cli_native_write(short,const void *,long *),cli_native_close(short),cli_native_rename(const char *,const char *),cli_native_delete(const char *);

int cli_isatty(int);
void app_progress(const char *,int);

int app_columns(void);

#define userCanceledErr (-128)
int NetResolve(const char *,unsigned long *,int (*)(void),unsigned long);

#define eofErr (-39)
int isdir(CInfoPBRec *);
int cli_native_read(short,void *,long *);

int cli_pause_ticks(unsigned long);
