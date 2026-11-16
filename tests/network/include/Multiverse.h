#ifndef TEST_MULTI
#define TEST_MULTI
#include "MacTypes.h"
#define pascal
#define fsRdPerm 1
#define paramErr (-50)
#define userCanceledErr (-128)
#define resNotFound (-192)
#define cacheFault (-23042)
#define commandTimeout (-23016)
#define noAnsErr (-23046)
#define ipctlGetAddr 15
typedef struct {short sysVRefNum;} SysEnvRec;
typedef struct {unsigned short length;Ptr ptr;} wdsEntry,rdsEntry;
typedef struct {char fill[12];ProcPtr ioCompletion;short ioResult;StringPtr ioNamePtr;short ioVRefNum,ioCRefNum,csCode;unsigned long ourAddress;long ourNetMask;} GetAddrParamBlock;
typedef void *ParmBlkPtr;
short OpenDriver(ConstStringPtr,short*),PBControlSync(ParmBlkPtr),PBControlAsync(ParmBlkPtr);
short CurResFile(void),SysEnvirons(short,SysEnvRec*),OpenRFPerm(ConstStringPtr,short,char),ResError(void);
Handle Get1IndResource(unsigned long,short);void LoadResource(Handle),DetachResource(Handle),HLock(Handle),HUnlock(Handle),CloseResFile(short),UseResFile(short),DisposeHandle(Handle),SystemTask(void);
unsigned long TickCount(void);
#endif
