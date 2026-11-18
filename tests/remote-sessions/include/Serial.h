#include "Multiverse.h"
typedef struct {unsigned char fXOn,fCTS,xOn,xOff,errs,evts,fInX,fDTR;} SerShk;
typedef struct {unsigned char cumErrs;} SerStaRec;
typedef struct {struct {short ioRefNum,ioResult;Ptr ioBuffer;long ioReqCount,ioActCount;} ioParam;} ParamBlockRec;
#define baud57600 0
#define data8 0
#define noParity 0
#define stop10 0
int KillIO(short);int CloseDriver(short);int SerSetBuf(short,Ptr,long);int SerReset(short,int);int SerHShake(short,SerShk*);int SerStatus(short,SerStaRec*);int SerGetBuf(short,long*);int FSRead(short,long*,void*);int PBWriteAsync(ParamBlockRec*);
