#ifndef CLARICE_NANO_BUFFERED_GETC_H
#define CLARICE_NANO_BUFFERED_GETC_H
#include <stdio.h>
/* Newlib/BSD FILE buffered cursor, also used by port_getline. This invocation
 * owns the stream; refill, pushback, EOF and errors remain libc's responsibility.
 * Equivalent to Newlib's single-thread __sgetc fast path, with the existing getc
 * import handling refills instead of adding libc-private runtime dependencies. */
static inline int port_nano_getc(FILE *stream){
#if defined(__NEWLIB__) || defined(__APPLE__)
 if(stream->_r>0){--stream->_r;return *stream->_p++;}
#endif
 return (getc)(stream);
}
#undef getc
#define getc(stream) port_nano_getc(stream)
#endif
