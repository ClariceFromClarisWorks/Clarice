#ifndef REMOTE_MOCK_NATIVE
#define REMOTE_MOCK_NATIVE
#include <stddef.h>
typedef char *Ptr;typedef const unsigned char *ConstStringPtr;typedef unsigned char Boolean;typedef short OSErr,SInt8;typedef void *ParmBlkPtr;
#define nil NULL
#define noErr 0
#define userCanceledErr -128
#define paramErr -50
#define false 0
#define true 1
#define TCPCreate 1
#define TCPPassiveOpen 2
#define TCPActiveOpen 3
#define TCPSend 4
#define TCPRcv 5
#define TCPStatus 6
#define TCPAbort 7
#define TCPRelease 8
#define TCPClose 9
#define openFailed -23015
#define commandTimeout -23016
#define connectionClosing -23005
#define connectionTerminated -23012
#define connectionDoesntExist -23008
#define invalidStreamPtr -23010
typedef struct {short ioCRefNum,csCode,ioResult;Ptr tcpStream;struct {
 struct {Ptr rcvBuff;unsigned long rcvBuffLen;void *notifyProc,*userDataPtr;} create;
 struct {int ulpTimeoutValue,ulpTimeoutAction,validityFlags,commandTimeoutValue;unsigned long remoteHost,localHost;unsigned short remotePort,localPort;int tosFlags,precedence,dontFrag,timeToLive,security,optionCnt;void *userDataPtr;} open;
 struct {int ulpTimeoutValue,ulpTimeoutAction,validityFlags,pushFlag,urgentFlag;Ptr wdsPtr;void *userDataPtr;} send;
 struct {int commandTimeoutValue;Ptr rcvBuff;unsigned long rcvBuffLen;void *userDataPtr;} receive;
 struct {unsigned long amtUnreadData;int connectionState;void *userDataPtr;} status;
 struct {int ulpTimeoutValue,ulpTimeoutAction,validityFlags;void *userDataPtr;} close;
} csParam;} TCPiopb;
short OpenDriver(ConstStringPtr,short*),PBControlSync(ParmBlkPtr),PBControlAsync(ParmBlkPtr),MemError(void);
Ptr NewPtrSys(unsigned long),NewPtrSysClear(unsigned long);void DisposePtr(Ptr);unsigned long TickCount(void);
#endif
