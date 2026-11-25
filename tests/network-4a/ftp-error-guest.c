/* Guest-only fault after real TCP data. Production FTP, staging and native
 * connection teardown remain unchanged. Server sends ordinary final 226. */
static int delivered;
#define NetAvailable injected_available
#define NetRecv injected_receive
#include "../../src/commands/ftp.c"
#undef NetAvailable
#undef NetRecv
extern long NetAvailable(NetConn *);
extern long NetRecv(NetConn *,void *,unsigned short,unsigned char);
long injected_available(NetConn *c){if(c==&data&&delivered)return connectionTerminated;return NetAvailable(c);}
long injected_receive(NetConn *c,void *p,unsigned short n,unsigned char t){long k=NetRecv(c,p,n,t);if(c==&data&&k>0)delivered=1;return k;}
