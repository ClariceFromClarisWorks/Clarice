#define connectionClosing (-23005)
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define userCanceledErr (-128)
typedef struct {int open;} NetConn;
int NetResolve(const char *,unsigned long *,int (*)(void),unsigned long);
int NetConnect(NetConn *,unsigned long,unsigned short),NetClose(NetConn *),NetSend(NetConn *,const void *,unsigned short);
long NetAvailable(NetConn *),NetRecv(NetConn *,void *,unsigned short,unsigned char);
int app_yield(void);void app_activity_interactive(int);

int cli_pause_ticks(unsigned long);
