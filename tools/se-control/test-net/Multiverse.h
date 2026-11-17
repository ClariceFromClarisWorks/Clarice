#pragma once
#include "MacTypes.h"
enum {TCPCreate=30,TCPActiveOpen=32,TCPSend=34,TCPRcv=37,TCPClose=38,TCPAbort=39,TCPStatus=40,TCPRelease=42};
enum {openFailed=-23015,commandTimeout=-23016,connectionDoesntExist=-23008,connectionClosing=-23005,connectionTerminated=-23012,invalidStreamPtr=-23010};
typedef struct {short ioCRefNum,csCode,ioResult; Ptr tcpStream;
 union {
 struct {Ptr rcvBuff;unsigned long rcvBuffLen;void *notifyProc,*userDataPtr;} create;
 struct {int ulpTimeoutValue,ulpTimeoutAction,validityFlags,commandTimeoutValue;unsigned long remoteHost,localHost;unsigned short remotePort,localPort;int tosFlags,precedence,dontFrag,timeToLive,security,optionCnt;void *userDataPtr;} open;
 struct {unsigned long amtUnreadData;int connectionState;void *userDataPtr;} status;
 struct {int commandTimeoutValue;Ptr rcvBuff;unsigned short rcvBuffLen;void *userDataPtr;} receive;
 struct {int ulpTimeoutValue,ulpTimeoutAction,validityFlags,pushFlag,urgentFlag;Ptr wdsPtr;void *userDataPtr;} send;
 } csParam;
} TCPiopb;
typedef TCPiopb *ParmBlkPtr;
OSErr OpenDriver(ConstStringPtr,short*),PBControlSync(ParmBlkPtr),PBControlAsync(ParmBlkPtr),MemError(void);
Ptr NewPtrSys(unsigned long);void DisposePtr(Ptr),SystemTask(void);unsigned long TickCount(void);
