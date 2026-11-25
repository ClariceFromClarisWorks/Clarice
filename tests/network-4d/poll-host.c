/* Exercise actual client loops with existing bounded fault mocks. */
#define main established_tests
#if defined(TEST_HTTP)
#include "../http/host.c"
#elif defined(TEST_FTP)
#include "../ftp/host.c"
#else
#include "../telnet/host.c"
#endif
#undef main
int main(void){
 established_tests();
#if defined(TEST_HTTP)
 char header[4200];memset(header,'x',sizeof header);
 const char *prefix="HTTP/1.0 200 OK\r\nX-Padding: ";memcpy(header,prefix,strlen(prefix));
 strcpy(header+4020,"\r\nContent-Length: 0\r\n\r\n");reset(header);fragment=2048;pauses=0;
 assert(!run()&&calls<32&&!pauses&&!live);printf("HTTP buffered header: %u checks, %u sleeps\n",calls,pauses);
 /* A cancellation arriving between checks is observed by the next 256-byte
    boundary, without a native read or sleep while bytes remain buffered. */
 reset(header);available=2048;used=1;pos=1;stop=1;calls=pauses=0;
 for(;used<256;used++)assert(poll_input()==1);
 assert(!calls&&poll_input()==-1&&cancelled&&!pauses);
 reset(header);stall=1;pauses=0;assert(run()==1&&pauses>0&&!temporary&&!live);
#elif defined(TEST_FTP)
 reset();memset(cb,'x',sizeof cb);cb[510]='\r';cb[511]='\n';at=0;have=512;calls=pauses=0;
 assert(line()&&calls==2&&!pauses&&strlen(reply_text)==510);
 reset();memset(cb,'x',sizeof cb);cb[510]='\r';cb[511]='\n';at=1;have=512;calls=80;stop=1;
 assert(!line()&&at==256&&cancelled);
 reset();stall=stop=1;pauses=0;assert(run()==130&&pauses>0&&!temp&&!files&&!live);
#else
 reset();cancel=1;pauses=0;assert(run()==130&&pauses>0&&!allocations&&!conn.open&&!terminal);
 reset();input[in++]='x';closed=1;pauses=0;assert(!run()&&!pauses&&on==1&&!allocations);
#endif
 puts("PASS buffered-event bounds, empty-read sleep/cancellation, available-data fast path and cleanup");return 0;
}
