/* Missing public MacTCP declarations from Apple's Programmer's Guide,
 * chapters5/6. These describe the installed interface, not an IP stack. */
#ifndef CLARICE_NETWORK_ABI_H
#define CLARICE_NETWORK_ABI_H
#include <stddef.h>
#pragma pack(push,2)
typedef struct {long rtnCode;char cname[255];unsigned long addr[4];} NetHostInfo;
#define NET_PB_HEADER unsigned char reserved[12];ProcPtr ioCompletion;short ioResult;StringPtr ioNamePtr;short ioVRefNum,ioCRefNum,csCode
typedef struct {NET_PB_HEADER;struct {struct {
 unsigned long dest;wdsEntry data;short timeout;Ptr options;unsigned short optLength;
 ProcPtr icmpCompletion;unsigned long userDataPtr;
} IPEchoPB;} csParam;} NetIPBlock;
typedef struct {NET_PB_HEADER;short params[11];struct {
 unsigned long echoRequestOut,echoReplyIn;rdsEntry echoedData;Ptr options;unsigned long userDataPtr;
} icmpEchoInfo;} NetICMPBlock;
#pragma pack(pop)
#ifndef NETWORK_HOST_TEST
_Static_assert(offsetof(NetHostInfo,addr)==260,"DNR hostInfo ABI");
_Static_assert(offsetof(NetIPBlock,csParam)==28,"IP PB ABI");
_Static_assert(sizeof(NetIPBlock)==54,"echo request ABI");
_Static_assert(offsetof(NetICMPBlock,icmpEchoInfo)==50,"echo callback ABI");
#endif
#undef NET_PB_HEADER
#endif
