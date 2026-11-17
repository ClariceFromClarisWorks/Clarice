/* Replay the valid LEN=94 packet captured during physical SE delivery.
   Exercises both adapter collection and engine validation, without hardware. */
#define KERMIT_HOST
#define main kermit_cli_main
#include "../../src/commands/kermit.c"
#undef main
#include <assert.h>
#include <sys/socket.h>
int main(int argc,char **argv){
 assert(argc==2);FILE *f=fopen(argv[1],"r");assert(f);unsigned char packet[128];int count=0,x;
 while(fscanf(f,"%2x",&x)==1){assert(count<(int)sizeof packet);packet[count++]=x;}fclose(f);
 assert(count==97&&packet[1]==126);int sockets[2];assert(!socketpair(AF_UNIX,SOCK_STREAM,0,sockets));wire=sockets[0];
 protocol.binary=1;protocol.xfermode=1;protocol.bct=3;protocol.bctf=1;
 protocol.zinbuf=in_file;protocol.zinlen=sizeof in_file;protocol.obuf=out_file;protocol.obuflen=sizeof out_file;
 protocol.txd=transmit;protocol.ixd=pending;protocol.openf=open_file;protocol.readf=read_file;protocol.writef=write_file;protocol.closef=close_file;
 assert(kermit(K_INIT,&protocol,0,0,"",&response)==X_OK);
 protocol.state=R_DATA;protocol.r_seq=32;protocol.rptflg=1;protocol.rptq='~';
 assert(write(sockets[1],packet,count)==count);short slot;UCHAR *p=getrslot(&protocol,&slot);assert(p);
 int n=read_packet(&protocol,p,P_PKTLEN+1);assert(n==95);
 assert(kermit(K_RUN,&protocol,slot,n,"",&response)==X_OK);
 unsigned char ack[32];int got=read(sockets[1],ack,sizeof ack);assert(got==8&&ack[3]=='Y');assert(protocol.obufpos==52);
 close(sockets[0]);close(sockets[1]);puts("PASS: maximum short packet collected, CRC validated, acknowledged, 52 binary bytes decoded");return 0;
}
